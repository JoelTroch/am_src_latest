#pragma once
class glStuff
{
public:
	glStuff();
	~glStuff();

	void DrawQuadRAW(int x, int y, int ex, int ey);
	void DrawQuadRAW(int x, int y, int ex, int ey, float tx, float ty, float tex, float tey);
	void DrawQuad(int x, int y, int texture, int ex, int ey, bool a = false);
	void DrawQuadWH(int x, int y, int texture, int w, int h, bool a = false);
	void DrawQuadS(float a, float b, int c, float d, float e, bool f);
	void DrawScreenOV(void);
};

extern glStuff gGlStuff;
