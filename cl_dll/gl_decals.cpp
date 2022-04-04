//
// written by BUzer for HL: Paranoia modification
//
//		2006

#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "cdll_int.h"
#include "com_model.h"
#include "glmanager.h"
#include "entity_types.h"
#include "gl_texloader.h"

#include "studio_event.h" // def. of mstudioevent_t
#include "r_efx.h"
#include "event_api.h"
#include "pm_defs.h"
#include "pmtrace.h"
#include "parsemsg.h"
#include "gl_renderer.h"

#include <vector>

#define MAX_DECAL_MSG_CACHE	256

#include "gl_decals.h"

DecalGroup *pDecalGroupList = NULL;

DecalGroup::DecalGroup(const char *_name, int numelems, DecalGroupEntry *source)
{
	strcpy(name, _name);
	pEntryArray = new DecalGroupEntry[numelems];
	size = numelems;
	memcpy(pEntryArray, source, sizeof(DecalGroupEntry)*size);
	pnext = pDecalGroupList;
	pDecalGroupList = this;
}

DecalGroup::~DecalGroup()
{
	delete[] pEntryArray;
	if (pnext) delete pnext;
}

DecalGroupEntry* DecalGroup::GetEntry(int num)
{
	return &pEntryArray[num];

}

const DecalGroupEntry* DecalGroup::GetRandomDecal()
{
	if (size == 0)
		return NULL;

	if (size == 1)
		return &pEntryArray[0];

	int idx = gEngfuncs.pfnRandomLong(0, size - 1);
	return &pEntryArray[idx];
}

DecalGroup* FindGroup(const char *_name)
{
	DecalGroup *plist = pDecalGroupList;
	while (plist)
	{
		if (!strcmp(plist->name, _name))
			return plist;

		plist = plist->pnext;
	}
	return NULL;
}

decaltexture decaltextures[MAX_DECALTEXTURES];
customdecal decals[MAX_CUSTOMDECALS];
customdecal staticdecals[MAX_STATICDECALS];
decal_msg_cache msgcache[MAX_DECAL_MSG_CACHE];

int numdecaltextures = 0;
int numdecals = 0;
int numstaticdecals = 0;
int curdecal = 0;
int cachedecals = 0;

int decalrendercounter = 0;
int staticdecalrendercounter = 0;

cvar_t *cv_customdecals;
cvar_t *cv_decalsdebug;



// ===========================
// Math
// ===========================

void FindIntersectionPoint(const vec3_t &p1, const vec3_t &p2, const vec3_t &normal, const vec3_t &planepoint, vec3_t &newpoint)
{
	vec3_t planevec;
	vec3_t linevec;
	float planedist, linedist;

	VectorSubtract(planepoint, p1, planevec);
	VectorSubtract(p2, p1, linevec);
	planedist = DotProduct(normal, planevec);
	linedist = DotProduct(normal, linevec);

	if (linedist != 0)
	{
		VectorMA(p1, planedist / linedist, linevec, newpoint);
		return;
	}
	VectorClear(newpoint);
}

int ClipPolygonByPlane(const vec3_t *arrIn, int numpoints, vec3_t normal, vec3_t planepoint, vec3_t *arrOut)
{
	int i, cur, prev;
	int first = -1;
	int outCur = 0;
	float dots[64];
	for (i = 0; i < numpoints; i++)
	{
		vec3_t vecDir;
		VectorSubtract(arrIn[i], planepoint, vecDir);
		dots[i] = DotProduct(vecDir, normal);
		if (dots[i] > 0) first = i;
	}
	if (first == -1) return 0;

	VectorCopy(arrIn[first], arrOut[outCur]);
	outCur++;

	cur = first + 1;
	if (cur == numpoints) cur = 0;
	while (cur != first)
	{
		if (dots[cur] > 0)
		{
			VectorCopy(arrIn[cur], arrOut[outCur]);
			cur++; outCur++;
			if (cur == numpoints) cur = 0;
		}
		else
			break;
	}

	if (cur == first) return outCur;

	if (dots[cur] < 0)
	{
		vec3_t newpoint;
		if (cur > 0) prev = cur - 1;
		else prev = numpoints - 1;
		FindIntersectionPoint(arrIn[prev], arrIn[cur], normal, planepoint, newpoint);
		VectorCopy(newpoint, arrOut[outCur]);
	}
	else
	{
		VectorCopy(arrIn[cur], arrOut[outCur]);
	}
	outCur++;
	cur++;
	if (cur == numpoints) cur = 0;

	while (dots[cur] < 0)
	{
		cur++;
		if (cur == numpoints) cur = 0;
	}

	if (cur > 0) prev = cur - 1;
	else prev = numpoints - 1;
	if (dots[cur] > 0 && dots[prev] < 0)
	{
		vec3_t newpoint;
		FindIntersectionPoint(arrIn[prev], arrIn[cur], normal, planepoint, newpoint);
		VectorCopy(newpoint, arrOut[outCur]);
		outCur++;
	}

	while (cur != first)
	{
		VectorCopy(arrIn[cur], arrOut[outCur]);
		cur++; outCur++;
		if (cur == numpoints) cur = 0;
	}
	return outCur;
}

void GetUpRight(vec3_t forward, vec3_t &up, vec3_t &right)
{
	VectorClear(up);
	if (forward.x || forward.y)
		up.z = 1;
	else
		up.x = 1;

	right = CrossProduct(forward, up);
	VectorNormalize(right);
	up = CrossProduct(forward, right);
	VectorNormalize(up);
}


// ===========================
// Decals loading
// ===========================

int LoadDecalTexture(const char *texname)
{
	for (int i = 0; i < numdecaltextures; i++)
	{
		if (!strcmp(decaltextures[i].name, texname))
			return decaltextures[i].gl_texid;
	}

	if (numdecaltextures >= MAX_DECALTEXTURES)
	{
		gEngfuncs.Con_Printf("Too many entries in decal info file (%d max)\n", MAX_DECALTEXTURES);
		return 0;
	}

	char path[256];
	sprintf(path, "gfx/decals/%s.tga", texname);
	int newid = CreateTexture(path, MIPS_YES);
	if (!newid)
	{
		gEngfuncs.Con_Printf("Missing decal texture %s!\n", path);
		return 0;
	}

	strcpy(decaltextures[numdecaltextures].name, texname);
	decaltextures[numdecaltextures].gl_texid = newid;
	numdecaltextures++;
	return newid;
}

void LoadDecals()
{
	if (!cv_customdecals->value)
		return;

	//	if (!gl.IsGLAllowed())
	//		return;

	// only once
	static int called = 0;
	if (called) return;
	called = 1;

	gEngfuncs.Con_Printf("\n>> Loading decals\n");

	char *pfile = (char *)gEngfuncs.COM_LoadFile("gfx/decals/decalgroup.txt", 5, NULL);
	if (!pfile)
	{
		gEngfuncs.Con_Printf("Cannot open file \"gfx/decals/decalgroupes.txt\"\n");
		return;
	}

	int counter = 0;
	char *ptext = pfile;
	while (1)
	{
		// store position where group names recorded
		char *groupnames = ptext;

		// loop until we'll find decal names
		int numgroups = 0;
		char token[256];
		while (1)
		{
			ptext = gEngfuncs.COM_ParseFile(ptext, token);
			if (!ptext) goto getout;
			if (token[0] == '{') break;
			numgroups++;
		}

		DecalGroupEntry tempentries[MAX_GROUPENTRIES];
		int numtemp = 0;
		while (1)
		{
			char sz_xsize[64];
			char sz_ysize[64];
			char sz_overlay[64];
			ptext = gEngfuncs.COM_ParseFile(ptext, token);
			if (!ptext) goto getout;
			if (token[0] == '}') break;

			if (numtemp >= MAX_GROUPENTRIES)
			{
				gEngfuncs.Con_Printf("Too many decals in group (%d max) - skipping %s\n", MAX_GROUPENTRIES, token);
				continue;
			}

			ptext = gEngfuncs.COM_ParseFile(ptext, sz_xsize);
			if (!ptext) goto getout;
			ptext = gEngfuncs.COM_ParseFile(ptext, sz_ysize);
			if (!ptext) goto getout;
			ptext = gEngfuncs.COM_ParseFile(ptext, sz_overlay);
			if (!ptext) goto getout;

			if (strlen(token) > 16)
			{
				gEngfuncs.Con_Printf("%s - got too large token when parsing decal info file\n", token);
				continue;
			}

			int id = LoadDecalTexture(token);
			if (!id)
				continue;

			tempentries[numtemp].gl_texid = id;
			tempentries[numtemp].xsize = atof(sz_xsize) / 2;
			tempentries[numtemp].ysize = atof(sz_ysize) / 2;
			tempentries[numtemp].overlay = atof(sz_overlay) * 2;
			numtemp++;
		}

		// get back to group names

		for (int i = 0; i < numgroups; i++)
		{
			groupnames = gEngfuncs.COM_ParseFile(groupnames, token);
			if (!numtemp)
			{
				gEngfuncs.Con_Printf("Warning: Empty Decal Group: %s\n", token);
				continue;
			}

			new DecalGroup(token, numtemp, tempentries);
			counter++;
		}
	}

getout:

	gEngfuncs.COM_FreeFile(pfile);
	//	gEngfuncs.Con_Printf("%d TGA Decal Groups Loaded in decalgroupes.txt\n", counter);
}



// ===========================
// Decals creation
// ===========================
customdecal *AllocDecal()
{
	customdecal *ret = &decals[curdecal];

	if (numdecals < MAX_CUSTOMDECALS)
		numdecals++;

	curdecal++;
	if (curdecal == MAX_CUSTOMDECALS)
		curdecal = 0; // get decals from tail

	return ret;
}


// static decals, used for level detalization, placed by mapper
customdecal *AllocStaticDecal()
{
	if (numstaticdecals < MAX_STATICDECALS)
	{
		customdecal *ret = &staticdecals[numstaticdecals];
		numstaticdecals++;
		return ret;
	}
	return NULL;
}
void CreateDecal(vec3_t endpos, vec3_t pnormal, const char *name, int persistent = 0, cl_entity_t *pEntity = 0)
{
	if (!cv_customdecals->value)
		return;

	// Decals On Entity Work Perfect ;P

	model_t *pModel = gEngfuncs.GetEntityByIndex(0)->model;

	LoadDecals();

	// CreateDecal often being called from server messages. Messages about static
	// decals are sent during initialization, and in some cases i cant access
	// world model when message received (this happens during level transitions).
	// So instead of creating decal now, i will add them to cache -
	// they will be created when drawing starts.
	// Additional Code in ev_hldm.cpp

	if (pEntity)
	{
		if (!pEntity->model)
			return;

		if (pEntity->model->type != mod_brush)
		{
			if (pEntity->model->type == mod_studio)
				return;
		}

		pModel = pEntity->model;
	}

	if (!pModel)
	{
		if (cachedecals >= MAX_DECAL_MSG_CACHE)
			return;

		strcpy(msgcache[cachedecals].name, name);
		msgcache[cachedecals].normal = pnormal;
		msgcache[cachedecals].pos = endpos;
		msgcache[cachedecals].persistent = persistent;
		cachedecals++;

		return;
	}

	DecalGroup *group = FindGroup(name);

	if (!group)
		return;

	const DecalGroupEntry *texptr = group->GetRandomDecal();

	if (!texptr)
		return;

	int xsize = texptr->xsize;
	int ysize = texptr->ysize;

	vec3_t right, up;
	GetUpRight(pnormal, up, right);
	float val = (xsize > ysize) ? xsize : ysize;

	float texc_orig_x = DotProduct(endpos, right);
	float texc_orig_y = DotProduct(endpos, up);

	msurface_t* surfaces = &pModel->surfaces[pModel->firstmodelsurface];

	for (int i = 0; i < pModel->nummodelsurfaces; i++)
	{
		if (!surfaces[i].plane)
			return;

		vec3_t norm;
		float ndist = surfaces[i].plane->dist;
		VectorCopy(surfaces[i].plane->normal, norm);
		if (surfaces[i].flags & SURF_PLANEBACK)
		{
			VectorInverse(norm);
			ndist *= -1;
		}

		if (!persistent)
		{
			float normdot = DotProduct(pnormal, norm);
			if (normdot < 0.4)
				continue;
		}
		else
		{
			GetUpRight(surfaces[i].plane->normal, right, up);
			texc_orig_x = DotProduct(endpos, right);
			texc_orig_y = DotProduct(endpos, up);
		}

		float dist = DotProduct(norm, endpos);
		dist = dist - ndist;

		if (dist < 0)
			dist *= -1;

		if (dist > val)
			continue;

		if (surfaces[i].flags & (SURF_DRAWSKY | SURF_DRAWTURB))
			continue;

		if (!surfaces[i].samples)
			continue;


		// execute part of drawing process to see if polygon completely clipped

		glpoly_t	*p = surfaces[i].polys;
		float		*v = p->verts[0];

		vec3_t array1[64];
		vec3_t array2[64];

		for (int j = 0; j<p->numverts; j++, v += VERTEXSIZE)
			VectorCopy(v, array1[j]);

		int nv;
		vec3_t planepoint;
		VectorMA(endpos, -xsize, right, planepoint);
		nv = ClipPolygonByPlane(array1, p->numverts, right, planepoint, array2);

		VectorMA(endpos, xsize, right, planepoint);
		nv = ClipPolygonByPlane(array2, nv, right*-1, planepoint, array1);

		VectorMA(endpos, -ysize, up, planepoint);
		nv = ClipPolygonByPlane(array1, nv, up, planepoint, array2);

		VectorMA(endpos, ysize, up, planepoint);
		nv = ClipPolygonByPlane(array2, nv, up*-1, planepoint, array1);

		if (!nv)
			continue; // no vertexes left after clipping

		customdecal *newdecal = NULL;

		if (persistent)
		{
			newdecal = AllocStaticDecal();

			if (!newdecal)
				return;
		}
		else
		{
			// look other decals on this surface - is someone too close?
			// if so, clear him

			for (int k = 0; k < numdecals; k++)
			{
				if ((decals[k].surface == &surfaces[i]) &&
					(decals[k].texinfo->xsize == texptr->xsize) &&
					(decals[k].texinfo->ysize == texptr->ysize))
				{

					float texc_x = DotProduct(decals[k].point, right) - texc_orig_x;
					float texc_y = DotProduct(decals[k].point, up) - texc_orig_y;
					if (texc_x < 0) texc_x *= -1;
					if (texc_y < 0) texc_y *= -1;
					if (texc_x < (float)texptr->xsize*texptr->overlay &&
						texc_y < (float)texptr->ysize*texptr->overlay)
					{
					//	newdecal = &decals[k];
						break;
					}
				}
			}

			if (!newdecal)
				newdecal = AllocDecal();
		}

		if (pEntity)
		{
			vec3_t forward, right, up, temp;
			VectorSubtract(endpos, pEntity->origin, endpos);
			if (pEntity->angles[0] || pEntity->angles[1] || pEntity->angles[2])
			{
				AngleVectors(pEntity->angles, forward, right, up);

				VectorCopy(endpos, temp);
				endpos[0] = DotProduct(temp, forward);
				endpos[1] = -DotProduct(temp, right);
				endpos[2] = DotProduct(temp, up);

				// normal too
				VectorCopy(pnormal, temp);
				pnormal[0] = DotProduct(temp, forward);
				pnormal[1] = -DotProduct(temp, right);
				pnormal[2] = DotProduct(temp, up);
			}
		}

		newdecal->verts.resize(nv);
		for (int j = 0; j < nv; j++)
		{
			float texc_x = (DotProduct(array1[j], right) - texc_orig_x) / xsize;
			float texc_y = (DotProduct(array1[j], up) - texc_orig_y) / ysize;
			newdecal->verts[j].texcoord[0] = (texc_x + 1) / 2;
			newdecal->verts[j].texcoord[1] = (texc_y + 1) / 2;

			newdecal->verts[j].position = array1[j];
		}

		newdecal->surface = &surfaces[i];
		VectorCopy(endpos, newdecal->point);
		VectorCopy(pnormal, newdecal->normal);
		newdecal->texinfo = texptr;

		if (pEntity)
			newdecal->entity = pEntity;
	}
	// END
}

void CreateDecalTrace(vec3_t start, vec3_t end, const char *name, int persistent = 0)
{
	/*	pmtrace_t tr;
	gEngfuncs.pEventAPI->EV_SetTraceHull( 2 );
	gEngfuncs.pEventAPI->EV_PlayerTrace( start, end, PM_NORMAL, -1, &tr );
	CreateDecal(tr, name, persistent);*/

	pmtrace_t tr;
	gEngfuncs.pEventAPI->EV_SetTraceHull(2);
	gEngfuncs.pEventAPI->EV_PlayerTrace(start, end, PM_NORMAL, -1, &tr);

	cl_entity_t *pEntity = NULL;

	if (tr.allsolid || tr.fraction == 1.0)
		return;

	if (tr.ent)
		pEntity = gEngfuncs.GetEntityByIndex(gEngfuncs.pEventAPI->EV_IndexFromTrace(&tr));

	CreateDecal(tr.endpos, tr.plane.normal, name, persistent, pEntity);
}


void CreateCachedDecals()
{
	if (!gEngfuncs.GetEntityByIndex(0)->model)
		return;

	for (int i = 0; i < cachedecals; i++)
	{
		CreateDecal(msgcache[i].pos, msgcache[i].normal, msgcache[i].name, msgcache[i].persistent);
	}

	cachedecals = 0;
}


// debugging feature
extern vec3_t	render_origin;
extern vec3_t	render_angles;

void PasteViewDecal()
{
	if (gEngfuncs.Cmd_Argc() <= 1)
	{
		gEngfuncs.Con_Printf("usage: pastedecal <decal name>\n");
		return;
	}

	vec3_t dir;
	AngleVectors(render_angles, dir, NULL, NULL);
	CreateDecalTrace(render_origin, render_origin + (dir * 1024), gEngfuncs.Cmd_Argv(1));
}



// ===========================
// Decals drawing
//
//		Uses surface visframe information from custom renderer's pass
// ===========================

int DrawSingleDecal(customdecal *decal)
{
	if (decal->surface->visframe != framecount)
		return FALSE;

	vec3_t right, up;
	GetUpRight(decal->normal, up, right);

	float texc_orig_x = DotProduct(decal->point, right);
	float texc_orig_y = DotProduct(decal->point, up);
	int xsize = decal->texinfo->xsize;
	int ysize = decal->texinfo->ysize;

	glpoly_t	*p = decal->surface->polys;
	float		*v = p->verts[0];

	vec3_t array1[64];
	vec3_t array2[64];
	for (int j = 0; j<p->numverts; j++, v += VERTEXSIZE)
	{
		VectorCopy(v, array1[j]);
	}

	int nv;
	vec3_t planepoint;
	VectorMA(decal->point, -xsize, right, planepoint);
	nv = ClipPolygonByPlane(array1, p->numverts, right, planepoint, array2);

	VectorMA(decal->point, xsize, right, planepoint);
	nv = ClipPolygonByPlane(array2, nv, right*-1, planepoint, array1);

	VectorMA(decal->point, -ysize, up, planepoint);
	nv = ClipPolygonByPlane(array1, nv, up, planepoint, array2);

	VectorMA(decal->point, ysize, up, planepoint);
	nv = ClipPolygonByPlane(array2, nv, up*-1, planepoint, array1);

	if (!nv)
		return FALSE; // shouldn't happen..

	Bind2DTexture(GL_TEXTURE0_ARB, decal->texinfo->gl_texid);
	gl.glBegin(GL_POLYGON);
	for (int k = 0; k<nv; k++, v += VERTEXSIZE)
	{
		float texc_x = (DotProduct(array1[k], right) - texc_orig_x) / xsize;
		float texc_y = (DotProduct(array1[k], up) - texc_orig_y) / ysize;
		texc_x = (texc_x + 1) / 2;
		texc_y = (texc_y + 1) / 2;

		gl.glMultiTexCoord2fARB(GL_TEXTURE0_ARB, texc_x, texc_y);
		gl.glVertex3fv(array1[k]);
	}
	gl.glEnd();
	return TRUE;
}


void DrawDecals()
{
	decalrendercounter = 0;
	staticdecalrendercounter = 0;

	CreateCachedDecals();

	if ((!numdecals && !numstaticdecals) || !cv_customdecals->value)
		return;

	gl.glEnable(GL_BLEND);
	gl.glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);
	gl.glDepthMask(GL_FALSE);

	gl.glPolygonOffset(-1, -1);
	gl.glEnable(GL_POLYGON_OFFSET_FILL);

	SetTexEnvs(ENVSTATE_REPLACE);

	for (int i = 0; i < numdecals; i++)
	{
		if (DrawSingleDecal(&decals[i]))
			decalrendercounter++;
	}

	for (int i = 0; i < numstaticdecals; i++)
	{
		if (DrawSingleDecal(&staticdecals[i]))
			staticdecalrendercounter++;
	}

	gl.glDisable(GL_POLYGON_OFFSET_FILL);
}

int MsgCustomDecal(const char *pszName, int iSize, void *pbuf)
{
	/*	BEGIN_READ( pbuf, iSize );

	vec3_t pos, normal;
	pos.x = READ_COORD();
	pos.y = READ_COORD();
	pos.z = READ_COORD();
	normal.x = READ_COORD();
	normal.y = READ_COORD();
	normal.z = READ_COORD();
	int persistent = READ_BYTE();

	CreateDecal(pos, normal, READ_STRING(), persistent);
	return 1;*/

	BEGIN_READ(pbuf, iSize);

	vec3_t pos, normal;
	pos.x = READ_COORD();
	pos.y = READ_COORD();
	pos.z = READ_COORD();
	normal.x = READ_COORD();
	normal.y = READ_COORD();
	normal.z = READ_COORD();
	int persistent = READ_BYTE();
	int entindex = READ_SHORT();

	if (entindex > 0)
		CreateDecal(pos, normal, READ_STRING(), persistent, gEngfuncs.GetEntityByIndex(entindex));
	else
		CreateDecal(pos, normal, READ_STRING(), persistent);

	return 1;
}

void DeleteDecals()
{
	numdecals = 0;
	curdecal = 0;
	numstaticdecals = 0;
}

void InitDecals()
{
	gEngfuncs.pfnAddCommand("pastedecal", PasteViewDecal);
	gEngfuncs.pfnAddCommand("decalsclear", DeleteDecals);
	cv_customdecals = gEngfuncs.pfnRegisterVariable("gl_customdecals", "1", 0);
	cv_decalsdebug = gEngfuncs.pfnRegisterVariable("gl_decalstatus", "0", 0);
	gEngfuncs.pfnHookUserMsg("customdecal", MsgCustomDecal);
}

void DecalsShutdown()
{
	if (pDecalGroupList) delete pDecalGroupList;
}

void DecalsPrintDebugInfo()
{
	if (cv_decalsdebug->value)
	{
		char msg[256];
		sprintf(msg, "%d decals in memory, %d rendered\n", numdecals, decalrendercounter);
		DrawConsoleString(XRES(10), YRES(100), msg);
		sprintf(msg, "%d static decals in memory, %d rendered\n", numstaticdecals, staticdecalrendercounter);
		DrawConsoleString(XRES(10), YRES(115), msg);
	}
}