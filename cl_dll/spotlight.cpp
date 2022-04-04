#include <math.h>
#include "hud.h"
#include "cl_util.h"
#include "com_model.h"

#include <Windows.h>

#include <gl/GL.h>
#include <gl/GLU.h>
#include <gl/glext.h>

#define	PLANE_X			0
#define	PLANE_Y			1
#define	PLANE_Z			2

#define	SURF_PLANEBACK		2
#define	SURF_DRAWSKY		4
#define SURF_DRAWSPRITE		8
#define SURF_DRAWTURB		0x10
#define SURF_DRAWTILED		0x20
#define SURF_DRAWBACKGROUND	0x40
#define SURF_UNDERWATER		0x80
#define SURF_DONTWARP		0x100
#define BACKFACE_EPSILON	0.01
#define M_PI				3.141592

int MAX_TU_supported = 4;
int use_vertex_array = 0;

// up / down
#define	PITCH	0
// left / right
#define	YAW		1
// fall over
#define	ROLL	2 

int currentbinds[32];
int spot_texture;
int norm_cubemap_id;
PFNGLACTIVETEXTUREARBPROC       glActiveTextureARB = NULL;
PFNGLCLIENTACTIVETEXTUREARBPROC       glClientActiveTextureARB = NULL;

vec3_t current_light_origin;
glpoly_t *chain;
float radius = 700.0f;
float cone_hor = 64;
float cone_ver = 64;
int attenuation_1d;
int current_ext_texture_id = (1 << 25);
int texid;
int default_normalmap_id;

int CreateSpotlightAttenuationTexture()
{
	color24 buf[256];
	for (int i = 0; i < 256; i++)
	{
		float dist = (float)i;
		//	float att = (1 - ((dist*dist) / (256*256))) * 255;
		float att = (((dist*dist) / (256 * 256)) - 1) * -255;

		// clear attenuation at ends to prevent light go outside
		if (i == 255 || i == 0)
			att = 0;

		buf[i].r = buf[i].g = buf[i].b = (unsigned char)att;
	}

	glBindTexture(GL_TEXTURE_1D, current_ext_texture_id);
	glTexImage1D(GL_TEXTURE_1D, 0, 3, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, buf);
	glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

//	Log("Created spotlight attenuation texture\n");
	current_ext_texture_id++;
	return (current_ext_texture_id - 1);
}

//===================================
// CreateNormalizationCubemap
//
//		thanks steps3d.narod.ru
//===================================
#define NORM_CUBE_SIZE	(32)

void getCubeVector(int side, int x, int y, vec3_t &v)
{
	float s = ((float)x + 0.5f) / (float)NORM_CUBE_SIZE;
	float t = ((float)y + 0.5f) / (float)NORM_CUBE_SIZE;
	float sc = 2 * s - 1;
	float tc = 2 * t - 1;

	switch (side)
	{
	case 0:
		v[0] = 1; v[1] = -tc; v[2] = -sc;
		break;

	case 1:
		v[0] = -1; v[1] = -tc; v[2] = sc;
		break;

	case 2:
		v[0] = sc; v[1] = 1; v[2] = tc;
		break;

	case 3:
		v[0] = sc; v[1] = -1; v[2] = -tc;
		break;

	case 4:
		v[0] = sc; v[1] = -tc; v[2] = 1;
		break;

	case 5:
		v[0] = -sc; v[1] = -tc; v[2] = -1;
		break;
	}
	VectorNormalize(v);
}

int CreateNormalizationCubemap()
{
	vec3_t v;
	byte pixels[NORM_CUBE_SIZE * NORM_CUBE_SIZE * 3];

	glEnable(GL_TEXTURE_CUBE_MAP_ARB);
	glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, current_ext_texture_id);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	for (int side = 0; side < 6; side++)
	{
		for (int x = 0; x < NORM_CUBE_SIZE; x++)
		for (int y = 0; y < NORM_CUBE_SIZE; y++)
		{
			int offs = 3 * (y * NORM_CUBE_SIZE + x);

			getCubeVector(side, x, y, v);

			pixels[offs] = 128 + 127 * v[0];
			pixels[offs + 1] = 128 + 127 * v[1];
			pixels[offs + 2] = 128 + 127 * v[2];
			//	gEngfuncs.Con_Printf("%d, %d, %d\n", pixels[0], pixels[1], pixels[2]);
		}

		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB + side, 0, GL_RGB,
			NORM_CUBE_SIZE, NORM_CUBE_SIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
	}

	//Log("Created normalization cubemap\n");
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glDisable(GL_TEXTURE_CUBE_MAP_ARB);

	current_ext_texture_id++;
	return (current_ext_texture_id - 1);
}

void GenerateMipMap(color24 *in, int width, int height)
{
	color24	*out = in;
	height >>= 1;
	for (int y = 0; y < height; y++, in += width * 2)
	{
		for (int x = 0; x < width / 2; x++, out++)
		{
			out->r = (in[x * 2].r + in[x * 2 + 1].r + in[width + x * 2].r + in[width + x * 2 + 1].r) >> 2;
			out->g = (in[x * 2].g + in[x * 2 + 1].g + in[width + x * 2].g + in[width + x * 2 + 1].g) >> 2;
			out->b = (in[x * 2].b + in[x * 2 + 1].b + in[width + x * 2].b + in[width + x * 2 + 1].b) >> 2;
		}
	}
}

#define TEXBUFFER_SIZE (512*512)
typedef struct tgaheader_s
{
	unsigned char	IdLength;
	unsigned char	ColorMap;
	unsigned char	DataType;
	unsigned char	unused[5];
	unsigned short	OriginX;
	unsigned short	OriginY;
	unsigned short	Width;
	unsigned short	Height;
	unsigned char	BPP;
	unsigned char	Description;
} TGAheader;

int CreateTexture(const char* filename, int mipmaps, int useid)
{
	color24 buf[TEXBUFFER_SIZE];

	int length = 0;
	char *file = (char *)gEngfuncs.COM_LoadFile((char*)filename, 5, &length);
	if (!file)
	{
		gEngfuncs.Con_Printf("CreateTexture failed to load file: %s\n", filename);
		return 0;
	}

	TGAheader *hdr = (TGAheader *)file;
	if (!(/*hdr->DataType == 10 ||*/ hdr->DataType == 2) || (hdr->BPP != 24))
	{
		gEngfuncs.Con_Printf("Error: texture %s uses unsupported data format -->\n(only 24-bit noncompressed TGA supported)\n", filename);
		gEngfuncs.COM_FreeFile(file);
		return 0;
	}

	int maxtexsize;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxtexsize);
	if (hdr->Width > maxtexsize || hdr->Height > maxtexsize)
	{
		gEngfuncs.Con_Printf("Error: texture %s: sizes %d x %d are not supported by your hardware\n", filename, hdr->Width, hdr->Height);
		gEngfuncs.COM_FreeFile(file);
		return 0;
	}

	int numpixels = hdr->Width * hdr->Height;
	if (numpixels > TEXBUFFER_SIZE)
	{
		gEngfuncs.Con_Printf("Error: texture %s (%d x %d) doesn't fit in static buffer\n", filename, hdr->Width, hdr->Height);
		gEngfuncs.COM_FreeFile(file);
		return 0;
	}

	color24 *pixels = (color24*)(file + sizeof(TGAheader)+hdr->IdLength);
	color24 *dest = buf;

	// non-compressed image
	for (int i = 0; i < numpixels; i++, pixels++, dest++)
	{
		dest->r = pixels->b;
		dest->g = pixels->g;
		dest->b = pixels->r;
	}

	// flip image
	if (!(hdr->Description & 0x20))
	{
		color24 line[1024];
		for (int i = 0; i < hdr->Height / 2; i++)
		{
			int linelen = hdr->Width * sizeof(color24);
			memcpy(line, &buf[i*hdr->Width], linelen);
			memcpy(&buf[i*hdr->Width], &buf[(hdr->Height - i - 1)*hdr->Width], linelen);
			memcpy(&buf[(hdr->Height - i - 1)*hdr->Width], line, linelen);
		}
	}

	int minfilter = GL_NEAREST;
	minfilter = GL_LINEAR_MIPMAP_NEAREST;

	// upload image
	if (useid)
		glBindTexture(GL_TEXTURE_2D, useid);
	else
		glBindTexture(GL_TEXTURE_2D, current_ext_texture_id);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minfilter);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, hdr->Width, hdr->Height, 0, GL_RGB, GL_UNSIGNED_BYTE, buf);

	int miplevel = 0;
	int width = hdr->Width;
	int height = hdr->Height;
	while (width > 1 || height > 1)
	{
		GenerateMipMap(buf, width, height);
		width >>= 1;
		height >>= 1;
		if (width < 1) width = 1;
		if (height < 1) height = 1;

		miplevel++;
		glTexImage2D(GL_TEXTURE_2D, miplevel, 3, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, buf);
	}

	gEngfuncs.COM_FreeFile(file);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	if (useid)
		return useid;

	current_ext_texture_id++;

	return (current_ext_texture_id - 1);
}

int CreateDefaultNormalMap()
{
	color24 buf[16 * 16];

	for (int i = 0; i < 256; i++)
	{
		buf[i].r = 127;
		buf[i].g = 127;
		buf[i].b = 255;
	}

	// upload image
	glBindTexture(GL_TEXTURE_2D, current_ext_texture_id);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 16, 16, 0, GL_RGB, GL_UNSIGNED_BYTE, buf);

	gEngfuncs.Con_Printf("Created default normal map\n");
	current_ext_texture_id++;
	return (current_ext_texture_id - 1);
}

void InitOpenGL()
{
	glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC)wglGetProcAddress("glActiveTextureARB");
	glClientActiveTextureARB = (PFNGLCLIENTACTIVETEXTUREARBPROC)wglGetProcAddress("glClientActiveTextureARB");

	texid = CreateTexture("gfx/flashlight.tga", 0, 0);

	if (texid)
		gEngfuncs.Con_Printf("CreateTexture got success.\n");
	else
		gEngfuncs.Con_Printf("Wtf?.\n");

	spot_texture = 0;	
	norm_cubemap_id = CreateNormalizationCubemap();
	attenuation_1d = CreateSpotlightAttenuationTexture();
	default_normalmap_id = CreateDefaultNormalMap();

	for (int i = 0; i < 32; i++)
		currentbinds[i] = -1;
	chain = NULL;
}

void SetProjectionTexCoords(float xmin, float ymin, float xmax, float ymax)
{
	float xdiff = (xmax - xmin) / 2;
	float ydiff = (ymax - ymin) / 2;
	glTranslatef(xmin + xdiff, ymin + ydiff, 0.0);
	glScalef(xdiff, -ydiff, 1.0);
}

void Bind2DTexture(GLenum texture, GLuint id)
{
	int idx = texture - GL_TEXTURE0_ARB;
	if (currentbinds[idx] != id)
	{
		glActiveTextureARB(texture);
		glBindTexture(GL_TEXTURE_2D, id);
		currentbinds[idx] = id;
	}
}

int IsPitchReversed(float pitch)
{
	int quadrant = int(pitch / 90) % 4;
	if ((quadrant == 1) || (quadrant == 2)) return TRUE;
	return FALSE;
}

void SetupTexMatrix(msurface_t *s, const vec3_t &vec)
{
/*	int facedataindex = (s->polys->flags >> 16) & 0xFFFF;
	BrushFace* ext = &faces_extradata[facedataindex];

	float m[16];
	m[0] = -ext->s_tangent[0];
	m[4] = -ext->s_tangent[1];
	m[8] = -ext->s_tangent[2];
	m[12] = DotProduct(ext->s_tangent, vec);
	m[1] = -ext->t_tangent[0];
	m[5] = -ext->t_tangent[1];
	m[9] = -ext->t_tangent[2];
	m[13] = DotProduct(ext->t_tangent, vec);
	m[2] = -ext->normal[0];
	m[6] = -ext->normal[1];
	m[10] = -ext->normal[2];
	m[14] = DotProduct(ext->normal, vec);
	m[3] = m[7] = m[11] = 0;
	m[15] = 1;

	glActiveTextureARB(GL_TEXTURE0_ARB);
	glMatrixMode(GL_TEXTURE);
	glLoadMatrixf(m);*/

	int t_id = 2;
	int n_id = 3;
	vec3_t tan,t_tan;
	tan = t_tan = Vector(0, 0, 0);
	vec3_t norm = s->plane->normal;

	float m[16];
	m[0] = -tan[0];
	m[4] = -tan[1];
	m[8] = -tan[2];
	m[12] = DotProduct(tan, vec);
	m[1] = -t_tan[0];
	m[5] = -t_tan[1];
	m[9] = -t_tan[2];
	m[13] = DotProduct(t_tan, vec);
	m[2] = -norm[0];
	m[6] = -norm[1];
	m[10] = -norm[2];
	m[14] = DotProduct(norm, vec);
	m[3] = m[7] = m[11] = 0;
	m[15] = 1;

	glActiveTextureARB(GL_TEXTURE0_ARB);
	glMatrixMode(GL_TEXTURE);
	glLoadMatrixf(m);
}

int BoxOnPlaneSide(vec3_t &emins, vec3_t &emaxs, mplane_t *p)
{
	float	dist1, dist2;
	int		sides;

	// general case
	switch (p->signbits)
	{
	case 0:
		dist1 = p->normal[0] * emaxs[0] + p->normal[1] * emaxs[1] + p->normal[2] * emaxs[2];
		dist2 = p->normal[0] * emins[0] + p->normal[1] * emins[1] + p->normal[2] * emins[2];
		break;
	case 1:
		dist1 = p->normal[0] * emins[0] + p->normal[1] * emaxs[1] + p->normal[2] * emaxs[2];
		dist2 = p->normal[0] * emaxs[0] + p->normal[1] * emins[1] + p->normal[2] * emins[2];
		break;
	case 2:
		dist1 = p->normal[0] * emaxs[0] + p->normal[1] * emins[1] + p->normal[2] * emaxs[2];
		dist2 = p->normal[0] * emins[0] + p->normal[1] * emaxs[1] + p->normal[2] * emins[2];
		break;
	case 3:
		dist1 = p->normal[0] * emins[0] + p->normal[1] * emins[1] + p->normal[2] * emaxs[2];
		dist2 = p->normal[0] * emaxs[0] + p->normal[1] * emaxs[1] + p->normal[2] * emins[2];
		break;
	case 4:
		dist1 = p->normal[0] * emaxs[0] + p->normal[1] * emaxs[1] + p->normal[2] * emins[2];
		dist2 = p->normal[0] * emins[0] + p->normal[1] * emins[1] + p->normal[2] * emaxs[2];
		break;
	case 5:
		dist1 = p->normal[0] * emins[0] + p->normal[1] * emaxs[1] + p->normal[2] * emins[2];
		dist2 = p->normal[0] * emaxs[0] + p->normal[1] * emins[1] + p->normal[2] * emaxs[2];
		break;
	case 6:
		dist1 = p->normal[0] * emaxs[0] + p->normal[1] * emins[1] + p->normal[2] * emins[2];
		dist2 = p->normal[0] * emins[0] + p->normal[1] * emaxs[1] + p->normal[2] * emaxs[2];
		break;
	case 7:
		dist1 = p->normal[0] * emins[0] + p->normal[1] * emins[1] + p->normal[2] * emins[2];
		dist2 = p->normal[0] * emaxs[0] + p->normal[1] * emaxs[1] + p->normal[2] * emaxs[2];
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

mplane_t	frustum[4];
mplane_t	farclip; // buz
int			farclipset; // buz

vec3_t	m_origin;
vec3_t	m_angles;
float	m_fov_hor, m_fov_ver, m_dist;

int SignbitsForPlane(mplane_t *out)
{
	int	bits, j;
	// for fast box on planeside test
	bits = 0;
	for (j = 0; j<3; j++)
	{
		if (out->normal[j] < 0)
			bits |= 1 << j;
	}
	return bits;
}

void ProjectPointOnPlane(vec3_t &dst, const vec3_t &p, const vec3_t &normal)
{
	float d;
	vec3_t n;
	float inv_denom;

	inv_denom = 1.0F / DotProduct(normal, normal);

	d = DotProduct(normal, p) * inv_denom;

	n[0] = normal[0] * inv_denom;
	n[1] = normal[1] * inv_denom;
	n[2] = normal[2] * inv_denom;

	dst[0] = p[0] - d * n[0];
	dst[1] = p[1] - d * n[1];
	dst[2] = p[2] - d * n[2];
}

void PerpendicularVector(vec3_t &dst, const vec3_t &src)
{
	int	pos;
	int i;
	float minelem = 1.0F;
	vec3_t tempvec;

	/*
	** find the smallest magnitude axially aligned vector
	*/
	for (pos = 0, i = 0; i < 3; i++)
	{
		if (fabs(src[i]) < minelem)
		{
			pos = i;
			minelem = fabs(src[i]);
		}
	}
	tempvec[0] = tempvec[1] = tempvec[2] = 0.0F;
	tempvec[pos] = 1.0F;

	/*
	** project the point onto the plane defined by src
	*/
	ProjectPointOnPlane(dst, tempvec, src);

	/*
	** normalize the result
	*/
	VectorNormalize(dst);
}

#define DEG2RAD( a ) ( a * M_PI ) / 180.0F

void R_ConcatRotations(float in1[3][3], float in2[3][3], float out[3][3])
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

void RotatePointAroundVector(vec3_t &dst, const vec3_t &dir, const vec3_t &point, float degrees)
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

	PerpendicularVector(vr, dir);
	//CrossProduct( vr, vf, vup );
	vup = CrossProduct(vr, vf);

	m[0][0] = vr[0];
	m[1][0] = vr[1];
	m[2][0] = vr[2];

	m[0][1] = vup[0];
	m[1][1] = vup[1];
	m[2][1] = vup[2];

	m[0][2] = vf[0];
	m[1][2] = vf[1];
	m[2][2] = vf[2];

	memcpy(im, m, sizeof(im));

	im[0][1] = m[1][0];
	im[0][2] = m[2][0];
	im[1][0] = m[0][1];
	im[1][2] = m[2][1];
	im[2][0] = m[0][2];
	im[2][1] = m[1][2];

	memset(zrot, 0, sizeof(zrot));
	zrot[0][0] = zrot[1][1] = zrot[2][2] = 1.0F;

	zrot[0][0] = cos(DEG2RAD(degrees));
	zrot[0][1] = sin(DEG2RAD(degrees));
	zrot[1][0] = -sin(DEG2RAD(degrees));
	zrot[1][1] = cos(DEG2RAD(degrees));

	R_ConcatRotations(m, zrot, tmpmat);
	R_ConcatRotations(tmpmat, im, rot);

	for (i = 0; i < 3; i++)
	{
		dst[i] = rot[i][0] * point[0] + rot[i][1] * point[1] + rot[i][2] * point[2];
	}
}

void R_SetFrustum(vec3_t vangles, vec3_t vorigin, float view_hor, float view_ver, float dist)
{
	int i;
	vec3_t vpn, vright, vup;

	AngleVectors(vangles, vpn, vright, vup);

	// rotate VPN right by FOV_X/2 degrees
	RotatePointAroundVector(frustum[0].normal, vup, vpn, -(90 - view_hor / 2));
	// rotate VPN left by FOV_X/2 degrees
	RotatePointAroundVector(frustum[1].normal, vup, vpn, 90 - view_hor / 2);
	// rotate VPN up by FOV_X/2 degrees
	RotatePointAroundVector(frustum[2].normal, vright, vpn, 90 - (view_ver) / 2);
	// rotate VPN down by FOV_X/2 degrees
	RotatePointAroundVector(frustum[3].normal, vright, vpn, -(90 - (view_ver) / 2));

	for (i = 0; i<4; i++)
	{
		frustum[i].type = PLANE_ANYZ;
		frustum[i].dist = DotProduct(vorigin, frustum[i].normal);
		frustum[i].signbits = SignbitsForPlane(&frustum[i]);
	}

	// buz: set also clipping by distance
	vec3_t farpoint = vorigin + (vpn * dist);
	farclip.normal = vpn * -1;
	farclip.type = PLANE_ANYZ;
	farclip.dist = DotProduct(farpoint, farclip.normal);
	farclip.signbits = SignbitsForPlane(&farclip);
	farclipset = 1;

	// save params for building stencil clipping volume
	VectorCopy(vorigin, m_origin);
	VectorCopy(vangles, m_angles);
	m_fov_hor = view_hor;
	m_fov_ver = view_ver;
	m_dist = dist;
}


bool R_CullBox(vec3_t mins, vec3_t maxs)
{
	int i;
	for (i = 0; i<4; i++)
	{
		if (BoxOnPlaneSide(mins, maxs, &frustum[i]) == 2)
			return true;
	}

	// buz: also check clipping by distance
	if (farclipset)
	{
		if (BoxOnPlaneSide(mins, maxs, &farclip) == 2)
			return true;
	}

	return false;
}

mleaf_t *Mod_PointInLeaf(vec3_t p, model_t *model)
{
	mnode_t *node;
	float d;
	mplane_t *plane;

	//	if (!model || !model->nodes)
	//		Sys_Error ("Mod_PointInLeaf: bad model");
	node = model->nodes;
	while (1)
	{
		if (node->contents < 0)
			return (mleaf_t *)node;
		plane = node->plane;
		d = DotProduct(p, plane->normal) - plane->dist;
		if (d > 0)
			node = node->children[0];
		else
			node = node->children[1];
	}

	return NULL;	// never reached
}


void DrawGLPolyChain(glpoly_t *p, float soffset, float toffset)
{
	qboolean	dynamic = true;

	if (soffset == 0.0f && toffset == 0.0f)
		dynamic = false;

	for (; p != NULL; p = p->chain)
	{
		float	*v;
		int	i;

		glBegin(GL_POLYGON);

		v = p->verts[0];
		for (i = 0; i < p->numverts; i++, v += VERTEXSIZE)
		{
			if (!dynamic) glTexCoord2f(v[5], v[6]);
			else glTexCoord2f(v[5] - soffset, v[6] - toffset);
			glVertex3fv(v);
		}
		glEnd();
	}
}

void DrawGLPoly(glpoly_t *p)
{
	float		*v;
	int		i;

	glBegin(GL_POLYGON);

	for (i = 0, v = p->verts[0]; i < p->numverts; i++, v += VERTEXSIZE)
	{
		glTexCoord2f(v[3], v[4]);
		glVertex3fv(v);
	}

	glEnd();
}

void LightDrawPoly(msurface_t *surf)
{
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(-1, -32);
	DrawGLPoly(surf->polys);
	glDisable(GL_POLYGON_OFFSET_FILL);
}

void RecursiveDrawWorldLight(mnode_t *node)
{
	if (node->contents == CONTENTS_SOLID)
		return;		// solid

	model_t *world = gEngfuncs.GetEntityByIndex(0)->model;
	mleaf_t *leaf = Mod_PointInLeaf(gHUD.m_vecOrigin, world);
	int curvisframe = leaf->visframe;

	if (node->visframe != curvisframe)
		return;

	// buz: visible surfaces already marked
	if (node->contents < 0)
		return;

	if (R_CullBox(node->minmaxs, node->minmaxs + 3)) // cull from spotlight cone
		return;
	
	// node is just a decision point, so go down the apropriate sides
	// find which side of the node we are on
	int side;
	double dot;
	mplane_t *plane = node->plane;
	switch (plane->type)
	{
	case PLANE_X:
		dot = current_light_origin[0] - plane->dist;	break;
	case PLANE_Y:
		dot = current_light_origin[1] - plane->dist;	break;
	case PLANE_Z:
		dot = current_light_origin[2] - plane->dist;	break;
	default:
		dot = DotProduct(current_light_origin, plane->normal) - plane->dist; break;
	}

	if (dot >= 0) side = 0;
	else side = 1;

	// recurse down the children, front side first
	RecursiveDrawWorldLight(node->children[side]);

	// draw stuff
	int c = node->numsurfaces;
	if (c)
	{
		model_t *world = gEngfuncs.GetEntityByIndex(0)->model;
		msurface_t *surf = world->surfaces + node->firstsurface;

		if (dot < 0 - BACKFACE_EPSILON)
			side = SURF_PLANEBACK;
		else if (dot > BACKFACE_EPSILON)
			side = 0;
		{
			for (; c; c--, surf++)
			{
//				if (surf->visframe != framecount)
//				continue;

				// don't backface underwater surfaces, because they warp
				if (!(surf->flags & SURF_UNDERWATER) && ((dot < 0) ^ !!(surf->flags & SURF_PLANEBACK)))
					continue;		// wrong side

				LightDrawPoly(surf);
			}
		}
	}

	// recurse down the back side
	RecursiveDrawWorldLight(node->children[!side]);
}

enum {
	TC_OFF,
	TC_TEXTURE,
	TC_LIGHTMAP,
	TC_VERTEX_POSITION, // for specular and dynamic lighting
	TC_NOSTATE // uninitialized
};

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

static int texpointer[4];

void SetTexPointer(int unitnum, int tc)
{
	if (unitnum >= MAX_TU_supported)
		return;

	if (texpointer[unitnum] == tc)
		return;

	glClientActiveTextureARB(unitnum + GL_TEXTURE0_ARB);

	switch (tc)
	{
	case TC_OFF:
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		break;

	case TC_TEXTURE:
	//	glTexCoordPointer(2, GL_FLOAT, sizeof(BrushVertex), &buffer_data[0].texcoord);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		break;

	case TC_LIGHTMAP:
//		glTexCoordPointer(2, GL_FLOAT, sizeof(BrushVertex), &buffer_data[0].lightmaptexcoord);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		break;

	case TC_VERTEX_POSITION:
//		glTexCoordPointer(3, GL_FLOAT, sizeof(BrushVertex), &buffer_data[0].pos);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		break;
	}

	texpointer[unitnum] = tc;
}

static int envstates[4];

void SetTexEnv_Internal(int env)
{
	switch (env)
	{
	case ENVSTATE_OFF:
		glDisable(GL_TEXTURE_2D);
		//	glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1);
		break;

	case ENVSTATE_REPLACE:
		glEnable(GL_TEXTURE_2D);
		//	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
		//	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_REPLACE);
		//	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_TEXTURE);
		//	glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		break;

	case ENVSTATE_MUL_CONST:
		glEnable(GL_TEXTURE_2D);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_CONSTANT_ARB);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1);
		break;

	case ENVSTATE_MUL_PREV_CONST:
		glEnable(GL_TEXTURE_2D);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_PREVIOUS_ARB);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_PRIMARY_COLOR_ARB);
		glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1);
		break;

	case ENVSTATE_MUL:
		glEnable(GL_TEXTURE_2D);
		/*	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_PREVIOUS_ARB);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1);/**/
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		break;

	case ENVSTATE_MUL_X2:
		glEnable(GL_TEXTURE_2D);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_PREVIOUS_ARB);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 2);
		break;

	case ENVSTATE_ADD:
		glEnable(GL_TEXTURE_2D);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_ADD);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_PREVIOUS_ARB);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1);
		break;

	case ENVSTATE_DOT:
		glEnable(GL_TEXTURE_2D);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_DOT3_RGBA_ARB);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_PREVIOUS_ARB);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1);
		break;

	case ENVSTATE_DOT_CONST:
		glEnable(GL_TEXTURE_2D);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_DOT3_RGBA_ARB);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_CONSTANT_ARB);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1);
		break;

	case ENVSTATE_PREVCOLOR_CURALPHA:
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_REPLACE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_PREVIOUS_ARB);
		glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1);

		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, GL_REPLACE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_ARB, GL_SRC_ALPHA);
		glTexEnvi(GL_TEXTURE_ENV, GL_ALPHA_SCALE, 1);
		break;
	}
}


void SetTexEnvs(int env0, int env1 = ENVSTATE_OFF, int env2 = ENVSTATE_OFF, int env3 = ENVSTATE_OFF)
{
	if (envstates[0] != env0) {
		glActiveTextureARB(GL_TEXTURE0_ARB);
		SetTexEnv_Internal(env0);
		envstates[0] = env0;
	}
	if (envstates[1] != env1) {
		glActiveTextureARB(GL_TEXTURE1_ARB);
		SetTexEnv_Internal(env1);
		envstates[1] = env1;
	}
	if (MAX_TU_supported < 3) return;
	if (envstates[2] != env2) {
		glActiveTextureARB(GL_TEXTURE2_ARB);
		SetTexEnv_Internal(env2);
		envstates[2] = env2;
	}
	if (MAX_TU_supported < 4) return;
	if (envstates[3] != env3) {
		glActiveTextureARB(GL_TEXTURE3_ARB);
		SetTexEnv_Internal(env3);
		envstates[3] = env3;
	}
}

void CreateProjection(float *mat,float xMax, float xMin, float yMax, float yMin, float zNear, float zFar)
{
	mat[0][0] = (2.0f * zNear) / (xMax - xMin);
	mat[1][1] = (2.0f * zNear) / (yMax - yMin);
	mat[2][2] = -(zFar + zNear) / (zFar - zNear);
	mat[3][3] = mat[0][1] = mat[1][0] = mat[3][0] = mat[0][3] = mat[3][1] = mat[1][3] = 0.0f;

	mat[0][2] = 0.0f;
	mat[1][2] = 0.0f;
	mat[2][0] = (xMax + xMin) / (xMax - xMin);
	mat[2][1] = (yMax + yMin) / (yMax - yMin);
	mat[2][3] = -1.0f;
	mat[3][2] = -(2.0f * zFar * zNear) / (zFar - zNear);
}

void CreateModelView(float *mat)
{
	mat[0][0] = mat[1][1] = mat[2][2] = 0.0f;
	mat[3][0] = mat[0][3] = 0.0f;
	mat[3][1] = mat[1][3] = 0.0f;
	mat[3][2] = mat[2][3] = 0.0f;
	mat[3][3] = 1.0f;
	mat[0][1] = mat[2][0] = mat[1][2] = 0.0f;
	mat[0][2] = mat[1][0] = -1.0f;
	mat[2][1] = 1.0f;
}


void DrawFlashLight()
{
	GLfloat	genVector[4][4];

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glColor4ub(255,255,255, 255);
	glDepthFunc(GL_EQUAL);

	glBindTexture(GL_TEXTURE_2D, texid);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	for (int i = 0; i < 4; i++)
	{
		genVector[0][i] = i == 0 ? 1 : 0;
		genVector[1][i] = i == 1 ? 1 : 0;
		genVector[2][i] = i == 2 ? 1 : 0;
		genVector[3][i] = i == 3 ? 1 : 0;
	}

/*	GL_TexGen(GL_S, GL_OBJECT_LINEAR);
	GL_TexGen(GL_T, GL_OBJECT_LINEAR);
	GL_TexGen(GL_R, GL_OBJECT_LINEAR);
	GL_TexGen(GL_Q, GL_OBJECT_LINEAR);
	*/

	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_PLANE); glTexGenfv(GL_S, GL_OBJECT_PLANE, genVector[0]);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_PLANE); glTexGenfv(GL_T, GL_OBJECT_PLANE, genVector[1]);
	glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_OBJECT_PLANE); glTexGenfv(GL_R, GL_OBJECT_PLANE, genVector[2]);
	glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_OBJECT_PLANE); glTexGenfv(GL_Q, GL_OBJECT_PLANE, genVector[3]);

	glTexGenfv(GL_S, GL_OBJECT_PLANE, genVector[0]);
	glTexGenfv(GL_T, GL_OBJECT_PLANE, genVector[1]);
	glTexGenfv(GL_R, GL_OBJECT_PLANE, genVector[2]);
	glTexGenfv(GL_Q, GL_OBJECT_PLANE, genVector[3]);

	// update the frustum only if needs
	GLdouble	xMin, xMax, yMin, yMax, zNear, zFar;
	float fov = 90;
	GLfloat	fov_x, fov_y;

	zNear = 0.1f;
	zFar = radius * 1.5f; // distance

	xMax = yMax = zNear * tan( fov * M_PI / 360.0 );
	xMin = yMin = -yMax;
	fov_x = fov_y = fov;

	fov = zNear * tan( fov * M_PI / 360.0 );

	float *projectionMatrix = new float[16];
	float *modelviewMatrix = new float[16];

	CreateProjection(projectionMatrix,xMax, xMin, yMax, yMin, zNear, zFar);
	CreateModelView(modelviewMatrix);


	delete[] projectionMatrix;
	delete[] modelviewMatrix;

	/*
	pl->modelviewMatrix.ConcatRotate( -pl->angles.z, 1, 0, 0 );
	pl->modelviewMatrix.ConcatRotate( -pl->angles.x, 0, 1, 0 );
	pl->modelviewMatrix.ConcatRotate( -pl->angles.y, 0, 0, 1 );
	pl->modelviewMatrix.ConcatTranslate( -pl->origin.x, -pl->origin.y, -pl->origin.z );

	matrix4x4 projectionView, m1;
	m1.CreateTranslate( 0.5f, 0.5f, 0.5f );

	projectionView = pl->projectionMatrix.Concat( pl->modelviewMatrix );
	m1.ConcatScale( 0.5f, -0.5f, 0.5f );

	pl->textureMatrix = m1.Concat( projectionView ); // only for world, sprites and studiomodels

	Vector vforward, vright, vup;
	AngleVectors( pl->angles, vforward, vright, vup );

	Vector farPoint = pl->origin + vforward * zFar;

	// rotate vforward right by FOV_X/2 degrees
	RotatePointAroundVector( pl->frustum[0].normal, vup, vforward, -(90 - fov_x / 2));
	// rotate vforward left by FOV_X/2 degrees
	RotatePointAroundVector( pl->frustum[1].normal, vup, vforward, 90 - fov_x / 2 );
	// rotate vforward up by FOV_Y/2 degrees
	RotatePointAroundVector( pl->frustum[2].normal, vright, vforward, 90 - fov_y / 2 );
	// rotate vforward down by FOV_Y/2 degrees
	RotatePointAroundVector( pl->frustum[3].normal, vright, vforward, -(90 - fov_y / 2));*/
	/*
	float dest[16];
	glMatrixMode(GL_TEXTURE);
	glLoadMatrixf(dest);
	*/
	/*			R_DrawLightChains();
			R_EndDrawProjection();*/
}