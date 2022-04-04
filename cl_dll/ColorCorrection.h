#pragma once
class CColorCorrection
{
public:
	void InitColorCorrection();
	void DrawEffect();
	void ResetColorCorrection();
	void InterpolateIt();

	void SetNew(float _r, float _g, float _b, float _s,float _ftime);

	//saved
	float r,g,b,s;
	float ir, ig, ib, is;
	float fadetime;

	float lastchanged;//The time when we changed values

	unsigned int screentex;
	bool inited;
};

extern CColorCorrection gColorCorrection;