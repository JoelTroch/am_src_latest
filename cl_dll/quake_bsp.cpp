
//===============================
// function for working with bsp levels, based on code from QW
//===============================

#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "cdll_int.h"
#include "com_model.h"
#include "gl_renderer.h"



// 0  - success
// -1 - failed
int RecursiveLightPoint (mnode_t *node, const vec3_t &start, const vec3_t &end, vec3_t &outcolor)
{
	int			r;
	float		front, back, frac;
	int			side;
	mplane_t	*plane;
	vec3_t		mid;
	msurface_t	*surf;
	int			s, t, ds, dt;
	int			i;
	mtexinfo_t	*tex;
//	byte		*lightmap;
	color24		*lightmap; // buz
//	unsigned	scale;
//	int			maps;

	if (node->contents < 0)
		return -1;		// didn't hit anything
	
// calculate mid point

// FIXME: optimize for axial
	plane = node->plane;
	front = DotProduct (start, plane->normal) - plane->dist;
	back = DotProduct (end, plane->normal) - plane->dist;
	side = front < 0;
	
	if ( (back < 0) == side)
		return RecursiveLightPoint (node->children[side], start, end, outcolor);
	
	frac = front / (front-back);
	mid[0] = start[0] + (end[0] - start[0])*frac;
	mid[1] = start[1] + (end[1] - start[1])*frac;
	mid[2] = start[2] + (end[2] - start[2])*frac;
	
// go down front side	
	r = RecursiveLightPoint (node->children[side], start, mid, outcolor);
	if (r >= 0)
		return r;		// hit something
		
	if ( (back < 0) == side )
		return -1;		// didn't hit anuthing
		
// check for impact on this node
	model_t* world = gEngfuncs.GetEntityByIndex(0)->model; // buz

	surf = world->surfaces + node->firstsurface;
	for (i=0 ; i<node->numsurfaces ; i++, surf++)
	{
		if (surf->flags & SURF_DRAWTILED)
			continue;	// no lightmaps

		tex = surf->texinfo;
		
		s = DotProduct (mid, tex->vecs[0]) + tex->vecs[0][3];
		t = DotProduct (mid, tex->vecs[1]) + tex->vecs[1][3];;

		if (s < surf->texturemins[0] ||
		t < surf->texturemins[1])
			continue;
		
		ds = s - surf->texturemins[0];
		dt = t - surf->texturemins[1];
		
		if ( ds > surf->extents[0] || dt > surf->extents[1] )
			continue;

		if (!surf->samples)
			return 0;

		ds >>= 4;
		dt >>= 4;

		lightmap = surf->samples;
		r = 0;
		if (lightmap)
		{
			lightmap += dt * ((surf->extents[0]>>4)+1) + ds;
			
			// buz
			outcolor[0] = (float)lightmap->r / 255.0f;
			outcolor[1] = (float)lightmap->g / 255.0f;
			outcolor[2] = (float)lightmap->b / 255.0f;

			/*	for (maps = 0 ; maps < MAXLIGHTMAPS && surf->styles[maps] != 255 ;
					maps++)
			{
				scale = d_lightstylevalue[surf->styles[maps]];
				r += *lightmap * scale;
				lightmap += ((surf->extents[0]>>4)+1) *
						((surf->extents[1]>>4)+1);
			}			
			r >>= 8;*/
		}		
		return r;
	}

// go down back side
	return RecursiveLightPoint (node->children[!side], mid, end, outcolor);
}

int R_LightPoint (vec3_t &p, vec3_t &outcolor)
{
	vec3_t		end;
//	int			r;

	model_t* world = gEngfuncs.GetEntityByIndex(0)->model; // buz
	
	if (!world->lightdata)
	{
		outcolor[0] = outcolor[1] = outcolor[2] = 1; // buz
		return 255;
	}
	
	end[0] = p[0];
	end[1] = p[1];
	end[2] = p[2] - 2048;
	
	return RecursiveLightPoint (world->nodes, p, end, outcolor);
}






#ifndef M_PI
#define M_PI		3.14159265358979323846	// matches value in gcc v2 math.h
#endif
/*
void CrossProduct2 (const float *v1, const float *v2, float *cross)
{
	cross[0] = v1[1]*v2[2] - v1[2]*v2[1];
	cross[1] = v1[2]*v2[0] - v1[0]*v2[2];
	cross[2] = v1[0]*v2[1] - v1[1]*v2[0];
}*/

#define DEG2RAD( a ) ( a * M_PI ) / 180.0F

void R_ConcatRotations (float in1[3][3], float in2[3][3], float out[3][3])
{
	out[0][0] = in1[0][0] * in2[0][0] + in1[0][1] * in2[1][0] +
				in1[0][2] * in2[2][0];
	out[0][1] = in1[0][0] * in2[0][1] + in1[0][1] * in2[1][1] +
				in1[0][2] * in2[2][1];
	out[0][2] = in1[0][0] * in2[0][2] + in1[0][1] * in2[1][2] +
				in1[0][2] * in2[2][2];
	out[1][0] = in1[1][0] * in2[0][0] + in1[1][1] * in2[1][0] +
				in1[1][2] * in2[2][0];
	out[1][1] = in1[1][0] * in2[0][1] + in1[1][1] * in2[1][1] +
				in1[1][2] * in2[2][1];
	out[1][2] = in1[1][0] * in2[0][2] + in1[1][1] * in2[1][2] +
				in1[1][2] * in2[2][2];
	out[2][0] = in1[2][0] * in2[0][0] + in1[2][1] * in2[1][0] +
				in1[2][2] * in2[2][0];
	out[2][1] = in1[2][0] * in2[0][1] + in1[2][1] * in2[1][1] +
				in1[2][2] * in2[2][1];
	out[2][2] = in1[2][0] * in2[0][2] + in1[2][1] * in2[1][2] +
				in1[2][2] * in2[2][2];
}

void ProjectPointOnPlane( vec3_t &dst, const vec3_t &p, const vec3_t &normal )
{
	float d;
	vec3_t n;
	float inv_denom;

	inv_denom = 1.0F / DotProduct( normal, normal );

	d = DotProduct( normal, p ) * inv_denom;

	n[0] = normal[0] * inv_denom;
	n[1] = normal[1] * inv_denom;
	n[2] = normal[2] * inv_denom;

	dst[0] = p[0] - d * n[0];
	dst[1] = p[1] - d * n[1];
	dst[2] = p[2] - d * n[2];
}

/*
** assumes "src" is normalized
*/
void PerpendicularVector( vec3_t &dst, const vec3_t &src )
{
	int	pos;
	int i;
	float minelem = 1.0F;
	vec3_t tempvec;

	/*
	** find the smallest magnitude axially aligned vector
	*/
	for ( pos = 0, i = 0; i < 3; i++ )
	{
		if ( fabs( src[i] ) < minelem )
		{
			pos = i;
			minelem = fabs( src[i] );
		}
	}
	tempvec[0] = tempvec[1] = tempvec[2] = 0.0F;
	tempvec[pos] = 1.0F;

	/*
	** project the point onto the plane defined by src
	*/
	ProjectPointOnPlane( dst, tempvec, src );

	/*
	** normalize the result
	*/
	VectorNormalize( dst );
}

void RotatePointAroundVector( vec3_t &dst, const vec3_t &dir, const vec3_t &point, float degrees )
{
	float	m[3][3];
	float	im[3][3];
	float	zrot[3][3];
	float	tmpmat[3][3];
	float	rot[3][3];
	int	i;
	vec3_t vr, vup, vf;

	vf[0] = dir[0];
	vf[1] = dir[1];
	vf[2] = dir[2];

	PerpendicularVector( vr, dir );
	//CrossProduct( vr, vf, vup );
	vup = CrossProduct( vr, vf );

	m[0][0] = vr[0];
	m[1][0] = vr[1];
	m[2][0] = vr[2];

	m[0][1] = vup[0];
	m[1][1] = vup[1];
	m[2][1] = vup[2];

	m[0][2] = vf[0];
	m[1][2] = vf[1];
	m[2][2] = vf[2];

	memcpy( im, m, sizeof( im ) );

	im[0][1] = m[1][0];
	im[0][2] = m[2][0];
	im[1][0] = m[0][1];
	im[1][2] = m[2][1];
	im[2][0] = m[0][2];
	im[2][1] = m[1][2];

	memset( zrot, 0, sizeof( zrot ) );
	zrot[0][0] = zrot[1][1] = zrot[2][2] = 1.0F;

	zrot[0][0] = cos( DEG2RAD( degrees ) );
	zrot[0][1] = sin( DEG2RAD( degrees ) );
	zrot[1][0] = -sin( DEG2RAD( degrees ) );
	zrot[1][1] = cos( DEG2RAD( degrees ) );

	R_ConcatRotations( m, zrot, tmpmat );
	R_ConcatRotations( tmpmat, im, rot );

	for ( i = 0; i < 3; i++ )
	{
		dst[i] = rot[i][0] * point[0] + rot[i][1] * point[1] + rot[i][2] * point[2];
	}
}

int SignbitsForPlane (mplane_t *out)
{
	int	bits, j;
	// for fast box on planeside test
	bits = 0;
	for (j=0 ; j<3 ; j++)
	{
		if (out->normal[j] < 0)
			bits |= 1<<j;
	}
	return bits;
}

int BoxOnPlaneSide (vec3_t &emins, vec3_t &emaxs, mplane_t *p)
{
	float	dist1, dist2;
	int		sides;

// general case
	switch (p->signbits)
	{
	case 0:
dist1 = p->normal[0]*emaxs[0] + p->normal[1]*emaxs[1] + p->normal[2]*emaxs[2];
dist2 = p->normal[0]*emins[0] + p->normal[1]*emins[1] + p->normal[2]*emins[2];
		break;
	case 1:
dist1 = p->normal[0]*emins[0] + p->normal[1]*emaxs[1] + p->normal[2]*emaxs[2];
dist2 = p->normal[0]*emaxs[0] + p->normal[1]*emins[1] + p->normal[2]*emins[2];
		break;
	case 2:
dist1 = p->normal[0]*emaxs[0] + p->normal[1]*emins[1] + p->normal[2]*emaxs[2];
dist2 = p->normal[0]*emins[0] + p->normal[1]*emaxs[1] + p->normal[2]*emins[2];
		break;
	case 3:
dist1 = p->normal[0]*emins[0] + p->normal[1]*emins[1] + p->normal[2]*emaxs[2];
dist2 = p->normal[0]*emaxs[0] + p->normal[1]*emaxs[1] + p->normal[2]*emins[2];
		break;
	case 4:
dist1 = p->normal[0]*emaxs[0] + p->normal[1]*emaxs[1] + p->normal[2]*emins[2];
dist2 = p->normal[0]*emins[0] + p->normal[1]*emins[1] + p->normal[2]*emaxs[2];
		break;
	case 5:
dist1 = p->normal[0]*emins[0] + p->normal[1]*emaxs[1] + p->normal[2]*emins[2];
dist2 = p->normal[0]*emaxs[0] + p->normal[1]*emins[1] + p->normal[2]*emaxs[2];
		break;
	case 6:
dist1 = p->normal[0]*emaxs[0] + p->normal[1]*emins[1] + p->normal[2]*emins[2];
dist2 = p->normal[0]*emins[0] + p->normal[1]*emaxs[1] + p->normal[2]*emaxs[2];
		break;
	case 7:
dist1 = p->normal[0]*emins[0] + p->normal[1]*emins[1] + p->normal[2]*emins[2];
dist2 = p->normal[0]*emaxs[0] + p->normal[1]*emaxs[1] + p->normal[2]*emaxs[2];
		break;
	default:
		dist1 = dist2 = 0;		// shut up compiler
		gEngfuncs.Con_Printf("BoxOnPlaneSide error\n");
		break;
	}

	sides = 0;
	if (dist1 >= p->dist)
		sides = 1;
	if (dist2 < p->dist)
		sides |= 2;
	return sides;
}


// ========================================================


void FrustumCheck::R_SetFrustum (vec3_t vangles, vec3_t vorigin, float viewsize)
{
	int i;
	vec3_t vpn, vright, vup;

	AngleVectors (vangles, vpn, vright, vup);
	if (viewsize == 90) 
	{
		// front side is visible

		VectorAdd (vpn, vright, frustum[0].normal);
		VectorSubtract (vpn, vright, frustum[1].normal);

		VectorAdd (vpn, vup, frustum[2].normal);
		VectorSubtract (vpn, vup, frustum[3].normal);
	}
	else
	{
		// rotate VPN right by FOV_X/2 degrees
		RotatePointAroundVector( frustum[0].normal, vup, vpn, -(90-viewsize / 2 ) );
		// rotate VPN left by FOV_X/2 degrees
		RotatePointAroundVector( frustum[1].normal, vup, vpn, 90-viewsize / 2 );
		// rotate VPN up by FOV_X/2 degrees
		RotatePointAroundVector( frustum[2].normal, vright, vpn, 90-(viewsize*0.75) / 2 );
		// rotate VPN down by FOV_X/2 degrees
		RotatePointAroundVector( frustum[3].normal, vright, vpn, -( 90 -(viewsize*0.75) / 2 ) );
	}

	for (i=0 ; i<4 ; i++)
	{
		frustum[i].type = PLANE_ANYZ;
		frustum[i].dist = DotProduct (vorigin, frustum[i].normal);
		frustum[i].signbits = SignbitsForPlane (&frustum[i]);
	}

	farclipset = 0; // buz
}



void FrustumCheck::R_SetFrustum (vec3_t vangles, vec3_t vorigin, float view_hor, float view_ver, float dist)
{
	int i;
	vec3_t vpn, vright, vup;

	AngleVectors (vangles, vpn, vright, vup);

	// rotate VPN right by FOV_X/2 degrees
	RotatePointAroundVector( frustum[0].normal, vup, vpn, -(90-view_hor / 2 ) );
	// rotate VPN left by FOV_X/2 degrees
	RotatePointAroundVector( frustum[1].normal, vup, vpn, 90-view_hor / 2 );
	// rotate VPN up by FOV_X/2 degrees
	RotatePointAroundVector( frustum[2].normal, vright, vpn, 90-(view_ver) / 2 );
	// rotate VPN down by FOV_X/2 degrees
	RotatePointAroundVector( frustum[3].normal, vright, vpn, -( 90 -(view_ver) / 2 ) );

	for (i=0 ; i<4 ; i++)
	{
		frustum[i].type = PLANE_ANYZ;
		frustum[i].dist = DotProduct (vorigin, frustum[i].normal);
		frustum[i].signbits = SignbitsForPlane (&frustum[i]);
	}

	// buz: set also clipping by distance
	vec3_t farpoint = vorigin + (vpn * dist);
	farclip.normal = vpn * -1;
	farclip.type = PLANE_ANYZ;
	farclip.dist = DotProduct (farpoint, farclip.normal);
	farclip.signbits = SignbitsForPlane (&farclip);
	farclipset = 1;

	// save params for building stencil clipping volume
	VectorCopy(vorigin, m_origin);
	VectorCopy(vangles, m_angles);
	m_fov_hor = view_hor;
	m_fov_ver = view_ver;
	m_dist = dist;
}



qboolean FrustumCheck::R_CullBox (vec3_t mins, vec3_t maxs)
{
	int i;
	for (i=0 ; i<4 ; i++)
	{
		if (BoxOnPlaneSide (mins, maxs, &frustum[i]) == 2)
			return true;
	}

	// buz: also check clipping by distance
	if (farclipset)
	{
		if (BoxOnPlaneSide (mins, maxs, &farclip) == 2)
			return true;
	}

	return false;
}



extern cvar_t *cv_clipplanes;
extern cvar_t *cv_showplane;
extern vec3_t render_origin;

//debug -
int g_showplane = 0;
vec3_t g_planepoints[3];

void FrustumCheck::GL_FrustumSetClipPlanes()
{
	if (cv_clipplanes->value)
	{
		GLdouble equation[4];
		for (int i=0 ; i<4 ; i++)
		{
			equation[0] = frustum[i].normal[0];
			equation[1] = frustum[i].normal[1];
			equation[2] = frustum[i].normal[2];
			equation[3] = -frustum[i].dist;
			gl.glEnable(GL_CLIP_PLANE0 + i);
			gl.glClipPlane(GL_CLIP_PLANE0 + i, equation);
		}
	}
	else
	{
		int planepoints[8] = {0,3,  1,2,  2,3,  0,1};

		vec3_t points[4]; // up-left, up-right, down-right, down-left
		vec3_t forward, right, up;
		AngleVectors (m_angles, forward, right, up);
		float ofs_hor = tan((M_PI/360) * m_fov_hor) * m_dist;
		float ofs_ver = tan((M_PI/360) * m_fov_ver) * m_dist;

		vec3_t temp1, temp2;
		VectorMA(m_origin, m_dist, forward, temp1);

		VectorMA(temp1, ofs_ver, up, temp2);
		VectorMA(temp2, -ofs_hor, right, points[0]);
		VectorMA(temp2, ofs_hor, right, points[1]);

		VectorMA(temp1, -ofs_ver, up, temp2);
		VectorMA(temp2, ofs_hor, right, points[2]);
		VectorMA(temp2, -ofs_hor, right, points[3]);

		if (cv_showplane->value > 0 && cv_showplane->value <= 4) // debug
		{
			int planenum = cv_showplane->value - 1;
			float dist = DotProduct(render_origin, frustum[planenum].normal);
			if (dist > frustum[planenum].dist) g_showplane = 1;
			else g_showplane = 2;
			planenum*=2;
			VectorCopy(m_origin, g_planepoints[0]);
			VectorCopy(points[planepoints[planenum]], g_planepoints[1]);
			VectorCopy(points[planepoints[planenum+1]], g_planepoints[2]);
		}

		float dists[5];
		for (int i=0 ; i<4 ; i++)
			dists[i] = DotProduct(render_origin, frustum[i].normal) - frustum[i].dist;
		dists[4] = DotProduct(render_origin, farclip.normal) - farclip.dist;

		gl.glClear(GL_STENCIL_BUFFER_BIT);
		gl.glEnable(GL_STENCIL_TEST);
		gl.glStencilFunc(GL_ALWAYS, 0, ~0);

		gl.glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		gl.glDisable(GL_BLEND);
		gl.glDepthMask(GL_FALSE);
		gl.glDepthFunc(GL_LEQUAL);
		int cullstate = gl.glIsEnabled(GL_CULL_FACE);
		gl.glDisable(GL_CULL_FACE);

		SetTexEnvs(ENVSTATE_OFF);

		// use zfail method
		// draw inner sides first
		gl.glStencilOp(GL_KEEP, GL_INCR, GL_KEEP);
		gl.glBegin (GL_TRIANGLES);
		for (int i=0 ; i<4 ; i++)
		{
			if (dists[i] > 0) {
				gl.glVertex3fv(m_origin);
				gl.glVertex3fv(points[planepoints[i*2]]);
				gl.glVertex3fv(points[planepoints[i*2+1]]);
			}
		}
		gl.glEnd();
		if (dists[4] > 0) {
			gl.glBegin (GL_QUADS);
			gl.glVertex3fv(points[0]);
			gl.glVertex3fv(points[1]);
			gl.glVertex3fv(points[2]);
			gl.glVertex3fv(points[3]);
			gl.glEnd();
		}

		// draw outer sides second
		gl.glStencilOp(GL_KEEP, GL_DECR, GL_KEEP);
		gl.glBegin (GL_TRIANGLES);
		for (int i = 0; i<4; i++)
		{
			if (dists[i] <= 0) {
				gl.glVertex3fv(m_origin);
				gl.glVertex3fv(points[planepoints[i*2]]);
				gl.glVertex3fv(points[planepoints[i*2+1]]);
			}
		}
		gl.glEnd();
		if (dists[4] <= 0) {
			gl.glBegin (GL_QUADS);
			gl.glVertex3fv(points[0]);
			gl.glVertex3fv(points[1]);
			gl.glVertex3fv(points[2]);
			gl.glVertex3fv(points[3]);
			gl.glEnd();
		}

		// allow drawing light...
		gl.glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		gl.glStencilFunc(GL_NOTEQUAL, 0, ~0);
		gl.glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		gl.glDepthFunc(GL_EQUAL);

		if (cullstate) gl.glEnable(GL_CULL_FACE);
	}
}

void FrustumCheck::GL_DisableClipPlanes()
{
	if (cv_clipplanes->value)
	{
		for (int i=0 ; i<4 ; i++)
		{
			gl.glDisable(GL_CLIP_PLANE0 + i);
		}
	}
	else
	{
		gl.glDisable(GL_STENCIL_TEST);		
	}
}



void DrawDebugFrustumPlane()
{
	if (!gl.IsGLAllowed() || !g_showplane)
		return;

	if (g_showplane == 1) gl.glColor4ub(255, 0, 0, 255);
	else gl.glColor4ub(0, 0, 255, 255);

	int cullstate = gl.glIsEnabled(GL_CULL_FACE);
	gl.glDisable(GL_CULL_FACE);

	gl.glDisable(GL_TEXTURE_2D);
	gl.glBegin (GL_TRIANGLES);
		gl.glVertex3fv(g_planepoints[0]);
		gl.glVertex3fv(g_planepoints[1]);
		gl.glVertex3fv(g_planepoints[2]);
	gl.glEnd ();

	if (cullstate)
		gl.glEnable(GL_CULL_FACE);

	gl.glEnable(GL_TEXTURE_2D);

	g_showplane = 0;
}

mleaf_t *Mod_PointInLeaf (vec3_t p, model_t *model)
{
	mnode_t *node;
	float d;
	mplane_t *plane;

	if (!model)
		return NULL;
	if (!model->nodes)
		return NULL;

//	if (!model || !model->nodes)
//		Sys_Error ("Mod_PointInLeaf: bad model");
	node = model->nodes;
	while (1)
	{
		if (node->contents < 0)
			return (mleaf_t *)node;
		plane = node->plane;
		d = DotProduct (p,plane->normal) - plane->dist;
		if (d > 0)
			node = node->children[0];
		else
			node = node->children[1];
	}

	return NULL;	// never reached
}