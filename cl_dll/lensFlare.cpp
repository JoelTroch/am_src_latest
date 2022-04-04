#include "hud.h"
#include "cl_util.h"
#include "glmanager.h"
#include "../common/com_model.h"

#include "gl_texloader.h"
#include "lensFlare.h"
#include "triangleapi.h"
#include "glStuff.h"

#include "pm_defs.h"
CLensFlare gLensFlares;

float convertYaw(float yaw){
	if (yaw > 180)
		yaw -= 360;

	if (yaw < -180.0)
		yaw += 360.0;

	return yaw;
}

void CLensFlare::InitLensFlares(){
	sunPosition = vec3_t(0, 0, 0);
	sunPitch = 0;
	enabled = 0;

	int i = 0;
	textures[i++] = LoadTextureWide("gfx/lens/flare2.tga");
	textures[i++] = LoadTextureWide("gfx/lens/flare1.tga");
	textures[i++] = LoadTextureWide("gfx/lens/glow3.tga");
	textures[i++] = LoadTextureWide("gfx/lens/glow2.tga");
	textures[i++] = LoadTextureWide("gfx/lens/glow1.tga");
	textures[i++] = LoadTextureWide("gfx/lens/flare1.tga");
	textures[i++] = LoadTextureWide("gfx/lens/lens3.tga");
	textures[i++] = LoadTextureWide("gfx/lens/lens1.tga");
}

void CLensFlare::SetSunParams(vec3_t pos, float pitch){
	enabled = true;

	for (int i = 0; i < 8;i++)
	if (textures[i] == null)
		enabled = false;
	
	sunPosition = pos;
	sunPitch = pitch;
}

void CLensFlare::DrawEffect(){
	if (!enabled) return;

	vec3_t viewAngle;
	gEngfuncs.GetViewAngles(viewAngle);
	float pitch = -viewAngle.x;
	float yaw = viewAngle.y;
	yaw = convertYaw(yaw);

	cl_entity_t *player = gEngfuncs.GetLocalPlayer();
	if (!player) return;
	pmtrace_t tr = *(gEngfuncs.PM_TraceLine(player->origin, sunPosition, PM_STUDIO_IGNORE, 2, -1));
	if (tr.fraction < 1.0)
		return;

	vec3_t sunAnglesToPlayer;
	vec3_t dir = sunPosition - player->origin;
	VectorNormalize(dir);
	VectorAngles(dir, sunAnglesToPlayer);
	sunAnglesToPlayer.y = convertYaw(sunAnglesToPlayer.y);
	sunAnglesToPlayer.x = -sunAnglesToPlayer.x;

	vec3_t screen;
	if (gEngfuncs.pTriAPI->WorldToScreen(sunPosition, screen))
		return;

	screen[0] = XPROJECT(screen[0]);
	screen[1] = YPROJECT(screen[1]);

	pitch += 20.0;
	float scale = (float)ScreenWidth / 1920.0f;
	float spriteScale = 2.5;
	float pitchDiff = (sunAnglesToPlayer.x - pitch + 4.0)*2.0*scale;
	float angleDiff = (yaw - sunAnglesToPlayer.y+4.0)*4.0*scale;
	float alpha = 1.0 - (angleDiff + pitchDiff)/80.0;

	for (int i = 0; i < 8; i++){
		int w, h;
		w = textures[i]->iWidth; h = textures[i]->iHeight;
		alpha = 1.0 - fabs((angleDiff) / 120.0);
		alpha *= 1.0 - (float)(i+1.0)/14.0;
		alpha *= 0.8;

		glColor4f(alpha, alpha, alpha, 1.0);
		gGlStuff.DrawQuadWH(screen[0] - w / 2 * spriteScale - i*angleDiff, screen[1] - h / 2 * spriteScale - i*pitchDiff, textures[i]->iIndex, w*spriteScale, h*spriteScale, true);

		angleDiff *= 1.05;
	}
}