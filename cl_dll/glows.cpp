//
// written by BUzer for HL: Paranoia modification
//
//		2006

#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "triangleapi.h"
#include "com_model.h"
#include "studio_util.h"
#include "glmanager.h"

#include "studio_event.h" // def. of mstudioevent_t
#include "r_efx.h"
#include "event_api.h"
#include "pm_defs.h"
#include "pmtrace.h"


#define MAX_GLOWS	16
#define GLOW_INTERP_SPEED	2

struct glowSprite
{
	cl_entity_t *ent;
	model_t *psprite;
	float	width, height;
};

glowSprite	glows[MAX_GLOWS];
int			numglows = 0;

extern vec3_t	render_origin;
extern vec3_t	render_angles;

cvar_t *v_glows;
cvar_t *v_glowstraces;
cvar_t *v_glowsdebug;

float lastDrawTime = 0;

// debug
void DrawVector(const vec3_t &start, const vec3_t &end, float r, float g, float b, float a)
{
	HLSPRITE hsprTexture = LoadSprite("sprites/white.spr");
	if (!hsprTexture){
		gEngfuncs.Con_Printf("NO SPRITE white.spr!\n");
		return;
	}
	const model_s *pTexture = gEngfuncs.GetSpritePointer( hsprTexture );
	gEngfuncs.pTriAPI->SpriteTexture( (struct model_s *)pTexture, 0 ); 
	gEngfuncs.pTriAPI->RenderMode( kRenderNormal );
	gEngfuncs.pTriAPI->Color4f( r, g, b, a );
	gEngfuncs.pTriAPI->Begin( TRI_LINES );
		gEngfuncs.pTriAPI->Vertex3f( start[0], start[1], start[2] );
		gEngfuncs.pTriAPI->Vertex3f( end[0], end[1], end[2] );
	gEngfuncs.pTriAPI->End();
}



int GlowFilterEntities ( int type, struct cl_entity_s *ent, const char *modelname )
{
	if (!gl.IsGLAllowed() || !v_glows->value)
		return 1;

	if (ent->curstate.rendermode == kRenderGlow &&
		ent->curstate.renderfx == kRenderFxNoDissipation)
	{
		if (numglows < MAX_GLOWS)
		{
			HLSPRITE hsprTexture = SPR_Load(modelname);
			if (!hsprTexture)
			{
				gEngfuncs.Con_Printf("GlowFilterEntities error: sprite %s not loaded!\n", modelname);
				return 1;
			}

			glows[numglows].psprite = (model_t *)gEngfuncs.GetSpritePointer( hsprTexture );
			glows[numglows].width = (float)SPR_Width(hsprTexture, 0) * ent->curstate.scale * 0.5;
			glows[numglows].height= (float)SPR_Height(hsprTexture, 0) * ent->curstate.scale * 0.5;
			glows[numglows].ent = ent;

			numglows++;
			return 0;
		}
	}
	return 1;
}

void SetPoint( float x, float y, float z, float (*matrix)[4])
{
	vec3_t point, result;
	point[0] = x;
	point[1] = y;
	point[2] = z;

	VectorTransform(point, matrix, result);

	gEngfuncs.pTriAPI->Vertex3f(result[0], result[1], result[2]);
}


// ent->latched.sequencetime keeps previous alpha for interpolation

void DrawGlows()
{
	float time = gEngfuncs.GetClientTime();
	float frametime = time - lastDrawTime;
	lastDrawTime = time;

	if (!numglows)
		return;

	int depthtestenabled = 0;
	if (gl.glIsEnabled(GL_DEPTH_TEST)) depthtestenabled = 1;
	gl.glDisable(GL_DEPTH_TEST);

	float matrix[3][4];
	AngleMatrix (render_angles, matrix);	// calc view matrix

	for (int i = 0; i < numglows; i++)
	{
		matrix[0][3] = glows[i].ent->curstate.origin[0]; // write origin to matrix
		matrix[1][3] = glows[i].ent->curstate.origin[1];
		matrix[2][3] = glows[i].ent->curstate.origin[2];

		vec3_t dir;
		AngleVectors(render_angles, dir, NULL, NULL);
		float dst = DotProduct(dir, glows[i].ent->curstate.origin - render_origin);
		if (dst > 64) dst = 64;
		if (dst < 0) dst = 0;
		dst = dst / 64;
		
		int numtraces = v_glowstraces->value;
		if (numtraces < 1) numtraces = 1;
		
		vec3_t aleft, aright, left, right, dist, step;
		left[0] = 0; left[1] = -glows[i].width / 5; left[2] = 0;
		right[0] = 0; right[1] = glows[i].width / 5; right[2] = 0;
		VectorTransform(left, matrix, aleft);
		VectorTransform(right, matrix, aright);
		VectorSubtract(aright, aleft, dist);
		VectorScale(dist, 1.0 / (numtraces+1), step);
		float frac = (float)1 / numtraces;
		float totalfrac = 0;

		gEngfuncs.pEventAPI->EV_SetTraceHull( 2 );
		for (int j = 0; j < numtraces; j++)
		{
			vec3_t start;
			VectorMA(aleft, j+1, step, start);

			pmtrace_t pmtrace;
			gEngfuncs.pEventAPI->EV_PlayerTrace( render_origin, start, PM_GLASS_IGNORE|PM_STUDIO_IGNORE, -1, &pmtrace );

			if ( pmtrace.fraction == 1 )
			{
				if (v_glowsdebug->value)
					DrawVector(start, render_origin + Vector(0, 0, -8), 0, 1, 0, 1);
				totalfrac += frac;
			}
			else
			{
				if (v_glowsdebug->value)
					DrawVector(start, render_origin + Vector(0, 0, -8), 1, 0, 0, 1);
			}
		}

		float targetalpha = totalfrac;
		if (glows[i].ent->latched.sequencetime > targetalpha)
		{
			glows[i].ent->latched.sequencetime -= frametime * GLOW_INTERP_SPEED;
			if (glows[i].ent->latched.sequencetime <= targetalpha)
				glows[i].ent->latched.sequencetime = targetalpha;
		}
		else if (glows[i].ent->latched.sequencetime < targetalpha)
		{
			glows[i].ent->latched.sequencetime += frametime * GLOW_INTERP_SPEED;
			if (glows[i].ent->latched.sequencetime >= targetalpha)
				glows[i].ent->latched.sequencetime = targetalpha;
		}

	//	gEngfuncs.Con_Printf("target: %f, current: %f\n", targetalpha, glows[i].ent->latched.sequencetime);

		gEngfuncs.pTriAPI->RenderMode(kRenderTransAdd);
		gEngfuncs.pTriAPI->CullFace( TRI_NONE ); //no culling
		gEngfuncs.pTriAPI->SpriteTexture( glows[i].psprite, 0);		
		gEngfuncs.pTriAPI->Color4f( (float)glows[i].ent->curstate.rendercolor.r / 255,
			(float)glows[i].ent->curstate.rendercolor.g / 255,
			(float)glows[i].ent->curstate.rendercolor.b / 255,
			(float)glows[i].ent->curstate.renderamt / 255 );
		gEngfuncs.pTriAPI->Brightness(((float)glows[i].ent->curstate.renderamt / 255) * glows[i].ent->latched.sequencetime * dst);

		gEngfuncs.pTriAPI->Begin( TRI_QUADS );

			gEngfuncs.pTriAPI->TexCoord2f( 0, 0 );
			SetPoint(0, glows[i].width ,glows[i].height, matrix);

			gEngfuncs.pTriAPI->TexCoord2f( 0, 1 );
			SetPoint(0, glows[i].width ,-glows[i].height, matrix);

			gEngfuncs.pTriAPI->TexCoord2f( 1, 1 );
			SetPoint(0, -glows[i].width ,-glows[i].height, matrix);

			gEngfuncs.pTriAPI->TexCoord2f( 1, 0 );
			SetPoint(0, -glows[i].width ,glows[i].height, matrix);
			
		gEngfuncs.pTriAPI->End();

	//	gEngfuncs.Con_Printf("draw glow at %f, %f, %f\n", glows[i].pos[0], glows[i].pos[1], glows[i].pos[2]);
	//	gEngfuncs.Con_Printf("with color %f, %f, %f, %f\n", glows[i].r, glows[i].g, glows[i].b, glows[i].a);
	//	gEngfuncs.Con_Printf("with width %f and height %f\n", glows[i].width, glows[i].height);
		
	}

	gEngfuncs.pTriAPI->RenderMode(kRenderNormal);
	numglows = 0;

	if (depthtestenabled)
		gl.glEnable(GL_DEPTH_TEST);
}

		
void InitGlows()
{
	v_glows = gEngfuncs.pfnRegisterVariable( "gl_glows", "1", 0 );
	v_glowstraces = gEngfuncs.pfnRegisterVariable( "gl_glowstraces", "5", 0 );
	v_glowsdebug = gEngfuncs.pfnRegisterVariable( "gl_glowsdebug", "0", 0 );
}


void ResetGlows()
{
	lastDrawTime = 0;
}
	