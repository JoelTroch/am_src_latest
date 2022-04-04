#pragma once
class CSSAOEffect
{
public:
	void InitSSAO();
	void DrawEffect();
	void blurThis();

	unsigned int screentex;
	unsigned int depthtex;
	unsigned int bluredtex; 
	unsigned int ssaotex;

	bool inited;
};

extern CSSAOEffect gSSAO;