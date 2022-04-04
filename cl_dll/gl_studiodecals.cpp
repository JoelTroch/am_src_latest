/*
	IT'S THE PART OF TRINITY RENDERER
	HAWK: Sorry, Andrew, I have no time for write this by myself.
*/

#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "com_model.h"
#include "studio.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "dlight.h"
#include "triangleapi.h"

#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <math.h>

#include "studio_util.h"
#include "r_studioint.h"

#include "StudioModelRenderer.h"
#include "GameStudioModelRenderer.h"

#include "gl_renderer.h"
#include "gl_dlight.h"

#include "gl_studiovbo.h"
#include "gl_shaders.h"

#include "pm_defs.h"
#include "pmtrace.h"

#include "gl_texloader.h"
#include "gl_decals.h"

#define MAX_MODEL_DECALS_VERTICES 4096

extern engine_studio_api_t IEngineStudio;
vec3_t m_vVertexTransform[MAXSTUDIOTRIANGLES];
void VectorIRotate(const vec3_t &in1, const float in2[3][4], vec3_t &out);

byte *ResizeArray(byte *pOriginal, int iSize, int iCount)
{
	byte *pArray = new byte[iSize*(iCount + 1)];
	memset(pArray, 0, sizeof(byte)*iSize*(iCount + 1));

	if (pOriginal && iCount)
	{
		memmove(pArray, pOriginal, iSize*iCount);
		delete[] pOriginal;
	}

	return pArray;
}

/*
====================
StudioAllocDecal

====================
*/
studiodecal_t *CStudioModelRenderer::StudioAllocDecal(void)
{
	if (!m_pCurrentEntity->efrag)
	{
		studiodecal_t *pDecal = StudioAllocDecalSlot();
		pDecal->totaldecals = 1;

		m_pCurrentEntity->efrag = (struct efrag_s *)pDecal;
		return pDecal;
	}

	// What this code does is basically set up a linked list as long
	// as it can, and once the max amount of decals have been reached
	// it starts recursing again, replacing each original decal. 
	studiodecal_t *pfirst = (studiodecal_t *)m_pCurrentEntity->efrag;
	studiodecal_t *pnext = pfirst;

	if (pfirst->totaldecals == MAX_MODEL_DECALS)
		pfirst->totaldecals = 0;

	for (int i = 0; i < MAX_MODEL_DECALS; i++)
	{
		if (i == pfirst->totaldecals)
		{
			pfirst->totaldecals++;

			if (pnext->numverts)
			{
				delete[] pnext->verts;
				pnext->verts = NULL;
				pnext->numverts = 0;
			}

			if (pnext->numpolys)
			{
				for (int k = 0; k < pnext->numpolys; k++)
					delete[] pnext->polys[k].verts;

				delete[] pnext->polys;
				pnext->polys = NULL;
				pnext->numpolys = 0;
			}

			return pnext;
		}

		if (!pnext->next)
		{
			studiodecal_t *pDecal = StudioAllocDecalSlot();
			pnext->next = pDecal;
			pfirst->totaldecals++;

			return pDecal;
		}

		studiodecal_t *next = pnext->next;
		pnext = next;
	}
	return NULL;
}

/*
====================
StudioDecalForEntity

====================
*/
void CStudioModelRenderer::StudioDecalForEntity(vec3_t position, vec3_t normal, const char *szName, cl_entity_t *pEntity)
{
	if (!pEntity)
		return;

	if (!pEntity->model)
		return;

	if (pEntity->model->type != mod_studio)
		return;

	if (pEntity == gEngfuncs.GetViewModel())
		return;

	int countVerts = countDecalVertices(pEntity);
	if (countVerts == -1 || countVerts >= MAX_MODEL_DECALS_VERTICES)
		return;

	DecalGroup *group = FindGroup(szName);

	if (!group)
		return;

	const DecalGroupEntry *texptr = group->GetRandomDecal();

	if (!texptr)
		return;

	m_pCurrentEntity = pEntity;
	m_pRenderModel = pEntity->model;
	m_pStudioHeader = (studiohdr_t *)IEngineStudio.Mod_Extradata(m_pRenderModel);

	studiodecal_t *pDecal = StudioAllocDecal();

	if (!pDecal)
		return;

	pDecal->entindex = m_pCurrentEntity->index;
	pDecal->texture = texptr;

	GL_SetupTextureHeader();
	StudioSetUpTransform(0);
	StudioSetupBones();

	for (int i = 0; i < m_pStudioHeader->numbodyparts; i++)
	{
		GL_SetupModel(i);
		StudioDecalForSubModel(position, normal, pDecal);
	}
}


/*
====================
StudioDecalForSubModel

====================
*/
void CStudioModelRenderer::StudioDecalForSubModel(vec3_t position, vec3_t normal, studiodecal_t *decal)
{
	byte *pvertbone = ((byte *)m_pStudioHeader + m_pSubModel->vertinfoindex);
	vec3_t *pstudioverts = (vec3_t *)((byte *)m_pStudioHeader + m_pSubModel->vertindex);
	
	for (int i = 0; i < m_pSubModel->numverts; i++)
		VectorTransformSSE(pstudioverts[i], (*m_pbonetransform)[pvertbone[i]], m_vVertexTransform[i]);

	for (int i = 0; i < m_pSubModel->nummesh; i++)
	{
		mstudiomesh_t *pmesh = (mstudiomesh_t *)((byte *)m_pStudioHeader + m_pSubModel->meshindex) + i;
		short *ptricmds = (short *)((byte *)m_pStudioHeader + pmesh->triindex);

		int j;
		while (j = *(ptricmds++))
		{
			if (j > 0)
			{
				// convert triangle strip
				j -= 3;
				studiotri_t triangle;
				triangle.verts[0].vertindex = ptricmds[0];
				triangle.verts[0].boneindex = pvertbone[ptricmds[0]];
				ptricmds += 4;

				triangle.verts[1].vertindex = ptricmds[0];
				triangle.verts[1].boneindex = pvertbone[ptricmds[0]];
				ptricmds += 4;

				triangle.verts[2].vertindex = ptricmds[0];
				triangle.verts[2].boneindex = pvertbone[ptricmds[0]];
				ptricmds += 4;

				StudioDecalTriangle(&triangle, position, normal, decal);

				bool reverse = false;
				for (; j > 0; j--, ptricmds += 4)
				{
					studiotri_t tritemp;
					triangle.verts[0] = triangle.verts[1];
					triangle.verts[1] = triangle.verts[2];

					triangle.verts[2].vertindex = ptricmds[0];
					triangle.verts[2].boneindex = pvertbone[ptricmds[0]];

					if (!reverse)
					{
						tritemp.verts[0] = triangle.verts[2];
						tritemp.verts[1] = triangle.verts[1];
						tritemp.verts[2] = triangle.verts[0];
					}
					else
					{
						tritemp.verts[0] = triangle.verts[0];
						tritemp.verts[1] = triangle.verts[1];
						tritemp.verts[2] = triangle.verts[2];
					}
					StudioDecalTriangle(&tritemp, position, normal, decal);
					reverse = !reverse;
				}
			}
			else
			{
				// convert triangle fan
				j = -j - 3;
				studiotri_t triangle;
				triangle.verts[0].vertindex = ptricmds[0];
				triangle.verts[0].boneindex = pvertbone[ptricmds[0]];
				ptricmds += 4;

				triangle.verts[1].vertindex = ptricmds[0];
				triangle.verts[1].boneindex = pvertbone[ptricmds[0]];
				ptricmds += 4;

				triangle.verts[2].vertindex = ptricmds[0];
				triangle.verts[2].boneindex = pvertbone[ptricmds[0]];
				ptricmds += 4;

				StudioDecalTriangle(&triangle, position, normal, decal);

				for (; j > 0; j--, ptricmds += 4)
				{
					triangle.verts[1] = triangle.verts[2];
					triangle.verts[2].vertindex = ptricmds[0];
					triangle.verts[2].boneindex = pvertbone[ptricmds[0]];

					StudioDecalTriangle(&triangle, position, normal, decal);
				}
			}
		}
	}
}


/*
====================
StudioDecalTriangle

====================
*/
void CStudioModelRenderer::StudioDecalTriangle(studiotri_t *tri, vec3_t position, vec3_t normal, studiodecal_t *decal)
{
	vec3_t dverts1[10];
	vec3_t dverts2[10];

	vec3_t norm, v1, v2, v3;
	VectorSubtract(m_vVertexTransform[tri->verts[1].vertindex], m_vVertexTransform[tri->verts[0].vertindex], v1);
	VectorSubtract(m_vVertexTransform[tri->verts[2].vertindex], m_vVertexTransform[tri->verts[1].vertindex], v2);
	CrossProduct(v2, v1, norm);

	if (DotProduct(normal, norm) < 0.0)
		return;

	vec3_t  right, up;
	GetUpRight(normal, up, right);
	float texc_orig_x = DotProduct(position, right);
	float texc_orig_y = DotProduct(position, up);

	int xsize = decal->texture->xsize;
	int ysize = decal->texture->ysize;

	for (int i = 0; i < 3; i++)
		VectorCopy(m_vVertexTransform[tri->verts[i].vertindex], dverts1[i]);

	int nv;
	vec3_t planepoint;
	VectorMASSE(position, -xsize, right, planepoint);
	nv = ClipPolygonByPlane(dverts1, 3, right, planepoint, dverts2);

	if (nv < 3)
		return;

	VectorMASSE(position, xsize, right, planepoint);
	nv = ClipPolygonByPlane(dverts2, nv, right*-1, planepoint, dverts1);

	if (nv < 3)
		return;

	VectorMASSE(position, -ysize, up, planepoint);
	nv = ClipPolygonByPlane(dverts1, nv, up, planepoint, dverts2);

	if (nv < 3)
		return;

	VectorMASSE(position, ysize, up, planepoint);
	nv = ClipPolygonByPlane(dverts2, nv, up*-1, planepoint, dverts1);

	if (nv < 3)
		return;

	// Only allow cut polys if the poly is only transformed by one bone
	if (nv > 3 && (tri->verts[0].boneindex != tri->verts[1].boneindex
		|| tri->verts[0].boneindex != tri->verts[2].boneindex
		|| tri->verts[1].boneindex != tri->verts[2].boneindex))
		return;

	// Check if the poly was cut
	if ((dverts1[0] != m_vVertexTransform[tri->verts[2].vertindex]
		|| dverts1[1] != m_vVertexTransform[tri->verts[0].vertindex]
		|| dverts1[2] != m_vVertexTransform[tri->verts[1].vertindex])
		&& (tri->verts[0].boneindex != tri->verts[1].boneindex
		|| tri->verts[0].boneindex != tri->verts[2].boneindex
		|| tri->verts[1].boneindex != tri->verts[2].boneindex))
		return;

	byte indexes[10];
	if (nv == 3 && dverts1[0] == m_vVertexTransform[tri->verts[2].vertindex]
		&& dverts1[1] == m_vVertexTransform[tri->verts[0].vertindex]
		&& dverts1[2] == m_vVertexTransform[tri->verts[1].vertindex])
	{
		indexes[0] = tri->verts[2].boneindex;
		indexes[1] = tri->verts[0].boneindex;
		indexes[2] = tri->verts[1].boneindex;
	}
	else
	{
		for (int i = 0; i < nv; i++)
			indexes[i] = tri->verts[0].boneindex;
	}

	decal->polys = (decalpoly_t *)ResizeArray((byte *)decal->polys, sizeof(decalpoly_t), decal->numpolys);
	decalpoly_t *polygon = &decal->polys[decal->numpolys]; decal->numpolys++;

	polygon->verts = new decalvert_t[nv];
	polygon->numverts = nv;

	for (int i = 0; i < nv; i++)
	{
		float texc_x = (DotProduct(dverts1[i], right) - texc_orig_x) / xsize;
		float texc_y = (DotProduct(dverts1[i], up) - texc_orig_y) / ysize;
		polygon->verts[i].texcoord[0] = ((texc_x + 1) / 2);
		polygon->verts[i].texcoord[1] = ((texc_y + 1) / 2);

		vec3_t temp, fpos; //PINGAS
		temp[0] = dverts1[i][0] - (*m_pbonetransform)[indexes[i]][0][3];
		temp[1] = dverts1[i][1] - (*m_pbonetransform)[indexes[i]][1][3];
		temp[2] = dverts1[i][2] - (*m_pbonetransform)[indexes[i]][2][3];
		VectorIRotate(temp, (*m_pbonetransform)[indexes[i]], fpos);

		int j = 0;
		for (; j < decal->numverts; j++)
		{
			if (decal->verts[j].position == fpos)
			{
				polygon->verts[i].vertindex = j;
				break;
			}
		}

		if (j == decal->numverts)
		{
			decal->verts = (decalvertinfo_t *)ResizeArray((byte *)decal->verts, sizeof(decalvertinfo_t), decal->numverts);
			decal->verts[decal->numverts].boneindex = indexes[i];
			decal->verts[decal->numverts].position = fpos;

			polygon->verts[i].vertindex = decal->numverts;
			decal->numverts++;
		}
	}
}


/*
====================
StudioAllocDecalSlot

====================
*/
studiodecal_t *CStudioModelRenderer::StudioAllocDecalSlot(void)
{
	if (m_iNumStudioDecals == MAX_CUSTOMDECALS)
		m_iNumStudioDecals = NULL;

	studiodecal_t *pDecal = &m_pStudioDecals[m_iNumStudioDecals];
	m_iNumStudioDecals++;

	if (pDecal->numverts)
	{
		delete[] pDecal->verts;
		pDecal->verts = NULL;
		pDecal->numverts = 0;
	}

	if (pDecal->numpolys)
	{
		for (int i = 0; i < pDecal->numpolys; i++)
			delete[] pDecal->polys[i].verts;

		delete[] pDecal->polys;
		pDecal->polys = NULL;
		pDecal->numpolys = 0;
	}

	// Make sure nothing references this decal
	for (int i = 0; i < m_iNumStudioDecals; i++)
	{
		if (m_pStudioDecals[i].next == pDecal)
			m_pStudioDecals[i].next = pDecal->next;
	}

	memset(pDecal, 0, sizeof(studiodecal_t));
	return pDecal;
};


/*
====================
StudioDrawDecals

====================
*/
void CStudioModelRenderer::StudioDrawDecals(void)
{
	if (!m_pCurrentEntity->efrag)
		return;

	// Just to make sure
	if (m_pCurrentEntity == gEngfuncs.GetViewModel())
		return;

	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);
	glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);
	glPolygonOffset(-1, -1);
	glEnable(GL_POLYGON_OFFSET_FILL);

	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.1);

	gl.glActiveTextureARB(GL_TEXTURE0_ARB);
	glEnable(GL_TEXTURE_2D);

	studiodecal_t *pnext = (studiodecal_t *)m_pCurrentEntity->efrag;
	while (pnext)
	{
		glBindTexture(GL_TEXTURE_2D, pnext->texture->gl_texid);

		for (int i = 0; i < pnext->numverts; i++)
		{
			if (pnext->verts[i].boneindex > m_pStudioHeader->numbones)
			{
				goto resetgl;// every time this happens, i shit bricks
				return;
			}

			VectorTransformSSE(pnext->verts[i].position, (*m_pbonetransform)[pnext->verts[i].boneindex], m_vVertexTransform[i]);
		}

		for (int i = 0; i < pnext->numpolys; i++)
		{
			decalvert_t *verts = &pnext->polys[i].verts[0];
			glBegin(GL_POLYGON);
			for (int j = 0; j < pnext->polys[i].numverts; j++)
			{
				glTexCoord2f(verts[j].texcoord[0], verts[j].texcoord[1]);
				glVertex3fv(m_vVertexTransform[verts[j].vertindex]);
			}
			glEnd();
		}

		studiodecal_t *next = pnext->next;
		pnext = next;
	}

resetgl:
	glDisable(GL_POLYGON_OFFSET_FILL);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	glAlphaFunc(GL_GREATER, 0);
}

int CStudioModelRenderer::countDecalVertices(cl_entity_t *ent){
	if (!ent) return -1;
	if (!ent->efrag)
		return 0;

	// Just to make sure
	if (ent == gEngfuncs.GetViewModel())
		return -1;

	int total = 0;
	studiodecal_t *pnext = (studiodecal_t *)ent->efrag;
	while (pnext)
	{
		total += pnext->numverts;

		studiodecal_t *next = pnext->next;
		pnext = next;
	}
	return total;
}