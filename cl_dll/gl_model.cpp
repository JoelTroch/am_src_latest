/*
==============================================================================

SPRITE MODELS RENDERING

Uses Half-Life FX (R) technology. Copyright XaeroX, 2001-2006

==============================================================================
*/

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "const.h"
#include "entity_types.h"
#include "cdll_int.h"
#include "pm_defs.h"
#include "event_api.h"
#include "com_model.h"
#include "glmanager.h"
#include "studio_util.h"
#include "r_studioint.h"
#include "triangleapi.h"

extern vec3_t g_vecUp;
extern vec3_t g_vecRight;
extern vec3_t g_vecForward;
extern vec3_t render_origin;

// buz
void GL_SetSpriteRenderMode(int mode)
{
	if (mode == kRenderNormal)
	{
		gl.glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		gl.glDisable(GL_BLEND);
		gl.glDepthMask(GL_TRUE);
		return;
	}

	gl.glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	gl.glEnable(GL_BLEND);
	gl.glDepthMask(GL_FALSE);

	if (mode == kRenderTransAdd || mode == kRenderGlow)
	{
		gl.glBlendFunc(GL_ONE, GL_ONE);
		return;
	}

	gl.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


typedef enum { SPR_SINGLE=0, SPR_GROUP } spriteframetype_t;

#define SPR_VP_PARALLEL_UPRIGHT			0
#define SPR_FACING_UPRIGHT				1
#define SPR_VP_PARALLEL					2
#define SPR_ORIENTED					3
#define SPR_VP_PARALLEL_ORIENTED		4

#define SPR_NORMAL						0
#define SPR_ADDITIVE					1
#define SPR_INDEXALPHA					2
#define SPR_ALPHTEST					3

typedef struct mspriteframe_s
{
     int          width;
     int          height;
     float     up, down, left, right;
     int          gl_texturenum;
} mspriteframe_t;

typedef struct
{
     int                    numframes;
     float               *intervals;
     mspriteframe_t     *frames[1];
} mspritegroup_t;

typedef struct
{
     spriteframetype_t     type;
     mspriteframe_t          *frameptr;
} mspriteframedesc_t;

typedef struct
{
     short                    type;
     short                    texFormat;
     int                         maxwidth;
     int                         maxheight;
     int                         numframes;
     byte                    unknown_data[12];
     mspriteframedesc_t     frames[1];
} msprite_t;

//=======================================
// R_GetSpriteFrame
//=======================================

mspriteframe_t *R_GetSpriteFrame (model_t *mod, int frame)
{
     msprite_t          *psprite;
     mspritegroup_t     *pspritegroup;
     mspriteframe_t     *pspriteframe;
     int                    i, numframes;
     float               *pintervals, fullinterval, targettime;

     psprite = (msprite_t*)mod->cache.data;

     if ((frame >= psprite->numframes) || (frame < 0))
     {
          gEngfuncs.Con_Printf ("R_GetSpriteFrame: no such frame %d\n", frame);
          frame = 0;
     }

     if (psprite->frames[frame].type == SPR_SINGLE)
     {
          pspriteframe = psprite->frames[frame].frameptr;
     }
     else
     {
		 float realtime = gEngfuncs.GetClientTime();
		// gEngfuncs.Con_Printf("aAAAAAA\n");
          pspritegroup = (mspritegroup_t *)psprite->frames[frame].frameptr;
          pintervals = pspritegroup->intervals;
          numframes = pspritegroup->numframes;
          fullinterval = pintervals[numframes-1];

          // when loading in Mod_LoadSpriteGroup, we guaranteed all interval values
          // are positive, so we don't have to worry about division by 0
          targettime = realtime - ((int)(realtime / fullinterval)) * fullinterval;

          for (i=0 ; i<(numframes-1) ; i++)
          {
               if (pintervals[i] > targettime)
                    break;
          }

          pspriteframe = pspritegroup->frames[i];
     }

     return pspriteframe;
}


//=======================================
// R_DrawSprite
//=======================================

void R_DrawSprite ( cl_entity_t *e )
{
     mspriteframe_t     *frame;
     msprite_t          *psprite;
     vec3_t               point, forward, right, up;

     float scale = e->curstate.scale;
	 if (scale <= 0)
          scale = 1;

     frame = R_GetSpriteFrame (e->model, e->curstate.frame);
     psprite = (msprite_t*)e->model->cache.data;

     if (psprite->type == SPR_ORIENTED)
     {
          AngleVectors (e->angles, forward, right, up);
     }
     else if (psprite->type == SPR_FACING_UPRIGHT)
     {
          up[0] = up[1] = 0;
          up[2] = 1;
          right[0] = e->origin[1] - render_origin[1];
          right[1] = -(e->origin[0] - render_origin[0]);
          right[2] = 0;
          VectorNormalize (right);
          VectorCopy (g_vecForward, forward);
     }
     else if (psprite->type == SPR_VP_PARALLEL_UPRIGHT)
     {
          up[0] = up[1] = 0;
          up[2] = 1;
          VectorCopy (g_vecRight, right);
          VectorCopy (g_vecForward, forward);
     }
     else
     {     // normal sprite
          VectorCopy (g_vecUp, up);
          VectorCopy (g_vecRight, right);
          VectorCopy (g_vecForward, forward);
     }

	 // buz: set rendermode
	 int rmode = e->curstate.rendermode;
//	 if (psprite->type == SPR_ALPHTEST) rmode = kRenderTransAlpha;
	 GL_SetSpriteRenderMode(rmode);

     if (e->curstate.rendermode == kRenderGlow)
          gl.glDisable(GL_DEPTH_TEST);

	// gl.glEnable(GL_ALPHA_TEST);
	// gl.glAlphaFunc(GL_GREATER, 0.5);

	 // buz: black color means white
	 color24 *col = &e->curstate.rendercolor;
	 if (col->r || col->g || col->b)
		 gl.glColor4ub(col->r, col->g, col->b, e->curstate.renderamt);
	 else
		 gl.glColor4ub(255, 255, 255, e->curstate.renderamt);

	 gl.glBindTexture(GL_TEXTURE_2D, frame->gl_texturenum);
     gl.glBegin (GL_QUADS);

          point = e->origin - forward * 0.01 + up * frame->up * scale;
          point = point + right * frame->left * scale;

          gl.glTexCoord2f(0, 0);
          gl.glVertex3fv(point);

          point = e->origin - forward * 0.01 + up * frame->up * scale;
          point = point + right * frame->right * scale;

          gl.glTexCoord2f(1, 0);
          gl.glVertex3fv (point);

          point = e->origin - forward * 0.01 + up * frame->down * scale;
          point = point + right * frame->right * scale;

          gl.glTexCoord2f(1, 1);
          gl.glVertex3fv (point);

          point = e->origin - forward * 0.01 + up * frame->down * scale;
          point = point + right * frame->left * scale;

          gl.glTexCoord2f(0, 1);
          gl.glVertex3fv (point);

     gl.glEnd ();

	// gl.glDisable(GL_ALPHA_TEST);

     if (e->curstate.rendermode == kRenderGlow)
          gl.glEnable(GL_DEPTH_TEST);

}