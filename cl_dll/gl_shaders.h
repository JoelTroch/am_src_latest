#ifndef __SHADERS_H__
#define __SHADERS_H__

enum shaderType
{
	VERTEX = 0,
	FRAGMENT,
};

bool LoadArbShader(unsigned int *shaderId,int shaderType,char shadertext[],int size,char *shadername = "noname");
extern PFNGLPROGRAMLOCALPARAMETER4FARBPROC glProgramLocalParameter4fARB;

class CGLSLShader
{
public:
	CGLSLShader(char *fragment, char *vertex);
	~CGLSLShader();
	void printLog(GLhandleARB handle);

	GLuint program;
	GLuint vertex;
	GLuint fragment;

	virtual void bind();
	virtual void unbind();
};

class CGLSLStudioNocube : public CGLSLShader
{
public:
	CGLSLStudioNocube(char *fragment, char *vertex) :
		CGLSLShader(fragment,vertex)
	{
		boneid = gl.glGetAttribLocationARB(program, "boneid");
		transform = gl.glGetUniformLocationARB(program, "mtx");
	}
	GLint boneid;
	GLint transform;
};

class CGLSLColorCorrection : public CGLSLShader
{
public:
	CGLSLColorCorrection(char *fragment, char *vertex) :
		CGLSLShader(fragment,vertex)
	{
		rgbsat = gl.glGetUniformLocationARB(program, "rgbsat");
	}

	GLint rgbsat;
};

class CGLSLStudio : public CGLSLShader
{
public:
	CGLSLStudio(char *fragment, char *vertex) :
		CGLSLShader(fragment,vertex)
	{
		lightdirection = gl.glGetUniformLocationARB(program, "direction");
		boneid = gl.glGetAttribLocationARB(program, "boneid");
		tangent = gl.glGetAttribLocationARB(program, "tangent");
		binormal = gl.glGetAttribLocationARB(program, "binormal");

		transform = gl.glGetUniformLocationARB(program, "mtx");
		colors = gl.glGetUniformLocationARB(program, "colors");
		camuniform = gl.glGetUniformLocationARB(program, "camunit");
		addlight = gl.glGetUniformLocationARB(program, "addlight");

		//fragment
		textureUniform = gl.glGetUniformLocationARB(program, "texture");
		maskUniform = gl.glGetUniformLocationARB(program, "mask");
		cubemapUniform = gl.glGetUniformLocationARB(program, "cubemap");
		enablecubes = gl.glGetUniformLocationARB(program, "ecubem");
		dynlight = gl.glGetUniformLocationARB(program, "dynvalues");

		skycolor = gl.glGetUniformLocationARB(program, "skycolor");
		skyvec = gl.glGetUniformLocationARB(program, "skyvec");

	}
	GLint boneid;
	GLint transform;
	GLint colors;
	GLint lightdirection;
	GLint dynlight;
	GLint tangent;
	GLint binormal;

	GLint skycolor;
	GLint skyvec;

	GLint camuniform;
	GLint addlight;

	GLint textureUniform;
	GLint maskUniform;
	GLint cubemapUniform;
	GLint enablecubes;

};

class CGLSLSoftParticle : public CGLSLShader
{
public:
	CGLSLSoftParticle(char *fragment, char *vertex) :
		CGLSLShader(fragment,vertex)
	{
		texture = gl.glGetUniformLocationARB(program, "particleMap");
		depth = gl.glGetUniformLocationARB(program, "depthMap");
	}
	GLint texture;
	GLint depth;
};

class CGLSLStudioProp : public CGLSLShader
{
public:
	CGLSLStudioProp(char *fragment, char *vertex) :
		CGLSLShader(fragment, vertex)
	{
		colorattrib = gl.glGetAttribLocationARB(program, "colorattrib");

		camuniform = gl.glGetUniformLocationARB(program, "camunit");

		//fragment
		textureUniform = gl.glGetUniformLocationARB(program, "texture");
		maskUniform = gl.glGetUniformLocationARB(program, "mask");
		cubemapUniform = gl.glGetUniformLocationARB(program, "cubemap");
		enablecubes = gl.glGetUniformLocationARB(program, "ecubem");
		dynlight = gl.glGetUniformLocationARB(program, "dynvalues");

		skycolor = gl.glGetUniformLocationARB(program, "skycolor");
		skyvec = gl.glGetUniformLocationARB(program, "skyvec");
	}
	GLint dynlight;
	GLint colorattrib;

	GLint skycolor;
	GLint skyvec;
	GLint camuniform;

	GLint textureUniform;
	GLint maskUniform;
	GLint cubemapUniform;
	GLint enablecubes;
};


class CGLSLStudioPropNocube : public CGLSLShader
{
public:
	CGLSLStudioPropNocube(char *fragment, char *vertex) :
		CGLSLShader(fragment, vertex)
	{

	}

};

class CGLSLSSAOShader : public CGLSLShader
{
public:
	CGLSLSSAOShader(char *fragment, char *vertex) :
		CGLSLShader(fragment, vertex)
	{
		depth = gl.glGetUniformLocationARB(program, "depthMap");
		local0 = gl.glGetUniformLocationARB(program, "local0");
	}

	GLint depth;
	GLint local0;
};

class CGLSLSSAOMixShader : public CGLSLShader
{
public:
	CGLSLSSAOMixShader(char *fragment, char *vertex) :
		CGLSLShader(fragment, vertex)
	{
		blured = gl.glGetUniformLocationARB(program, "bluredMap");
		screen = gl.glGetUniformLocationARB(program, "screenMap");

		local0 = gl.glGetUniformLocationARB(program, "local0");
	}

	GLint blured, screen;
	GLint local0;
};

void loadAllShaders();
extern CGLSLShader *genericShader;
extern CGLSLStudio *studioShader;
extern CGLSLStudioProp *studioShaderProp;
extern CGLSLStudioPropNocube *studioShaderPropNocube;

extern CGLSLStudioNocube *studioShaderNocube;
extern CGLSLColorCorrection *colorCorrectionShader;
extern CGLSLSoftParticle *softParticlesShader;
extern CGLSLSSAOShader *SSAOShader;
extern CGLSLSSAOMixShader *SSAOMixShader;

#endif