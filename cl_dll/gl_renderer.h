
#include "glmanager.h"
#include "ref_params.h"
#include "com_model.h"
#include "gl_dlight.h"

// ============== common render definitions ==============

#define	SURF_PLANEBACK		2
#define	SURF_DRAWSKY		4
#define SURF_DRAWSPRITE		8
#define SURF_DRAWTURB		0x10
#define SURF_DRAWTILED		0x20
#define SURF_DRAWBACKGROUND	0x40
#define SURF_UNDERWATER		0x80
#define SURF_DONTWARP		0x100
#define BACKFACE_EPSILON	0.01

// 0-2 are axial planes
#define	PLANE_X			0
#define	PLANE_Y			1
#define	PLANE_Z			2

#define SURF_BUMPREADY		(1<<9)
#define SURF_SPECULAR		(1<<10)

#include "gl_frustum.h"

extern cvar_t *sv_skycolor_r;
extern cvar_t *sv_skycolor_g;
extern cvar_t *sv_skycolor_b;

mleaf_t		*Mod_PointInLeaf (vec3_t p, model_t *model);

//
// caches
//
enum {
	TC_OFF,
	TC_TEXTURE,
	TC_LIGHTMAP,
	TC_VERTEX_POSITION, // for specular and dynamic lighting
	TC_NOSTATE // uninitialized
};

void SetTexPointer(int unitnum, int tc);
void R_DrawBrushModel(cl_entity_t *e, int onlyfirstpass);
int IsPitchReversed(float pitch);
int IsEntityMoved(cl_entity_t *e);

extern cl_entity_t *currententity;

enum {
	ENVSTATE_OFF,
	ENVSTATE_REPLACE,
	ENVSTATE_MUL_CONST,
	ENVSTATE_MUL_PREV_CONST, // ignores texture
	ENVSTATE_MUL,
	ENVSTATE_MUL_X2,
	ENVSTATE_ADD,
	ENVSTATE_DOT,
	ENVSTATE_DOT_CONST,
	ENVSTATE_PREVCOLOR_CURALPHA,
	ENVSTATE_NOSTATE // uninitialized
};

void SetTexEnvs(int env0, int env1 = ENVSTATE_OFF, int env2 = ENVSTATE_OFF, int env3 = ENVSTATE_OFF);
void Bind2DTexture(GLenum texture, GLuint id);
void ResetCache();

extern cvar_t *gl_shadowsize;
// polygon rendering
void SetupTexMatrix (msurface_t *s, const vec3_t &vec);
void DrawPolyFromArray (msurface_t *s);
void DrawPolyFromArray (glpoly_t *p);


extern vec3_t vec_to_eyes;

extern vec3_t sky_origin;
extern vec3_t sky_world_origin;
extern float  sky_speed;

extern int skylight_amb;
extern int skylight_shade;

extern cvar_t *cv_renderer;
extern cvar_t *cv_bump;
extern cvar_t *cv_highspecular;
extern cvar_t *cv_detailtex;
extern cvar_t *cv_bumpdebug;
extern cvar_t *cv_specular;
extern cvar_t *cv_specular_nocombiners;
extern cvar_t *cv_specular_noshaders;
extern cvar_t *cv_gamma;
extern cvar_t *cv_brightness;
extern cvar_t *cv_contrast;
extern cvar_t *cv_blurtest;
extern cvar_t *cv_nsr; // new studio renderer :)
extern cvar_t *cv_lambert;

// BUMP lightmaps base indexes
extern int g_lightmap_baselight;
extern int g_lightmap_addlight;
extern int g_lightmap_lightvecs;


enum {
	BUMP_BASELIGHT_STYLE	= 61,
	BUMP_ADDLIGHT_STYLE		= 62,
	BUMP_LIGHTVECS_STYLE	= 63,
};


extern cl_entity_t *renderents[];
extern int numrenderents;
extern int framecount;

float ApplyGamma( float value );




// =========== link to world =============
void RendererCalcRefDef( ref_params_t *pparams );
void RendererDrawNormal();
void RendererDrawTransparent();
void RendererCreateEntities();
void RendererTentsUpdate();
int  RendererFilterEntities( int type, struct cl_entity_s *ent, const char *modelname );
void RendererVidInit();
void RendererInit();
void RendererCleanup();
void RendererDrawHud();
void RendererUpdateEntityList();

#define ONCE( somecode )	\
do {						\
	static int called = 0;	\
	if (!called)			\
	{						\
		somecode			\
		called = 1;			\
	}						\
} while (0)