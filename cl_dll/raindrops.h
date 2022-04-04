#ifndef __RAINDROPS__
#define __RAINDROPS__

extern cvar_t *r_rainmode;

class CRainDrop
{
public:
	CRainDrop(int tex,int lowTex, int _x, int _y, float _lifeTime)
	{
		texture = tex;
		lowtexture = lowTex;

		x = _x; y = _y;

		if (lifeTime == 0)
			lifeTime = 0.1f;

		lifeTime = _lifeTime;
		dieTime = gEngfuncs.GetClientTime() + _lifeTime;
	};

	int Draw(float flTime);
	int DrawLowRaindrop(float fltime);
	int DrawHighRaindrop(float fltime);

	unsigned int texture;
	unsigned int lowtexture;
	int x, y;
	float dieTime;
	float lifeTime;

	CRainDrop *next;
};

class CRainDropsEffect
{
public:
	CRainDropsEffect()
	{
		objectsHead = NULL;
	}
	~CRainDropsEffect()
	{
		Reset();
	}
	void Reset()
	{
		nextAdd = 0.0;
		while (objectsHead)
		{
			CRainDrop *next;
			next = objectsHead->next;
			delete objectsHead;
			objectsHead = next;
		}
	}
	void AddObject(CRainDrop *obj)
	{
		obj->next = objectsHead;
		objectsHead = obj;
	}
	void ProcessRainDrops();
	CRainDrop *objectsHead;
	void InitRain();
	void ResetRain();
	void CaptureScreen();
	unsigned int screentex;
	unsigned int smalltex;

	bool inited;
	unsigned int fp_raindrop;
	unsigned int fp_rain;
	unsigned int vp_raindrop;
	void DrawRaindrops();
	float nextAdd;

	unsigned int rainScreenTexture;
	unsigned int rainScreenTextureLow;
	int rainTextures[25];
	int rainTexturesLow[25];

	unsigned int rainSplash[2];
};

#define SUBDIVIDE_SIZE	64
#endif

extern CRainDropsEffect gRaindrops;