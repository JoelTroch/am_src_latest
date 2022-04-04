#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "glmanager.h"

#include "gl_renderer.h"
#include "gl_shaders.h"
CGLSLShader *genericShader = 0;
CGLSLStudio *studioShader = 0;
CGLSLStudioProp *studioShaderProp = 0;
CGLSLStudioPropNocube *studioShaderPropNocube = 0;

CGLSLStudioNocube *studioShaderNocube = 0;
CGLSLColorCorrection *colorCorrectionShader = 0;

CGLSLSoftParticle *softParticlesShader = 0;
CGLSLSSAOShader *SSAOShader = 0;
CGLSLSSAOMixShader *SSAOMixShader = 0;

void loadAllShaders()
{
	//if (genericShader) return;

	gEngfuncs.Con_Printf("---- LOADING SHADER ----");
	genericShader = new CGLSLShader("shaders/glsl/generic.f",
		"shaders/glsl/generic.v");

	studioShader = new CGLSLStudio("shaders/glsl/studio.f",
		"shaders/glsl/studio.v");

	studioShaderNocube = new CGLSLStudioNocube("shaders/glsl/studioNocube.f",
		"shaders/glsl/studioNocube.v");

	studioShaderProp = new CGLSLStudioProp("shaders/glsl/studioProp.f",
		"shaders/glsl/studioProp.v");

	studioShaderPropNocube = new CGLSLStudioPropNocube("shaders/glsl/studioPropNocube.f",
		"shaders/glsl/studioPropNocube.v");

	colorCorrectionShader = new CGLSLColorCorrection("shaders/glsl/colorCorrection.f",
		"shaders/glsl/colorCorrection.v");

	softParticlesShader = new CGLSLSoftParticle("shaders/glsl/softParticle.f",
		"shaders/glsl/softParticle.v");

	SSAOShader = new CGLSLSSAOShader("shaders/glsl/SSAO.f",
		"shaders/glsl/SSAO.v");

	SSAOMixShader = new CGLSLSSAOMixShader("shaders/glsl/SSAOMix.f",
		"shaders/glsl/SSAOMix.v");
}

CGLSLShader::CGLSLShader(char *frag, char *vert)
{
	int len,lenvert;
	len = lenvert = 0;
	const char *buf = (const char *)AM_LoadFile(frag, 5, &len);
	if (!buf || len <= 0)
	{
		gEngfuncs.Con_Printf("unable to load shader file: %s %i\n", frag,len);
		return;
	}

	const char *bufVert = (const char *)AM_LoadFile(vert, 5, &lenvert);
	if (!bufVert || lenvert <= 0)
	{
		gEngfuncs.Con_Printf("unable to load shader file: %s %i\n", frag,lenvert);
		return;
	}
	
	program = gl.glCreateProgramObjectARB();
	vertex = gl.glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	fragment = gl.glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

	gl.glShaderSourceARB(fragment, 1, &buf, &len);
	gl.glShaderSourceARB(vertex, 1, &bufVert, &lenvert);

	gl.glCompileShader(vertex);

	gEngfuncs.Con_Printf("Vertex %s\n",vert);
	printLog(vertex); 

	gl.glAttachObjectARB(program, vertex);
	gl.glCompileShader(fragment);

	gEngfuncs.Con_Printf("Fragment %s\n",frag);
	printLog(fragment);

	gl.glAttachObjectARB(program, fragment);
	gl.glLinkProgramARB(program);

	AM_FreeFile((byte*)buf);
	AM_FreeFile((byte*)bufVert);
}

CGLSLShader::~CGLSLShader()
{

}

void CGLSLShader::printLog(GLhandleARB obj)
{
	int infologLength = 0;
	int charsWritten = 0;
	char *infoLog;

	gl.glGetObjectParameterivARB(obj, GL_OBJECT_INFO_LOG_LENGTH_ARB,
		&infologLength);

	if (infologLength > 0)
	{
		infoLog = (char *)malloc(infologLength);
		gl.glGetInfoLogARB(obj, infologLength, &charsWritten, infoLog);
		gEngfuncs.Con_Printf("%s\n", infoLog);
		free(infoLog);
	}
}

void CGLSLShader::bind()
{
	gl.glUseProgramObjectARB(program);
}

void CGLSLShader::unbind()
{
	gl.glUseProgramObjectARB(0);
}

PFNGLPROGRAMLOCALPARAMETER4FARBPROC glProgramLocalParameter4fARB = NULL;
GLboolean LoadProgramNative(GLenum target, GLenum format, GLsizei len, const char *string);
bool LoadShaderFile(char *fname, unsigned int &idx, bool fragment)
{
	glProgramLocalParameter4fARB = (PFNGLPROGRAMLOCALPARAMETER4FARBPROC)wglGetProcAddress("glProgramLocalParameter4fARB");

/*	if (!shaderssupports)
	{
		gEngfuncs.Con_Printf("Sorry, your videocard doesn't supports ARB shaders.\n");
		return false;
	}*/

	int target = GL_VERTEX_PROGRAM_ARB;

	if (fragment)
		target = GL_FRAGMENT_PROGRAM_ARB;

	int l;
	char *bb = (char*)gEngfuncs.COM_LoadFile(fname, 5, &l);

	if (!bb)
	{
		gEngfuncs.Con_Printf("Shader file %s not found.\n", fname);
		return false;
	}

	gl.glEnable(target);
	gl.glGenProgramsARB(1, &idx);
	gl.glBindProgramARB(target, idx);
	if (LoadProgramNative(target, GL_PROGRAM_FORMAT_ASCII_ARB, l - 1, bb))
		gEngfuncs.Con_Printf("Shader loaded %s\n", fname);
	else
	{
		gl.glDisable(target);
		gEngfuncs.Con_Printf("Can't load shader %s\n", fname);
		return false;
	}
	gl.glDisable(target);
	gEngfuncs.COM_FreeFile(bb);

	return true;
}

bool LoadArbShader(unsigned int *shaderId, int shaderType, char shadertext[],int size,char *shadername)
{
	int iErrorPos, iIsNative;

	gl.glEnable(GL_FRAGMENT_PROGRAM_ARB);
	gl.glGenProgramsARB(1, shaderId);
	gl.glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, *shaderId);

	gl.glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, size - 1, shadertext);
	gl.glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &iErrorPos);
	gl.glGetProgramivARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB, &iIsNative);
	gl.glDisable(GL_FRAGMENT_PROGRAM_ARB);

	if (iErrorPos != -1 || !iIsNative)
	{
		gEngfuncs.Con_Printf("Unable to compile shader: %s\n", shadername);
		return FALSE;
	}

	gEngfuncs.Con_Printf("Succes with loading shader: %s\n", shadername);
	return TRUE;
}
