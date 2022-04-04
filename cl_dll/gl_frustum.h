#ifndef __FRUSTUM_CLASS_
#define __FRUSTUM_CLASS_

// bsp stuff
class FrustumCheck
{
public:
	qboolean	R_CullBox(vec3_t mins, vec3_t maxs);
	void		R_SetFrustum(vec3_t vangles, vec3_t vorigin, float viewsize);
	void		R_SetFrustum(vec3_t vangles, vec3_t vorigin, float view_hor, float view_ver, float dist);
	void		GL_FrustumSetClipPlanes();
	void		GL_DisableClipPlanes();

private:
	mplane_t	frustum[4];
	mplane_t	farclip; // buz
	int			farclipset; // buz

	vec3_t	m_origin;
	vec3_t	m_angles;
	float	m_fov_hor, m_fov_ver, m_dist;
};

#endif //__FRUSTUM_CLASS_