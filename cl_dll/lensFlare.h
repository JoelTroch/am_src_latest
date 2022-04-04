#ifndef __LENSES_H__
#define __LENSES_H__

class CLensFlare{
public:
	void InitLensFlares();
	void SetSunParams(vec3_t pos,float pitch);
	void DrawEffect();

protected:
	bool enabled;
	vec3_t sunPosition;
	float sunPitch;
	cl_texture_t *textures[8];
};

extern CLensFlare gLensFlares;
#endif