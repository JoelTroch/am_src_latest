#ifndef __GLMANAGER_H
#define __GLMANAGER_H

#include <windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <gl/glext.h>
#include "../common/com_model.h"

//#include "log.h"

enum {
	GMSTATE_NOINIT = 0, // should try to load at next changelevel
	GMSTATE_INITFAILED, // failed to load, dont try until video mode changes
	GMSTATE_GL, // loaded ok, we are in opengl mode
};

bool BoxInPVS(vec3_t mins, vec3_t maxs);
byte* GetCurrentPVS();
mleaf_t *Mod_PointInLeaf(vec3_t p, model_t *model);

class GLManager
{
public:
	GLManager();
	~GLManager();
	void Init();
	void VidInit();	

	int  IsGLAllowed();
	bool IsExtensionSupported (const char *ext);
	vec3_t m_vViewAngles;
	vec3_t m_vRenderOrigin;
	int m_iVisFrame;
	bool glslSupport;

	void GetUpRight(vec3_t normal, vec3_t &up, vec3_t &right);

	// imported gl functions (initialization failed if not present)
	void (APIENTRY *glAccum)		(GLenum op, GLfloat value);
	void (APIENTRY *glAlphaFunc)	(GLenum func, GLclampf ref);
	void (APIENTRY *glBegin)		(GLenum mode);
	void (APIENTRY *glBindTexture)	(GLenum target, GLuint texture);
	void (APIENTRY *glBlendFunc)	(GLenum sfactor, GLenum dfactor);
	void (APIENTRY *glClipPlane)	(GLenum plane, const GLdouble *equation);
	void (APIENTRY *glClear)		(GLbitfield mask);
	void (APIENTRY *glClearColor)	(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
	void (APIENTRY *glColor3f)	(GLfloat red, GLfloat green, GLfloat blue);
	void (APIENTRY *glColor4f)	(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
	void (APIENTRY *glColor4ub) (GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
	void (APIENTRY *glColorMask)(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
	void (APIENTRY *glCopyTexImage2D) (GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
	void (APIENTRY *glCopyTexSubImage2D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
	void (APIENTRY *glCullFace)		(GLenum mode);
	void (APIENTRY *glDepthMask)	(GLboolean flag);
	void (APIENTRY *glDepthRange)	(GLclampd zNear, GLclampd zFar);
	void (APIENTRY *glDepthFunc)	(GLenum func);
	void (APIENTRY *glDisable)	(GLenum cap);
	void (APIENTRY *glDisableClientState)	(GLenum array);
	void (APIENTRY *glDrawArrays) (GLenum mode, GLint first, GLsizei count);
	void (APIENTRY *glDrawBuffer) (GLenum mode);
	void (APIENTRY *glDrawElements) (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices); // Wargon: Stencil shadows by BUzer.
	void (APIENTRY *glEnable)	(GLenum cap);
	void (APIENTRY *glEnableClientState) (GLenum array);
	void (APIENTRY *glEnd)		(void);
	void (APIENTRY *glFrustum)	(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
	const GLubyte* (APIENTRY *glGetString) (GLenum name);
	GLenum (APIENTRY *glGetError)	(void);
	void (APIENTRY *glGetDoublev) (GLenum pname, GLdouble *params);
	void (APIENTRY *glGetFloatv) (GLenum pname, GLfloat *params);
	void (APIENTRY *glGetIntegerv) (GLenum pname, GLint *params);
	void (APIENTRY *glGetTexEnviv) (GLenum target, GLenum pname, GLint *params);
	void (APIENTRY *glGetTexImage) (GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels);
	void (APIENTRY *glGetTexLevelParameteriv) (GLenum target, GLint level, GLenum pname, GLint *params);
	GLboolean (APIENTRY *glIsEnabled)	(GLenum cap);
	void (APIENTRY *glLoadIdentity)	(void);
	void (APIENTRY *glLoadMatrixf)	(const GLfloat *m);
	void (APIENTRY *glMatrixMode)	(GLenum mode);
	void (APIENTRY *glMultMatrixd)	(const GLdouble *m);
	void (APIENTRY *glOrtho)	(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
	void (APIENTRY *glPopAttrib)	(void);
	void (APIENTRY *glPushAttrib)	(GLbitfield mask);
	void (APIENTRY *glPixelStorei)	(GLenum pname, GLint param);
	void (APIENTRY *glPolygonOffset)	(GLfloat factor, GLfloat units);
	void (APIENTRY *glPushMatrix)	(void);
	void (APIENTRY *glPopMatrix)	(void);
	void (APIENTRY *glReadBuffer)	(GLenum mode);
	void (APIENTRY *glRotatef) (GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
	void (APIENTRY *glScalef)		(GLfloat x, GLfloat y, GLfloat z);
	void (APIENTRY *glShadeModel)	(GLenum mode);
	void (APIENTRY *glStencilFunc)	(GLenum func, GLint ref, GLuint mask);
	void (APIENTRY *glStencilOp)	(GLenum fail, GLenum zfail, GLenum zpass);
	void (APIENTRY *glStencilMask) (GLuint mask);
	void (APIENTRY *glTexCoord2f)	(GLfloat s, GLfloat t);
	void (APIENTRY *glTexCoordPointer) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
	void (APIENTRY *glTexEnvfv) (GLenum target, GLenum pname, const GLfloat *params);
	void (APIENTRY *glTexEnvf)	(GLenum target, GLenum pname, GLfloat param);
	void (APIENTRY *glTexEnvi) (GLenum target, GLenum pname, GLint param);
	void (APIENTRY *glTexGenfv)	(GLenum coord, GLenum pname, const GLfloat *params);
	void (APIENTRY *glTexGeni)	(GLenum coord, GLenum pname, GLint param);
	void (APIENTRY *glTexImage1D) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
	void (APIENTRY *glTexImage2D) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
	void (APIENTRY *glTexParameteri)	(GLenum target, GLenum pname, GLint param);
	void (APIENTRY *glTexParameterf) (GLenum target, GLenum pname, GLfloat param);
	void (APIENTRY *glTranslated)	(GLdouble x, GLdouble y, GLdouble z);
	void (APIENTRY *glTranslatef)	(GLfloat x, GLfloat y, GLfloat z);
	void (APIENTRY *glVertex3f) (GLfloat x, GLfloat y, GLfloat z);
	void (APIENTRY *glVertex3fv)	(const GLfloat *v);
	void (APIENTRY *glVertexPointer) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
	void (APIENTRY *glViewport)	(GLint x, GLint y, GLsizei width, GLsizei height);
	void (APIENTRY *glScissor)	(GLint x, GLint y, GLsizei width, GLsizei height);
	void (APIENTRY *glMultMatrixf)	(const GLfloat *m);
	void (APIENTRY *glNormalPointer)(GLenum type, GLsizei stride, const void *ptr);
	void (APIENTRY *glColorPointer)(GLint size, GLenum type, GLsizei stride, const void *ptr);
	PROC(APIENTRY *wglGetProcAddress)	(LPCSTR);

	// extension specific functions

	// ARB_multitexture
	int ARB_multitexture_supported;
	int MAX_TU_supported;
	PFNGLACTIVETEXTUREARBPROC		glActiveTextureARB;
	PFNGLCLIENTACTIVETEXTUREARBPROC	glClientActiveTextureARB;
	PFNGLMULTITEXCOORD1FARBPROC		glMultiTexCoord1fARB;
	PFNGLMULTITEXCOORD2FARBPROC		glMultiTexCoord2fARB;
	PFNGLMULTITEXCOORD3FVARBPROC	glMultiTexCoord3fvARB;
	PFNGLMULTITEXCOORD4FARBPROC		glMultiTexCoord4fARB;

	PFNGLUSEPROGRAMOBJECTARBPROC	 glUseProgramObjectARB;
	PFNGLCREATEPROGRAMOBJECTARBPROC	glCreateProgramObjectARB;
	PFNGLCREATESHADEROBJECTARBPROC glCreateShaderObjectARB;
	PFNGLSHADERSOURCEARBPROC glShaderSourceARB;
	PFNGLATTACHOBJECTARBPROC glAttachObjectARB;
	PFNGLCOMPILESHADERPROC glCompileShader;
	PFNGLLINKPROGRAMARBPROC glLinkProgramARB;
	PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
	PFNGLGETINFOLOGARBPROC glGetInfoLogARB;
	PFNGLGETOBJECTPARAMETERIVARBPROC glGetObjectParameterivARB;
	PFNGLGETATTRIBLOCATIONARBPROC glGetAttribLocationARB;
	PFNGLENABLEVERTEXATTRIBARRAYARBPROC glEnableVertexAttribArrayARB;
	PFNGLDISABLEVERTEXATTRIBARRAYARBPROC glDisableVertexAttribArrayARB; 
	PFNGLVERTEXATTRIBPOINTERARBPROC glVertexAttribPointerARB;
	PFNGLGETUNIFORMLOCATIONARBPROC glGetUniformLocationARB;
	PFNGLUNIFORM4FVARBPROC glUniform4fvARB;
	PFNGLUNIFORMMATRIX3X4FVPROC glUniformMatrix3x4fv;

	PFNGLUNIFORM1IARBPROC glUniform1iARB;
	PFNGLUNIFORM1FVARBPROC glUniform1fvARB;
	PFNGLUNIFORM1FARBPROC glUniform1fARB;
	PFNGLUNIFORM3FARBPROC glUniform3fARB;

	PFNGLCOMPRESSEDTEXIMAGE2DARBPROC glCompressedTexImage2DARB;
	// Diffuse bump-mapping extensions
	int ARB_dot3_supported;

	// NV_register_combiners
	int NV_combiners_supported;
	int MAX_NV_combiners;
	PFNGLCOMBINERPARAMETERINVPROC	glCombinerParameteriNV;
	PFNGLCOMBINERPARAMETERFVNVPROC	glCombinerParameterfvNV;
	PFNGLCOMBINERINPUTNVPROC		glCombinerInputNV;
	PFNGLCOMBINEROUTPUTNVPROC		glCombinerOutputNV;
	PFNGLFINALCOMBINERINPUTNVPROC	glFinalCombinerInputNV;

	// ARB_fragment_program
	int ARB_fragment_program_supported;
	int fp_max_image_units;
	int fp_max_texcoords;
	PFNGLGENPROGRAMSARBPROC			glGenProgramsARB;
	PFNGLBINDPROGRAMARBPROC			glBindProgramARB;
	PFNGLPROGRAMSTRINGARBPROC		glProgramStringARB;
	PFNGLGETPROGRAMIVARBPROC		glGetProgramivARB;

	// NV_fragment_program
	int NV_fragment_program_supported;
	int NV_fp_max_image_units;
	int NV_fp_max_texcoords;
	PFNGLGENPROGRAMSNVPROC			glGenProgramsNV;
	PFNGLBINDPROGRAMNVPROC			glBindProgramNV;
	PFNGLLOADPROGRAMNVPROC			glLoadProgramNV;

	// Cubemaps
	int ARB_texture_cube_map_supported;
	PFNGLTEXIMAGE3DEXTPROC			glTexImage3DEXT;

	// 3d textures
	int EXT_3Dtexture_supported;

	// VBO
	int ARB_VBO_supported;
	PFNGLBINDBUFFERARBPROC					glBindBufferARB;
	PFNGLGENBUFFERSARBPROC					glGenBuffersARB;
	PFNGLBUFFERDATAARBPROC					glBufferDataARB;
	PFNGLDELETEBUFFERSARBPROC				glDeleteBuffersARB;
	PFNGLGETBUFFERPARAMETERIVARBPROC		glGetBufferParameterivARB;

	// Non-power of two textures
	int texture_rectangle_supported;
	int texture_rectangle_max_size;

	void SaveStates();
	void RestoreStates();

	void bindToShader(int pointer, int number, GLuint texture);

	int activeUnit;
	int currentBindings[64];
	int enable1d[64];
	int enable2d[64];
	int enable3d[64];

//	void (APIENTRY *glColorTableEXT) (GLenum, GLenum, GLsizei, GLenum, GLenum, const GLvoid *);

	// Wargon: Stencil shadows by BUzer.
	void (APIENTRY *glLockArraysEXT) (GLint first, GLsizei count);
	void (APIENTRY *glUnlockArraysEXT) (void);

	// Paranoia hacks
	int max_texture_size;

	GLint alphabits;
	GLint stencilbits;

	// glu functions
	void gluLookAt( GLdouble eyex, GLdouble eyey, GLdouble eyez,
			GLdouble centerx, GLdouble centery, GLdouble centerz,
			GLdouble upx, GLdouble upy, GLdouble upz );

private:
	int LoadFunctions();
	int Load_ARB_multitexture();
	int Load_ARB_dot3();
	int Load_ARB_fragment_program();
	int Load_GLSL(); 
	int Load_NV_fragment_program();
	int Load_ARB_texture_cube_map();
	int Load_NV_register_combiners();
	int Load_EXT_texture_3D();
	int Load_ARB_VBO();
	int Load_Rectangle_Textures();
	void LogDebugInfo();

	HMODULE		hOpengl32dll;
//	cvar_t		*v_GLAllowed;
	int			glstate;
};


//==============================
//		PARTICLE ENGINE DEFS
//
//==============================
#define SYSTEM_SHAPE_POINT				0
#define SYSTEM_SHAPE_BOX				1
#define SYSTEM_SHAPE_PLANE_ABOVE_PLAYER 2
#define SYSTEM_SHAPE_PLANE				4
#define SYSTEM_SHAPE_AROUND_PLAYER		8


#define SYSTEM_DISPLAY_NORMAL			0
#define SYSTEM_DISPLAY_PARALELL			1
#define SYSTEM_DISPLAY_PLANAR			2
#define SYSTEM_DISPLAY_TRACER			3

#define SYSTEM_RENDERMODE_ADDITIVE		0
#define SYSTEM_RENDERMODE_ALPHABLEND	1
#define SYSTEM_RENDERMODE_INTENSITY		2

#define PARTICLE_COLLISION_NONE			0
#define PARTICLE_COLLISION_DIE			1
#define PARTICLE_COLLISION_BOUNCE		2
#define PARTICLE_COLLISION_DECAL		3
#define PARTICLE_COLLISION_STUCK		4
#define PARTICLE_COLLISION_NEW_SYSTEM	5

#define PARTICLE_WIND_NONE				0
#define PARTICLE_WIND_LINEAR			1
#define PARTICLE_WIND_SINE				2

#define PARTICLE_LIGHTCHECK_NONE		0
#define PARTICLE_LIGHTCHECK_NORMAL		1
#define PARTICLE_LIGHTCHECK_SCOLOR		2
#define PARTICLE_LIGHTCHECK_MIXP		3

typedef struct cl_texture_s
{
	unsigned int iIndex;
	unsigned int iWidth, iHeight;
	vec3_t color;
}cl_texture_t;
//========================================
//			PARTICLE ENGINE STRUCTS
//
//========================================
struct particle_system_t
{
	int id;
	int shapetype;
	int randomdir;
	bool bSmoke;
	int forceColorMode;
	vec3_t forceColor;
	vec3_t origin;
	vec3_t dir;
	int softparticles;
	int bloodparticles;

	int startingFrame, endingFrame;
	float minvel;
	float maxvel;
	float maxofs;

	float skyheight;

	float iPlaneXLength;
	float iPlaneYLength;

	float spawntime;
	float fadeintime;
	float fadeoutdelay;
	float velocitydamp;
	float stuckdie;
	float tracerdist;

	float maxheight;

	float windx;
	float windy;
	float windvar;
	float windmult;
	float windmultvar;
	int windtype;

	float maxlife;
	float maxlifevar;
	float systemsize;

	vec3_t primarycolor;
	vec3_t secondarycolor;
	float transitiondelay;
	float transitiontime;
	float transitionvar;

	float rotationvar;
	float rotationvel;
	float rotationdamp;
	float rotationdampdelay;

	float rotxvar;
	float rotxvel;
	float rotxdamp;
	float rotxdampdelay;

	float rotyvar;
	float rotyvel;
	float rotydamp;
	float rotydampdelay;

	float scale;
	float scalevar;
	float scaledampdelay;
	float scaledampfactor;
	float scalemax;
	float veldampdelay;
	float gravity;
	float particlefreq;
	float impactdamp;
	float mainalpha;

	int startparticles;
	int maxparticles;
	int	maxparticlevar;

	int overbright;
	int lightcheck;
	int collision;
	int colwater;
	int displaytype;
	int rendermode;
	int numspawns;

	int fadedistfar;
	int fadedistnear;

	int numframes;
	int framesizex;
	int framesizey;
	int framerate;

	char create[64];
	char deathcreate[64];
	char watercreate[64];

	particle_system_t *createsystem;
	particle_system_t *watersystem;
	particle_system_t *parentsystem;

	cl_texture_t *texture;
	mleaf_t *leaf;

	particle_system_t	*next;
	particle_system_t	*prev;

	struct cl_particle_t *particleheader;

	byte pad[14];

	float systemlife;
};

struct cl_particle_t
{
	vec3_t velocity;
	vec3_t origin;
	vec3_t color;
	vec3_t scolor;
	vec3_t lastspawn;
	vec3_t normal;

	float spawntime;
	float life;
	float scale;
	float alpha;

	float fadeoutdelay;

	float scaledampdelay;
	float scalemax;
	float secondarydelay;
	float secondarytime;

	float rotationvel;
	float rotation;

	float rotx;
	float rotxvel;

	float roty;
	float rotyvel;

	float windxvel;
	float windyvel;
	float windmult;

	float texcoords[4][2];

	int frame;

	particle_system_t *pSystem;

	cl_particle_t	*next;
	cl_particle_t	*prev;

	byte pad[4];

};

extern GLManager gl;
extern inline void		DotProductSSE(float* result, const float* v0, const float* v1);
extern inline void		SSEDotProductWorld(float* result, const float* v0, const float* v1);
extern inline void		SSEDotProductWorldInt(int* result, const float* v0, const float* v1);
extern inline void		SSEDotProductSub(float* result, vec3_t *v0, vec3_t *v1, float *subval);

extern inline void		VectorAddSSE(const float* v0, const float* v1, const float* result);
extern inline void		VectorMASSE(const float *veca, float scale, const float *vecb, float *vecc);
extern inline void		VectorTransformSSE(const float *in1, float in2[3][4], float *out);
extern inline void		VectorRotateSSE(const float *in1, float in2[3][4], float *out);


#endif // __GLMANAGER_H