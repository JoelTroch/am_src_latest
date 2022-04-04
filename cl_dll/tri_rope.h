#define MAX_ROPES		128

//for Bézier calcs
#define	B1(t)			(t*t)
#define	B2(t)			(2*t*(1-t))
#define	B3(t)			((1-t)*(1-t))

#define START			0
#define MID_POINT		1
#define END				2
#define MAX_SEGMENTS	64

struct sRope
{
	vec3_t vSplineColor[MAX_SEGMENTS];
	vec3_t mySpline[MAX_SEGMENTS];
	vec3_t myPoints[3];

	int	mySegments;
	int	myLenght;
	float	myScale;

	int	r;
	int	g;
	int	b;
	  
	char mySpriteFile[512];

	int idx;
	bool bDraw;
	bool bRandomCheck;
};

class GLRopeRender
{
public:
	GLRopeRender();
	~GLRopeRender();

	bool IsRopeVisible(sRope *rope);
	void DrawRopes( float fltime);
	void ResetRopes();

	void CreateRope( char *datafile, vec3_t start_source,vec3_t end_source,int idx );

	void DrawBeam(vec3_t start,vec3_t end,float width,char *Sprite, bool bCheckLight, int r, int g, int b, vec3_t color );
	
	sRope hRope[MAX_ROPES];

private:
	int rope_id;
};

extern GLRopeRender gRopeRender;

