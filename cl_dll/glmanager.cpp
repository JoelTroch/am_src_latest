//
// written by BUzer for HL: Paranoia modification
//
//		2005 - 2006

#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "com_model.h"

#include "studio_util.h"
#include "r_studioint.h"

#include "glmanager.h"
#include "gl_shaders.h"

extern engine_studio_api_t IEngineStudio;
GLManager gl;

void GLManager::bindToShader(int pointer, int number, GLuint texture){
	gl.glActiveTextureARB(GL_TEXTURE0_ARB + number);
	gl.glEnable(GL_TEXTURE_2D);
	gl.glUniform1iARB(pointer, number);
	gl.glBindTexture(GL_TEXTURE_2D, texture);
}

void GLManager::SaveStates()
{
	gl.glGetIntegerv(GL_ACTIVE_TEXTURE, &activeUnit);

	for (int i = 0; i<gl.MAX_TU_supported; i++)
	{
		gl.glActiveTextureARB(GL_TEXTURE0_ARB + i);
		gl.glGetIntegerv(GL_TEXTURE_BINDING_2D, &currentBindings[i]);

		gl.glGetIntegerv(GL_TEXTURE_1D, &enable1d[i]);
		gl.glGetIntegerv(GL_TEXTURE_2D, &enable2d[i]);
		gl.glGetIntegerv(GL_TEXTURE_3D, &enable3d[i]);
	}

	gl.glActiveTextureARB(activeUnit);
}

void GLManager::RestoreStates()
{
	for (int i = 0; i<gl.MAX_TU_supported; i++)
	{
		gl.glActiveTextureARB(GL_TEXTURE0_ARB + i);
		gl.glBindTexture(GL_TEXTURE_2D, currentBindings[i]);

		if (enable1d[i])
			gl.glEnable(GL_TEXTURE_1D);
		else
			gl.glDisable(GL_TEXTURE_1D);

		if (enable2d[i])
			gl.glEnable(GL_TEXTURE_2D);
		else
			gl.glDisable(GL_TEXTURE_2D);

		if (enable3d[i])
			gl.glEnable(GL_TEXTURE_3D);
		else
			gl.glDisable(GL_TEXTURE_3D);
	}

	gl.glActiveTextureARB(activeUnit);
}

void GLManager::GetUpRight(vec3_t normal, vec3_t &up, vec3_t &right)
{
	vec3_t temp;
	temp[0] = 0; temp[1] = 0; temp[2] = 1;
	if (normal[0] != 0 || normal[1] != 0)
	{
		CrossProduct(normal, temp, right);
		CrossProduct(right, normal, up);
	}
	else
	{
		right[0] = 0; right[1] = 1; right[2] = 0;
		up[0] = 1; up[1] = 0; up[2] = 0;
	}
}

bool GLManager::IsExtensionSupported(const char *ext)
{
	const char * extensions = (const char *)glGetString ( GL_EXTENSIONS );
	const char * start = extensions;
	const char * ptr;

	while ( ( ptr = strstr ( start, ext ) ) != NULL )
	{
		// we've found, ensure name is exactly ext
		const char * end = ptr + strlen ( ext );
		if ( isspace ( *end ) || *end == '\0' )
			return true;

		start = end;
	}
	return false;
}

// ===================== Basic initialization ======================

int GLManager::IsGLAllowed()
{
	if (glstate == GMSTATE_GL)
		return TRUE;
	else
		return FALSE;
}


GLManager::GLManager()
{
	glstate = GMSTATE_NOINIT;
	hOpengl32dll = 0;
}


void GLManager::Init()
{
//	v_GLAllowed = gEngfuncs.pfnRegisterVariable( "gl_clientfx", "1", 0 );
}


void GLManager::VidInit()
{
	gEngfuncs.Con_Printf("GL manager: reset\n");
	if (glstate != GMSTATE_NOINIT)
	{
		gEngfuncs.Con_Printf("GL manager: already loaded\n");
		return;
	}

	// try to load library
	//	if (!v_GLAllowed->value)
	if (gEngfuncs.CheckParm("-noglfx", NULL))
	{
		gEngfuncs.Con_Printf("GL manager: disabled by user\n");
		gEngfuncs.Con_Printf("VidInit: skipping loading of opengl32.dll - disabled by user\n");
		return;
	}

	if (!IEngineStudio.IsHardware())
	{
		gEngfuncs.Con_Printf("GL manager: Software renderer\n");
		gEngfuncs.Con_Printf("VidInit: Software renderer\n");
		glstate = GMSTATE_INITFAILED;
		return;
	}

	hOpengl32dll = LoadLibrary("opengl32.dll");
	/*
	if (hOpengl32dll)
	{
	// check hacked opengl library
	if (IsExtensionSupported("PARANOIA_HACKS_V1") || IsExtensionSupported("RAVEN_HACKS_V1"))
	{
	MessageBox(NULL, "VIDEO ERROR: Paranoia's hacked OpenGL32.dll is found!\n", "ERROR", MB_OK);

	gEngfuncs.Con_Printf("VidInit: Error! opengl32.dll is present!\n");

	//FreeLibrary( hOpengl32dll );
	//hOpengl32dll = 0;
	glstate = GMSTATE_INITFAILED;

	gEngfuncs.pfnClientCmd("quit\n");

	return;
	}
	}*/

	if (!hOpengl32dll)
	{
		gEngfuncs.Con_Printf("GL manager: Cannot load opengl32.dll\n");
		gEngfuncs.Con_Printf("VidInit ERROR: Cannot load opengl32.dll!\n");
		glstate = GMSTATE_INITFAILED;
		return;
	}

	if (!LoadFunctions())
	{
		gEngfuncs.Con_Printf("GL manager: Error loading opengl32.dll functions\n");
		gEngfuncs.Con_Printf("VidInit: Error loading opengl32.dll functions!\n");
		FreeLibrary(hOpengl32dll);
		hOpengl32dll = 0;
		glstate = GMSTATE_INITFAILED;
		return;
	}

	gEngfuncs.Con_Printf("VidInit: opengl32.dll loaded successfully\n");
	gEngfuncs.Con_Printf("GL manager: loaded library\n");

	const GLubyte *str = glGetString(GL_RENDERER);
	if (str)
	{
		gEngfuncs.Con_Printf("GL manager: opengl renderer\n");
		gEngfuncs.Con_Printf("VidInit: OpenGL renderer (%s)\n", str);
		glstate = GMSTATE_GL;

		LogDebugInfo();

		// load extensions here
		Load_ARB_multitexture();
		Load_ARB_dot3();
		Load_ARB_fragment_program();
		Load_ARB_texture_cube_map();
		//	Load_NV_fragment_program();
		Load_NV_register_combiners();
		Load_EXT_texture_3D();
		Load_ARB_VBO();
		Load_Rectangle_Textures();
		Load_GLSL();

		gl.glGetIntegerv(GL_ALPHA_BITS, &alphabits);
		gl.glGetIntegerv(GL_ALPHA_BITS, &stencilbits);
		gl.glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);
		return;
	}
	else
	{
		gEngfuncs.Con_Printf("GL manager: Direct 3D renderer\n");
		gEngfuncs.Con_Printf("VidInit: Direct3D renderer\n");
		FreeLibrary(hOpengl32dll);
		hOpengl32dll = 0;
		glstate = GMSTATE_INITFAILED;
	}
}


GLManager::~GLManager()
{
	if (hOpengl32dll)
		FreeLibrary( hOpengl32dll );

	gEngfuncs.Con_DPrintf("GL manager: shutdown\n");
	hOpengl32dll = 0;
}


void GLManager::LogDebugInfo()
{
	gEngfuncs.Con_DPrintf("GL_RENDERER: %s\n", glGetString(GL_RENDERER));
	gEngfuncs.Con_DPrintf("GL_VENDOR: %s\n", glGetString(GL_VENDOR));
	gEngfuncs.Con_DPrintf("GL_VERSION: %s\n", glGetString(GL_VERSION));

	GLint val_r, val_g, val_b, val_a;
	glGetIntegerv(GL_RED_BITS, &val_r);
	glGetIntegerv(GL_GREEN_BITS, &val_g);
	glGetIntegerv(GL_BLUE_BITS, &val_b);
	glGetIntegerv(GL_ALPHA_BITS, &val_a);
	gEngfuncs.Con_DPrintf("Color buffer RGBA bits: %d, %d, %d, %d\n", val_r, val_g, val_b, val_a);

	glGetIntegerv(GL_ACCUM_RED_BITS, &val_r);
	glGetIntegerv(GL_ACCUM_GREEN_BITS, &val_g);
	glGetIntegerv(GL_ACCUM_BLUE_BITS, &val_b);
	glGetIntegerv(GL_ACCUM_ALPHA_BITS, &val_a);
	gEngfuncs.Con_DPrintf("Accum buffer RGBA bits: %d, %d, %d, %d\n", val_r, val_g, val_b, val_a);

	glGetIntegerv(GL_DEPTH_BITS, &val_r);
	gEngfuncs.Con_DPrintf("Depth buffer bits: %d\n", val_r);

	glGetIntegerv(GL_STENCIL_BITS, &val_r);
	gEngfuncs.Con_DPrintf("Stencil buffer bits: %d\n", val_r);
}

// ===================== functions loading ======================

template <typename FuncType>
inline void GLMLoadProc( FuncType &pfn, const char* name, HMODULE hlib )
{
	pfn = (FuncType)GetProcAddress( hlib, name );
}

#define LOAD_PROC(x) GLMLoadProc( x, #x, hOpengl32dll ); \
	if (!x) {iret = FALSE; \
		gEngfuncs.Con_Printf ("Error loading opengl function "); \
		gEngfuncs.Con_Printf (#x); \
		gEngfuncs.Con_Printf ("\n");}


template <typename FuncType>
inline void GLMLoadProc_EXT( FuncType &pfn, const char* name )
{
	pfn = (FuncType)gl.wglGetProcAddress( name );
}

#define LOAD_PROC_EXT(x) GLMLoadProc_EXT( x, #x ); \
	if (!x) {iret = FALSE; \
		gEngfuncs.Con_Printf ("Error loading extension opengl function "); \
		gEngfuncs.Con_Printf (#x); \
		gEngfuncs.Con_Printf ("\n");}


int GLManager::LoadFunctions()
{
	int iret = TRUE;
	
	LOAD_PROC(glAccum);
	LOAD_PROC(glAlphaFunc);
	LOAD_PROC(glBegin);
	LOAD_PROC(glBindTexture);
	LOAD_PROC(glBlendFunc);
	LOAD_PROC(glClear);
	LOAD_PROC(glClearColor);
	LOAD_PROC(glClipPlane);
	LOAD_PROC(glColor3f);
	LOAD_PROC(glColor4f);
	LOAD_PROC(glColor4ub);
	LOAD_PROC(glColorMask);
	LOAD_PROC(glCopyTexImage2D);
	LOAD_PROC(glCopyTexSubImage2D);
	LOAD_PROC(glCullFace);
	LOAD_PROC(glDepthMask);
	LOAD_PROC(glDepthRange);
	LOAD_PROC(glDepthFunc);
	LOAD_PROC(glDisable);
	LOAD_PROC(glDisableClientState);
	LOAD_PROC(glDrawArrays);
	LOAD_PROC(glDrawBuffer);
	LOAD_PROC(glDrawElements); // Wargon: Stencil shadows by BUzer.
	LOAD_PROC(glEnable);
	LOAD_PROC(glEnableClientState);
	LOAD_PROC(glEnd);
	LOAD_PROC(glFrustum);
	LOAD_PROC(glGetDoublev);
	LOAD_PROC(glGetFloatv);
	LOAD_PROC(glGetIntegerv);
	LOAD_PROC(glGetTexEnviv);
	LOAD_PROC(glGetTexImage);
	LOAD_PROC(glGetTexLevelParameteriv);
	LOAD_PROC(glGetString);
	LOAD_PROC(glGetError);
	LOAD_PROC(glIsEnabled);
	LOAD_PROC(glLoadIdentity);
	LOAD_PROC(glLoadMatrixf);
	LOAD_PROC(glMatrixMode);
	LOAD_PROC(glMultMatrixd);
	LOAD_PROC(glOrtho);
	LOAD_PROC(glPopAttrib);
	LOAD_PROC(glPushAttrib);
	LOAD_PROC(glPixelStorei);
	LOAD_PROC(glPolygonOffset);
	LOAD_PROC(glPushMatrix);
	LOAD_PROC(glPopMatrix);
	LOAD_PROC(glReadBuffer);
	LOAD_PROC(glRotatef);
	LOAD_PROC(glScalef);
	LOAD_PROC(glShadeModel);
	LOAD_PROC(glStencilFunc);
	LOAD_PROC(glStencilOp);
	LOAD_PROC(glStencilMask);
	LOAD_PROC(glTexCoord2f);
	LOAD_PROC(glTexCoordPointer);
	LOAD_PROC(glTexEnvfv);
	LOAD_PROC(glTexEnvf);
	LOAD_PROC(glTexEnvi);
	LOAD_PROC(glTexGenfv);
	LOAD_PROC(glTexGeni);
	LOAD_PROC(glTexImage1D);
	LOAD_PROC(glTexImage2D);
	LOAD_PROC(glTexParameteri);
	LOAD_PROC(glTexParameterf);
	LOAD_PROC(glTranslated);
	LOAD_PROC(glTranslatef);
	LOAD_PROC(glVertex3f);
	LOAD_PROC(glVertex3fv);
	LOAD_PROC(glVertexPointer);
	LOAD_PROC(glViewport);
	LOAD_PROC(glScissor);
	LOAD_PROC(glMultMatrixf);
	LOAD_PROC(glNormalPointer);
	LOAD_PROC(glColorPointer);
	LOAD_PROC(wglGetProcAddress);

	// Wargon: Stencil shadows by BUzer.
	LOAD_PROC_EXT(glLockArraysEXT);
	LOAD_PROC_EXT(glUnlockArraysEXT);
	LOAD_PROC_EXT(glCompressedTexImage2DARB);

	return iret;
}

int GLManager::Load_ARB_multitexture()
{
	ARB_multitexture_supported = FALSE;
	MAX_TU_supported = 1;
	if (IsExtensionSupported("GL_ARB_multitexture"))
	{
		int iret = TRUE;

		glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &MAX_TU_supported);
	//	MAX_TU_supported = 2;// test
		LOAD_PROC_EXT(glActiveTextureARB);
		LOAD_PROC_EXT(glClientActiveTextureARB);
		LOAD_PROC_EXT(glMultiTexCoord1fARB);
		LOAD_PROC_EXT(glMultiTexCoord2fARB);
		LOAD_PROC_EXT(glMultiTexCoord3fvARB);
		LOAD_PROC_EXT(glMultiTexCoord4fARB);

		ARB_multitexture_supported = iret;
	}
	return ARB_multitexture_supported;
}

int GLManager::Load_NV_register_combiners()
{
	NV_combiners_supported = FALSE;
	MAX_NV_combiners = 0;
	if (IsExtensionSupported("GL_NV_register_combiners"))
	{
		int iret = TRUE;

		glGetIntegerv(GL_MAX_GENERAL_COMBINERS_NV, &MAX_NV_combiners);
		LOAD_PROC_EXT(glCombinerParameteriNV);
		LOAD_PROC_EXT(glCombinerParameterfvNV);
		LOAD_PROC_EXT(glCombinerInputNV);
		LOAD_PROC_EXT(glCombinerOutputNV);
		LOAD_PROC_EXT(glFinalCombinerInputNV);

		NV_combiners_supported = iret;
	}
	return NV_combiners_supported;
}

int GLManager::Load_ARB_dot3()
{
	if (IsExtensionSupported("ARB_texture_env_combine") &&
		IsExtensionSupported("ARB_texture_env_dot3"))
	{
		ARB_dot3_supported = TRUE;
	}
	else
		ARB_dot3_supported = FALSE;

	return ARB_dot3_supported;
}

int GLManager::Load_ARB_fragment_program()
{
	ARB_fragment_program_supported = FALSE;
	fp_max_image_units = 0;
	fp_max_texcoords = 0;
	if (IsExtensionSupported("GL_ARB_fragment_program"))
	{
		int iret = TRUE;

		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS_ARB, &fp_max_image_units);
		glGetIntegerv(GL_MAX_TEXTURE_COORDS_ARB, &fp_max_texcoords);
		LOAD_PROC_EXT(glGenProgramsARB);
		LOAD_PROC_EXT(glBindProgramARB);
		LOAD_PROC_EXT(glProgramStringARB);
		LOAD_PROC_EXT(glGetProgramivARB);

		ARB_fragment_program_supported = iret;
	}
	return ARB_fragment_program_supported;
}

int GLManager::Load_GLSL()
{
	int iret = TRUE;
	glslSupport = true;

	LOAD_PROC_EXT(glCreateProgramObjectARB);
	LOAD_PROC_EXT(glCreateShaderObjectARB);
	LOAD_PROC_EXT(glLinkProgramARB);
	LOAD_PROC_EXT(glAttachObjectARB);
	LOAD_PROC_EXT(glShaderSourceARB);
	LOAD_PROC_EXT(glCompileShader);
	LOAD_PROC_EXT(glUseProgramObjectARB);
	LOAD_PROC_EXT(glGetInfoLogARB);
	LOAD_PROC_EXT(glGetObjectParameterivARB);
	LOAD_PROC_EXT(glGetAttribLocationARB);
	LOAD_PROC_EXT(glVertexAttribPointerARB);
	LOAD_PROC_EXT(glDisableVertexAttribArrayARB);
	LOAD_PROC_EXT(glEnableVertexAttribArrayARB);

	LOAD_PROC_EXT(glGetUniformLocationARB);
	LOAD_PROC_EXT(glUniform4fvARB);
	LOAD_PROC_EXT(glUniformMatrix3x4fv);

	LOAD_PROC_EXT(glUniform1iARB);
	LOAD_PROC_EXT(glUniform1fARB);
	LOAD_PROC_EXT(glUniform1fvARB);
	LOAD_PROC_EXT(glUniform3fARB);

	if (iret == FALSE)
	{
		glslSupport = false;
		gEngfuncs.Con_Printf("---- Unable to LOAD_PROC_EXT for GLSL");
	}
	return iret;
}

int GLManager::Load_NV_fragment_program()
{
	NV_fragment_program_supported = FALSE;
	NV_fp_max_image_units = 0;
	NV_fp_max_texcoords = 0;
	if (IsExtensionSupported("GL_NV_fragment_program"))
	{
		int iret = TRUE;

		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS_NV, &NV_fp_max_image_units);
		glGetIntegerv(GL_MAX_TEXTURE_COORDS_NV, &NV_fp_max_texcoords);
		LOAD_PROC_EXT(glGenProgramsNV);
		LOAD_PROC_EXT(glBindProgramNV);
		LOAD_PROC_EXT(glLoadProgramNV);

		NV_fragment_program_supported = iret;
	}
	return NV_fragment_program_supported;
}

int GLManager::Load_ARB_texture_cube_map()
{
	ARB_texture_cube_map_supported = FALSE;
	if (IsExtensionSupported("GL_ARB_texture_cube_map"))
		ARB_texture_cube_map_supported = TRUE;

	return ARB_texture_cube_map_supported;
}

int GLManager::Load_EXT_texture_3D()
{
	EXT_3Dtexture_supported = FALSE;
	if (IsExtensionSupported("GL_EXT_texture3D"))
	{
		int iret = TRUE;
		LOAD_PROC_EXT(glTexImage3DEXT);
		EXT_3Dtexture_supported = iret;		
	}
	return EXT_3Dtexture_supported;
}

int GLManager::Load_ARB_VBO()
{
	ARB_VBO_supported = FALSE;
	if (IsExtensionSupported("GL_ARB_vertex_buffer_object"))
	{
		int iret = TRUE;

		LOAD_PROC_EXT(glBindBufferARB);
		LOAD_PROC_EXT(glGenBuffersARB);
		LOAD_PROC_EXT(glBufferDataARB);
		LOAD_PROC_EXT(glDeleteBuffersARB);
		LOAD_PROC_EXT(glGetBufferParameterivARB);

		ARB_VBO_supported = iret;
	}
	return ARB_VBO_supported;
}

int GLManager::Load_Rectangle_Textures()
{
	texture_rectangle_supported = FALSE;
	texture_rectangle_max_size = 0;
	if (IsExtensionSupported("GL_NV_texture_rectangle") || IsExtensionSupported("GL_ARB_texture_rectangle") || IsExtensionSupported("GL_EXT_texture_rectangle"))
	{
		glGetIntegerv(GL_MAX_RECTANGLE_TEXTURE_SIZE_NV, &texture_rectangle_max_size);		
		texture_rectangle_supported = TRUE;
	}
	return texture_rectangle_supported;
}
	



// GLU functions
void GLManager::gluLookAt( GLdouble eyex, GLdouble eyey, GLdouble eyez,
			GLdouble centerx, GLdouble centery, GLdouble centerz,
			GLdouble upx, GLdouble upy, GLdouble upz )
{
   GLdouble m[16];
   GLdouble x[3], y[3], z[3];
   GLdouble mag;

   /* Make rotation matrix */

   /* Z vector */
   z[0] = eyex - centerx;
   z[1] = eyey - centery;
   z[2] = eyez - centerz;
   mag = sqrt( z[0]*z[0] + z[1]*z[1] + z[2]*z[2] );
   if (mag) {  /* mpichler, 19950515 */
      z[0] /= mag;
      z[1] /= mag;
      z[2] /= mag;
   }

   /* Y vector */
   y[0] = upx;
   y[1] = upy;
   y[2] = upz;

   /* X vector = Y cross Z */
   x[0] =  y[1]*z[2] - y[2]*z[1];
   x[1] = -y[0]*z[2] + y[2]*z[0];
   x[2] =  y[0]*z[1] - y[1]*z[0];

   /* Recompute Y = Z cross X */
   y[0] =  z[1]*x[2] - z[2]*x[1];
   y[1] = -z[0]*x[2] + z[2]*x[0];
   y[2] =  z[0]*x[1] - z[1]*x[0];

   /* mpichler, 19950515 */
   /* cross product gives area of parallelogram, which is < 1.0 for
    * non-perpendicular unit-length vectors; so normalize x, y here
    */

   mag = sqrt( x[0]*x[0] + x[1]*x[1] + x[2]*x[2] );
   if (mag) {
      x[0] /= mag;
      x[1] /= mag;
      x[2] /= mag;
   }

   mag = sqrt( y[0]*y[0] + y[1]*y[1] + y[2]*y[2] );
   if (mag) {
      y[0] /= mag;
      y[1] /= mag;
      y[2] /= mag;
   }

#define M(row,col)  m[col*4+row]
   M(0,0) = x[0];  M(0,1) = x[1];  M(0,2) = x[2];  M(0,3) = 0.0;
   M(1,0) = y[0];  M(1,1) = y[1];  M(1,2) = y[2];  M(1,3) = 0.0;
   M(2,0) = z[0];  M(2,1) = z[1];  M(2,2) = z[2];  M(2,3) = 0.0;
   M(3,0) = 0.0;   M(3,1) = 0.0;   M(3,2) = 0.0;   M(3,3) = 1.0;
#undef M
   glMultMatrixd( m );

   /* Translate Eye to Origin */
   glTranslated( -eyex, -eyey, -eyez );
}
#define shuffle(a, b, c) (((a)<<4) | ((b)<<2) | ((c)))

//==========================
// SSE DotProduct Plane EQ
//
// Thanks Richard
//==========================
inline void SSEDotProductSub(float *result, vec3_t *v0, vec3_t *v1, float *subval)
{
	_asm{
		mov             esi, v0
			mov             edi, v1
			mov             eax, result;
		mov             edx, subval;

		movups  xmm0, [esi];
		movups  xmm1, [edi];

		mulps   xmm0, xmm1;

		movups  xmm2, xmm0;
		shufps  xmm2, xmm0, shuffle(0x01, 0x00, 0x02);
		addps   xmm2, xmm0;
		shufps  xmm0, xmm2, shuffle(0x02, 0x00, 0x01);
		addps   xmm0, xmm2;

		subss   xmm0, [edx];
		movss[eax], xmm0;
	}
}

//==========================
// SSE DotProduct world coord on Studio Models
//
// Thanks Richard
//==========================
inline void SSEDotProductWorld(float* result, const float* v0, const float* v1)
{
	_asm{
		mov             esi, v0
			mov             edi, v1
			mov             eax, result;

		movups  xmm0, [esi];
		movups  xmm1, [edi];

		mulps   xmm0, xmm1;

		movups  xmm2, xmm0;
		shufps  xmm2, xmm0, shuffle(0x01, 0x00, 0x02);
		addps   xmm2, xmm0;
		shufps  xmm0, xmm2, shuffle(0x02, 0x00, 0x01);
		addps   xmm0, xmm2;

		addss   xmm0, [edi + 12];
		movss[eax], xmm0;
	}
}

//==========================
// SSE DotProduct
//
// Thanks Richard
//==========================
inline void DotProductSSE(float* result, const float* v0, const float* v1)
{
	_asm{
		mov             esi, v0
			mov             edi, v1
			mov             eax, result;

		movups  xmm0, [esi];
		movups  xmm1, [edi];

		mulps   xmm0, xmm1;

		movups  xmm2, xmm0;
		shufps  xmm2, xmm0, shuffle(0x01, 0x00, 0x02);
		addps   xmm2, xmm0;
		shufps  xmm0, xmm2, shuffle(0x02, 0x00, 0x01);
		addps   xmm0, xmm2;

		movss[eax], xmm0;
	}
}

/*
====================
VectorAddSSE

Thanks Richard
====================
*/
inline void VectorAddSSE(const float* v0, const float* v1, const float* result)
{
	_asm {
		mov	esi, v0
			mov	edi, v1
			mov	eax, result

			movss	xmm0, [esi]
			movss	xmm1, [esi + 4]
			movss   xmm2, [esi + 8]

			addss	xmm0, [edi]
			addss	xmm1, [edi + 4]
			addss	xmm2, [edi + 8]

			movss[eax], xmm0
			movss[eax + 4], xmm1
			movss[eax + 8], xmm2
	}
}

/*
====================
VectorSubtract

Thanks Richard
====================
*/
inline void VectorMASSE(const float *veca, float scale, const float *vecb, float *vecc)
{
	_asm {
		mov		eax, veca;
		mov		ebx, vecb;
		mov		ecx, vecc;
		movss	xmm7, scale;

		; scale*vecb
			movss	xmm0, [ebx];
		movss	xmm1, [ebx + 4];
		movss	xmm2, [ebx + 8];
		mulss	xmm0, xmm7;
		mulss	xmm1, xmm7;
		mulss	xmm2, xmm7;

		; (scale*vecb) + veca
			movss	xmm3, [eax];
		movss	xmm4, [eax + 4];
		movss	xmm5, [eax + 8];
		addss	xmm0, xmm3;
		addss	xmm1, xmm4;
		addss	xmm2, xmm5;

		; return_it
			movss[ecx], xmm0;
		movss[ecx + 4], xmm1;
		movss[ecx + 8], xmm2;
	}
}
/*
====================
VectorRotateSSE

====================
*/
inline void VectorRotateSSE(const float *in1, float in2[3][4], float *out)
{
	DotProductSSE(&out[0], in1, in2[0]);
	DotProductSSE(&out[1], in1, in2[1]);
	DotProductSSE(&out[2], in1, in2[2]);
}

/*
====================
VectorTransformSSE

====================
*/
inline void VectorTransformSSE(const float *in1, float in2[3][4], float *out)
{
	SSEDotProductWorld(&out[0], in1, in2[0]);
	SSEDotProductWorld(&out[1], in1, in2[1]);
	SSEDotProductWorld(&out[2], in1, in2[2]);
}