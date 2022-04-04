#ifndef CL_DECALS_H
#define CL_DECALS_H
#include <vector>

#define MAX_DECALTEXTURES	128
#define MAX_CUSTOMDECALS	1024
#define MAX_STATICDECALS	256
#define MAX_GROUPENTRIES	64

class DecalGroupEntry
{
public:
	int gl_texid;
	int xsize, ysize;
	float overlay;
};

class DecalGroup
{
public:
	DecalGroup(const char *_name, int numelems, DecalGroupEntry *source);
	~DecalGroup();

	DecalGroupEntry *GetEntry(int num);
	const DecalGroupEntry *GetRandomDecal();

	char name[16];

	DecalGroup *pnext;
	DecalGroupEntry *pEntryArray;
	int size;
};


typedef struct decaltexture_s {
	char name[16];
	int gl_texid;
} decaltexture;

// FIX
typedef struct customdecalvert_s
{
	vec3_t position;
	float texcoord[2];
} customdecalvert_t;
// END

typedef struct customdecal_s
{
	msurface_t *surface;
	cl_entity_t *entity; // Fix
	std::vector<customdecalvert_t> verts; // Fix

	vec3_t point;
	vec3_t normal;
	const DecalGroupEntry *texinfo;
} customdecal;


struct decal_msg_cache
{
	vec3_t	pos;
	vec3_t	normal;
	char	name[16];
	int		persistent;
};

struct decalvertinfo_t
{
	vec3_t position;
	byte boneindex;
};

struct decalvert_t
{
	int vertindex;
	float texcoord[2];
};

struct decalpoly_t
{
	decalvert_t *verts;
	int numverts;
};

struct studiodecal_t
{
	int entindex;

	decalpoly_t *polys;
	int numpolys;

	decalvertinfo_t *verts;
	int numverts;

	const DecalGroupEntry *texture;

	int totaldecals;
	studiodecal_t *next; // linked list on this entity
};

struct studiovert_t
{
	int vertindex;
	int normindex;
	int texcoord[2];
	byte boneindex;
};

struct studiotri_t
{
	studiovert_t verts[3];
};

DecalGroup* FindGroup(const char *_name);
int ClipPolygonByPlane(const vec3_t *arrIn, int numpoints, vec3_t normal, vec3_t planepoint, vec3_t *arrOut);
void FindIntersectionPoint(const vec3_t &p1, const vec3_t &p2, const vec3_t &normal, const vec3_t &planepoint, vec3_t &newpoint);
void GetUpRight(vec3_t forward, vec3_t &up, vec3_t &right);

#define	MAX_MODEL_DECALS	16

#endif