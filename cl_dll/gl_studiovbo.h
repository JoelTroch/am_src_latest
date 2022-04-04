#ifndef STUDIO_VBO
#define STUDIO_VBO

extern GLvoid *TEXCOORDOFFSET;
extern GLvoid *NORMALOFFSET;
extern GLvoid *BONEIDOFFSET;
extern GLvoid *TANGENTOFFSET;
extern GLvoid *BINORMALOFFSET;

class ExtStudioTextureData;
typedef struct studioVbo
{
	mstudiomesh_t *mesh;
	GLuint vboId;
	GLuint iboId;

	mstudiotexture_t *texture;
	ExtStudioTextureData *extTextData;
	short *ptricmds;
	Vector *g_pxformverts;
	unsigned int indexCount;
	byte *pvertbones;
	Vector *pstudionorms;

	float originCache, oldOriginCache;
	lighting_ext oldlight;
	bool prop;
	int entindex;
	byte *pnormbones;
	int triindex;
	int numtris;

}studiovbo_t;

typedef struct studioVertex
{
	Vector vertex;
	Vector2D texcoord;
	float boneid;
	Vector normal;
	float tangent[4];
	Vector binormal;
	int index;
	int normalidx;
}studiovertex_t;


extern GLvoid *TEXCOORDOFFSETPROP;
extern GLvoid *NORMALOFFSETPROP;
extern GLvoid *TANGENTOFFSETPROP;
extern GLvoid *LIGHTINGOFFSETPROP;

typedef struct studioVertexProp
{
	Vector vertex;
	Vector2D texcoord;
	Vector normal;
	float tangent[4];
	Vector lighting;
}studiovertexprop_t;

void initStudioVBO();
studiovbo_t *getVbo(cl_entity_t *ent,mstudiomesh_t *mesh, short *ptricmds = NULL, mstudiotexture_t *texture = NULL, Vector *g_pxformverts = NULL, byte *pvertbones = NULL, Vector *pstudionorms = NULL,byte *pnormbones = NULL);
studiovbo_t *getVboProp(cl_entity_t *ent, mstudiomesh_t *mesh, short *ptricmds = NULL, mstudiotexture_t *texture = NULL, Vector *g_pxformverts = NULL, byte *pvertbones = NULL, Vector *pstudionorms = NULL, byte *pnormbones = NULL);
studiovbo_t *findVbo(mstudiomesh_t *mesh);
studiovbo_t *findVboProp(mstudiomesh_t *mesh,cl_entity_t *ent);

#endif