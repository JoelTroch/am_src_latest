#define MAX_WATER_SURFACES 256

class CWaterEffect
{
public:
	void InitWater();
	void DrawWaterPoly(msurface_t *surf);
	void ResetWater();
	void CaptureScreen();
	void EmitWaterPolys( msurface_t *s );
	unsigned int screentex;
	
	bool inited;
	unsigned int fp_cubemap_water;
	unsigned int fp_cubemap_water_noblur;
	unsigned int vp_cubemap_water;
	void DrawWater();
	void AddToChain(msurface_t *surf);

	msurface_t *surfaces[MAX_WATER_SURFACES];
	int numSurfaces;

	int waterTexture;
};

#define SUBDIVIDE_SIZE	64

extern CWaterEffect gWater;