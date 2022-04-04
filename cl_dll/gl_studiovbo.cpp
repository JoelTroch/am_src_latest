#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "com_model.h"
#include "glmanager.h"
#include "gl_texloader.h"

#include "studio_util.h"
#include "r_studioint.h"

#include "r_efx.h"
#include "event_api.h"
#include "pm_defs.h"
#include "pmtrace.h"
#include "gl_renderer.h"
#include "gl_dlight.h" // buz

#include "studio.h"
#include "StudioModelRenderer.h"
#include "GameStudioModelRenderer.h"

#include <map>
#include "gl_studiovbo.h"

GLvoid *TEXCOORDOFFSET = (GLvoid*)sizeof(Vector);
GLvoid *NORMALOFFSET = (GLvoid*)(sizeof(Vector)+sizeof(Vector2D)+sizeof(float));
GLvoid *BONEIDOFFSET = (GLvoid*)(sizeof(Vector)+sizeof(Vector2D));
GLvoid *TANGENTOFFSET = (GLvoid*)(sizeof(Vector)+sizeof(Vector2D)+sizeof(float) + sizeof(Vector));
GLvoid *BINORMALOFFSET = (GLvoid*)(sizeof(Vector)+sizeof(Vector2D)+sizeof(float)*4+sizeof(float)*4);

GLvoid *TEXCOORDOFFSETPROP = (GLvoid*)sizeof(Vector);
GLvoid *NORMALOFFSETPROP = (GLvoid*)(sizeof(Vector)+sizeof(Vector2D));
GLvoid *TANGENTOFFSETPROP = (GLvoid*)(sizeof(Vector)+sizeof(Vector2D)+sizeof(Vector));
GLvoid *LIGHTINGOFFSETPROP = (GLvoid*)(sizeof(Vector)+sizeof(Vector2D)+sizeof(Vector)+sizeof(float)*4);

unsigned int		m_arrayelems[MAXARRAYVERTS * 6];
Vector		tan1[MAXARRAYVERTS * 6];
Vector		tan2[MAXARRAYVERTS * 6];
float		tangents[MAXARRAYVERTS][3];

Vector		normals[MAXARRAYVERTS];
Vector		avs[MAXARRAYVERTS];

studiovertex_t vertexes[MAXARRAYVERTS];
studiovertexprop_t vertexesprop[MAXARRAYVERTS];

int numvbos = 0;
studiovbo_t *vbos[2048];

void initStudioVBO()
{
	for (int i = 0; i < numvbos; i++)
	{
		if (vbos[i])
		{
			gl.glDeleteBuffersARB(1, &vbos[i]->iboId);
			gl.glDeleteBuffersARB(1, &vbos[i]->vboId);
			free(vbos[i]);
		}
	}
	numvbos = 0;
}

studiovbo_t *findVbo(mstudiomesh_t *mesh)
{
	if (!mesh) return NULL;
	for (int i = 0; i < numvbos; i++)
	{
		if (vbos[i] && vbos[i]->mesh == mesh)
			return vbos[i];
	}

	return NULL;
}

studiovbo_t *findVboProp(mstudiomesh_t *mesh,cl_entity_t *ent)
{
	if (!mesh || !ent) return NULL;
	for (int i = 0; i < numvbos; i++)
	{
		if (vbos[i] && vbos[i]->mesh == mesh && vbos[i]->entindex == ent->index)
			return vbos[i];
	}

	return NULL;
}

extern CGameStudioModelRenderer g_StudioRenderer;
void generateVertexes(mstudiomesh_t *mesh, studiovbo_t *vbo)
{
	if (!mesh || !vbo)
		return;
	int cntv = 0;
	short *ptricmds = vbo->ptricmds;
	int m_nNumArrayElems = 0;
	int m_nNumArrayVerts = 0;

	int i;
	int vertexSize = sizeof(vertexes[0]);
	int texCoordOffset = sizeof(vertexes[0].vertex);

	while (i = *(ptricmds++))
	{
		int	vertexState = 0;
		bool	tri_strip;

		if (i < 0)
		{
			tri_strip = false;
			i = -i;
		}
		else
			tri_strip = true;

		cntv += (i - 2);

		for (; i > 0; i--, ptricmds += 4)
		{
			Vector2D	uv;
			float s, t;
			s = 1.0f / (float)vbo->texture->width;
			t = 1.0f / (float)vbo->texture->height;
			uv.x = ptricmds[2] * s;
			uv.y = ptricmds[3] * t;
			Vector av = vbo->g_pxformverts[ptricmds[0]];

			if (vertexState++ < 3)
			{
				m_arrayelems[m_nNumArrayElems++] = m_nNumArrayVerts;
			}
			else if (tri_strip)
			{
				// flip triangles between clockwise and counter clockwise
				if (vertexState & 1)
				{
					// draw triangle [n-2 n-1 n]
					m_arrayelems[m_nNumArrayElems++] = m_nNumArrayVerts - 2;
					m_arrayelems[m_nNumArrayElems++] = m_nNumArrayVerts - 1;
					m_arrayelems[m_nNumArrayElems++] = m_nNumArrayVerts;
				}
				else
				{
					// draw triangle [n-1 n-2 n]
					m_arrayelems[m_nNumArrayElems++] = m_nNumArrayVerts - 1;
					m_arrayelems[m_nNumArrayElems++] = m_nNumArrayVerts - 2;
					m_arrayelems[m_nNumArrayElems++] = m_nNumArrayVerts;
				}
			}
			else
			{
				// draw triangle fan [0 n-1 n]
				m_arrayelems[m_nNumArrayElems++] = m_nNumArrayVerts - (vertexState - 1);
				m_arrayelems[m_nNumArrayElems++] = m_nNumArrayVerts - 1;
				m_arrayelems[m_nNumArrayElems++] = m_nNumArrayVerts;
			}

			vertexes[m_nNumArrayVerts].vertex = av;
			vertexes[m_nNumArrayVerts].texcoord = uv;
			vertexes[m_nNumArrayVerts].boneid = (float)vbo->pvertbones[ptricmds[0]];
			vertexes[m_nNumArrayVerts].normal[0] = vbo->pstudionorms[ptricmds[1]].x;
			vertexes[m_nNumArrayVerts].normal[1] = vbo->pstudionorms[ptricmds[1]].y;
			vertexes[m_nNumArrayVerts].normal[2] = vbo->pstudionorms[ptricmds[1]].z;

			vertexes[m_nNumArrayVerts].index = m_arrayelems[m_nNumArrayElems];
			vertexes[m_nNumArrayVerts].normalidx = ptricmds[i];
			m_nNumArrayVerts++;
		}
	}

	// Calculate tangents
	vec3_t *s_tangents = (vec3_t *)malloc(sizeof(vec3_t)*m_nNumArrayVerts);
	memset(s_tangents, 0, sizeof(vec3_t)*m_nNumArrayVerts);

	vec3_t *t_tangents = (vec3_t *)malloc(sizeof(vec3_t)*m_nNumArrayVerts);
	memset(t_tangents, 0, sizeof(vec3_t)*m_nNumArrayVerts);

	for (int l = 0; l < m_nNumArrayElems; l += 3)
	{
		studiovertex_t *v0 = &vertexes[m_arrayelems[l]];
		studiovertex_t *v1 = &vertexes[m_arrayelems[l + 1]];
		studiovertex_t *v2 = &vertexes[m_arrayelems[l + 2]];

		float x1 = v1->normal.x - v0->normal.x;
		float x2 = v2->normal.x - v0->normal.x;
		float y1 = v1->normal.y - v0->normal.y;
		float y2 = v2->normal.y - v0->normal.y;
		float z1 = v1->normal.z - v0->normal.z;
		float z2 = v2->normal.z - v0->normal.z;

		float s1 = v1->texcoord.x - v0->texcoord.x;
		float s2 = v2->texcoord.x - v0->texcoord.x;
		float t1 = v1->texcoord.y - v0->texcoord.y;
		float t2 = v2->texcoord.y - v0->texcoord.y;

		float div = (s1 * t2 - s2 * t1);
		float r = div == 0.0f ? 0.0F : 1.0F / div;

		vec3_t sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r);
		vec3_t tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r);

		VectorAdd(s_tangents[m_arrayelems[l]], sdir, s_tangents[m_arrayelems[l]]);
		VectorAdd(s_tangents[m_arrayelems[l + 1]], sdir, s_tangents[m_arrayelems[l + 1]]);
		VectorAdd(s_tangents[m_arrayelems[l + 2]], sdir, s_tangents[m_arrayelems[l + 2]]);

		VectorAdd(t_tangents[m_arrayelems[l]], tdir, t_tangents[m_arrayelems[l]]);
		VectorAdd(t_tangents[m_arrayelems[l + 1]], tdir, t_tangents[m_arrayelems[l + 1]]);
		VectorAdd(t_tangents[m_arrayelems[l + 2]], tdir, t_tangents[m_arrayelems[l + 2]]);
	}

	for (int i = 0; i < m_nNumArrayVerts; i++)
	{
		vec3_t tangent = (s_tangents[i] - vertexes[i].normal * DotProduct(vertexes[i].normal, s_tangents[i]));
		VectorNormalize(tangent);

		for (int j = 0; j < 3;j++)
		vertexes[i].tangent[j] = tangent[j];

		vec3_t vCross = CrossProduct(vertexes[i].normal, s_tangents[i]);
		float flDot = DotProduct(vCross, t_tangents[i]);
		vertexes[i].tangent[3] = (flDot >= 0.0) ? 1.0 : -1.0;
	}

	// Free data
	free(s_tangents);
	free(t_tangents);

	vbo->indexCount = m_nNumArrayElems;
	gl.glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo->vboId);
	gl.glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(studioVertex)*m_nNumArrayVerts,vertexes,GL_STATIC_DRAW_ARB);
	gl.glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

	gl.glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, vbo->iboId);
	gl.glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*m_nNumArrayElems, &m_arrayelems[0], GL_STATIC_DRAW_ARB);
	gl.glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void generateVertexesProp(mstudiomesh_t *mesh, studiovbo_t *vbo)
{
	if (!mesh || !vbo)
		return;
	int cntv = 0;
	short *ptricmds = vbo->ptricmds;
	int m_nNumArrayElems = 0;
	int m_nNumArrayVerts = 0;

	int i;
	int vertexSize = sizeof(vertexesprop[0]);
	int texCoordOffset = sizeof(vertexesprop[0].vertex);

	while (i = *(ptricmds++))
	{
		int	vertexState = 0;
		bool	tri_strip;

		if (i < 0)
		{
			tri_strip = false;
			i = -i;
		}
		else
			tri_strip = true;

		cntv += (i - 2);

		for (; i > 0; i--, ptricmds += 4)
		{
			Vector2D	uv;
			float s, t;
			s = 1.0f / (float)vbo->texture->width;
			t = 1.0f / (float)vbo->texture->height;
			uv.x = ptricmds[2] * s;
			uv.y = ptricmds[3] * t;
			Vector av = vbo->g_pxformverts[ptricmds[0]];

			if (vertexState++ < 3)
			{
				m_arrayelems[m_nNumArrayElems++] = m_nNumArrayVerts;
			}
			else if (tri_strip)
			{
				// flip triangles between clockwise and counter clockwise
				if (vertexState & 1)
				{
					// draw triangle [n-2 n-1 n]
					m_arrayelems[m_nNumArrayElems++] = m_nNumArrayVerts - 2;
					m_arrayelems[m_nNumArrayElems++] = m_nNumArrayVerts - 1;
					m_arrayelems[m_nNumArrayElems++] = m_nNumArrayVerts;
				}
				else
				{
					// draw triangle [n-1 n-2 n]
					m_arrayelems[m_nNumArrayElems++] = m_nNumArrayVerts - 1;
					m_arrayelems[m_nNumArrayElems++] = m_nNumArrayVerts - 2;
					m_arrayelems[m_nNumArrayElems++] = m_nNumArrayVerts;
				}
			}
			else
			{
				// draw triangle fan [0 n-1 n]
				m_arrayelems[m_nNumArrayElems++] = m_nNumArrayVerts - (vertexState - 1);
				m_arrayelems[m_nNumArrayElems++] = m_nNumArrayVerts - 1;
				m_arrayelems[m_nNumArrayElems++] = m_nNumArrayVerts;
			}

			vertexesprop[m_nNumArrayVerts].vertex = av;
			vertexesprop[m_nNumArrayVerts].texcoord = uv;
			
			lighting_ext lightinfo;
			EXT_LightPointStudioProp(av, vbo->pstudionorms[ptricmds[1]], &lightinfo, 0);

			float lightcos;
			Vector dir = lightinfo.lightdir;
			VectorRotateSSE(dir, (*g_StudioRenderer.m_pbonetransform)[vbo->pvertbones[ptricmds[0]]], dir);
			DotProductSSE(&lightcos, vbo->pstudionorms[ptricmds[1]], dir);

			if (lightcos > 1.0)
				lightcos = 1;

			Vector illum = lightinfo.ambientlight;
			VectorAddSSE(illum, lightinfo.addlight, illum);

			float r = cv_lambert->value;
			if (r <= 1.0) r = 1.0;

			lightcos = (lightcos + (r - 1.0)) / r; 
			if (lightcos > 0.0)
				VectorMASSE(illum, -lightcos, lightinfo.addlight, illum);

			if (illum[0] <= 0) illum[0] = 0;
			if (illum[1] <= 0) illum[1] = 0;
			if (illum[2] <= 0) illum[2] = 0;

			for (int m = 0; m < 3; m++){
				vertexesprop[m_nNumArrayVerts].lighting[m] = ApplyGamma(illum[m])*0.75;
			}

			vertexesprop[m_nNumArrayVerts].normal[0] = vbo->pstudionorms[ptricmds[1]].x;
			vertexesprop[m_nNumArrayVerts].normal[1] = vbo->pstudionorms[ptricmds[1]].y;
			vertexesprop[m_nNumArrayVerts].normal[2] = vbo->pstudionorms[ptricmds[1]].z;

			m_nNumArrayVerts++;
		}
	}

	// Calculate tangents
	vec3_t *s_tangents = (vec3_t *)malloc(sizeof(vec3_t)*m_nNumArrayVerts);
	memset(s_tangents, 0, sizeof(vec3_t)*m_nNumArrayVerts);

	vec3_t *t_tangents = (vec3_t *)malloc(sizeof(vec3_t)*m_nNumArrayVerts);
	memset(t_tangents, 0, sizeof(vec3_t)*m_nNumArrayVerts);

	for (int l = 0; l < m_nNumArrayElems; l += 3)
	{
		studiovertexprop_t *v0 = &vertexesprop[m_arrayelems[l]];
		studiovertexprop_t *v1 = &vertexesprop[m_arrayelems[l + 1]];
		studiovertexprop_t *v2 = &vertexesprop[m_arrayelems[l + 2]];

		float x1 = v1->normal.x - v0->normal.x;
		float x2 = v2->normal.x - v0->normal.x;
		float y1 = v1->normal.y - v0->normal.y;
		float y2 = v2->normal.y - v0->normal.y;
		float z1 = v1->normal.z - v0->normal.z;
		float z2 = v2->normal.z - v0->normal.z;

		float s1 = v1->texcoord.x - v0->texcoord.x;
		float s2 = v2->texcoord.x - v0->texcoord.x;
		float t1 = v1->texcoord.y - v0->texcoord.y;
		float t2 = v2->texcoord.y - v0->texcoord.y;

		float div = (s1 * t2 - s2 * t1);
		float r = div == 0.0f ? 0.0F : 1.0F / div;

		vec3_t sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r);
		vec3_t tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r);

		VectorAdd(s_tangents[m_arrayelems[l]], sdir, s_tangents[m_arrayelems[l]]);
		VectorAdd(s_tangents[m_arrayelems[l + 1]], sdir, s_tangents[m_arrayelems[l + 1]]);
		VectorAdd(s_tangents[m_arrayelems[l + 2]], sdir, s_tangents[m_arrayelems[l + 2]]);

		VectorAdd(t_tangents[m_arrayelems[l]], tdir, t_tangents[m_arrayelems[l]]);
		VectorAdd(t_tangents[m_arrayelems[l + 1]], tdir, t_tangents[m_arrayelems[l + 1]]);
		VectorAdd(t_tangents[m_arrayelems[l + 2]], tdir, t_tangents[m_arrayelems[l + 2]]);
	}

	for (int i = 0; i < m_nNumArrayVerts; i++)
	{
		vec3_t tangent = (s_tangents[i] - vertexesprop[i].normal * DotProduct(vertexesprop[i].normal, s_tangents[i]));
		VectorNormalize(tangent);

		for (int j = 0; j < 3; j++)
			vertexesprop[i].tangent[j] = tangent[j];

		vec3_t vCross = CrossProduct(vertexesprop[i].normal, s_tangents[i]);
		float flDot = DotProduct(vCross, t_tangents[i]);
		vertexesprop[i].tangent[3] = (flDot >= 0.0) ? 1.0 : -1.0;
	}

	// Free data
	free(s_tangents);
	free(t_tangents);
	//gEngfuncs.Con_Printf("Done m_nNumArrayVerts = %i\n", m_nNumArrayVerts);

	vbo->indexCount = m_nNumArrayElems;
	gl.glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo->vboId);
	gl.glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(studioVertexProp)*m_nNumArrayVerts, vertexesprop, GL_STATIC_DRAW_ARB);
	gl.glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

	gl.glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, vbo->iboId);
	gl.glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*m_nNumArrayElems, &m_arrayelems[0], GL_STATIC_DRAW_ARB);
	gl.glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, 0);
}

studiovbo_t *getVbo(cl_entity_t *ent, mstudiomesh_t *mesh, short *ptricmds, mstudiotexture_t *texture, Vector *g_pxformverts, byte *pvertbones, Vector *pstudionorms,byte *pnormbones)
{
	if (!mesh || !ent)
		return NULL;
	
	studiovbo_t *returnVbo = findVbo(mesh);
	if (returnVbo)
		return returnVbo;

	if (!mesh || !ptricmds || !texture || !g_pxformverts || !pnormbones || !pvertbones)
		return NULL;

	bool staticModel = (ent->curstate.controller[3] == 12);
	//gEngfuncs.Con_Printf("Colormap = %i\n", ent->curstate.controller[3]);

	returnVbo = (studiovbo_t*)malloc(sizeof(studioVbo));
	returnVbo->mesh = mesh;
	returnVbo->extTextData = NULL;//for now
	returnVbo->ptricmds = ptricmds;
	returnVbo->texture = texture;
	returnVbo->g_pxformverts = g_pxformverts;
	returnVbo->pstudionorms = pstudionorms;
	returnVbo->pnormbones = pnormbones;
	returnVbo->prop = 0;

	GLuint id,iboId;
	gl.glGenBuffersARB(1, &id);
	gl.glGenBuffersARB(1, &iboId);

	returnVbo->vboId = id;
	returnVbo->iboId = iboId;
	returnVbo->pvertbones = pvertbones;
	//gEngfuncs.Con_Printf("Generating vbo %i\n", id);
	//gEngfuncs.Con_Printf("Generating ibo %i\n", iboId);

	generateVertexes(mesh, returnVbo);
	vbos[numvbos++] = returnVbo;
	returnVbo->triindex = mesh->triindex;
	returnVbo->numtris = mesh->numtris;
	return returnVbo;
}


studiovbo_t *getVboProp(cl_entity_t *ent, mstudiomesh_t *mesh, short *ptricmds, mstudiotexture_t *texture, Vector *g_pxformverts, byte *pvertbones, Vector *pstudionorms, byte *pnormbones)
{
	if (!mesh || !ent)
		return NULL;

	studiovbo_t *returnVbo = findVboProp(mesh,ent);
	if (returnVbo)
		return returnVbo;

	if (!mesh || !ptricmds || !texture || !g_pxformverts || !pnormbones || !pvertbones)
		return NULL;

	bool staticModel = (ent->curstate.controller[3]==12);
	if (!staticModel) return NULL;

	//gEngfuncs.Con_Printf("Colormap = %i\n", ent->curstate.controller[3]);

	returnVbo = (studiovbo_t*)malloc(sizeof(studioVbo));
	returnVbo->mesh = mesh;
	returnVbo->extTextData = NULL;//for now
	returnVbo->ptricmds = ptricmds;
	returnVbo->texture = texture;
	returnVbo->g_pxformverts = g_pxformverts;
	returnVbo->pstudionorms = pstudionorms;
	returnVbo->pnormbones = pnormbones;
	returnVbo->entindex = ent->index;
	returnVbo->prop = staticModel;

	GLuint id, iboId;
	gl.glGenBuffersARB(1, &id);
	gl.glGenBuffersARB(1, &iboId);

	returnVbo->vboId = id;
	returnVbo->iboId = iboId;
	returnVbo->pvertbones = pvertbones;

	//gEngfuncs.Con_Printf("Generating prop vbo %i\n", id);
	//gEngfuncs.Con_Printf("Generating prop ibo %i\n", iboId);

	generateVertexesProp(mesh, returnVbo);
	vbos[numvbos++] = returnVbo;
	returnVbo->triindex = mesh->triindex;
	returnVbo->numtris = mesh->numtris;
	return returnVbo;
}
