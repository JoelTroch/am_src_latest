class CBlurEffect
{
public:
	void InitBlur();
	void DrawEffect();
	void ResetBlur();
	void InterpolateIt();

	void SetNew(float _str,float _a,float _f,int _fmode = false);

	int fademode;

	//saved
	float strength,alpha,fadetime;
	//ideal
	float istrength,ialpha;
	//current
	float cstrength,calpha;

	float lastchanged;//The time when we changed blur

	unsigned int screentex,blurtex;
	bool inited;

	unsigned int fp_blur;
};

class CGlowEffect
{
public:
	void InitGlow();
	void DrawEffect();
	void ResetGlow();
	void InterpolateIt();

	void SetNew(float _str,float _a,float _f,int _fmode);

	int fademode;

	//saved
	float strength,alpha,fadetime;
	//ideal
	float istrength,ialpha;
	//current
	float cstrength,calpha;

	float lastchanged;//The time when we change blur

	unsigned int screentex,glowtex;
	bool inited;

	unsigned int fp_glow;
};

extern CBlurEffect gBlur;
extern CGlowEffect gGlow;
