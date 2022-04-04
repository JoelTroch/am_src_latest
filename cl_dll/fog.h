class CFogEffect
{
public:
	void InitFog();
	void DrawEffect();
	void ResetFog();

	float fogNear, fogFar;
	unsigned int screentex, depthtex;
	bool inited;

	unsigned int fp_fog;
	unsigned int fp_foghi;
};

extern CFogEffect gFog;