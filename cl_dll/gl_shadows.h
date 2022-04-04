#ifndef __SHADOWS_H__
#define __SHADOWS_H__

extern DynamicLight *current_light;
extern int shadowMaps[MAX_DLIGHTS];

void InitShadows();
void SetupProjectionShadow();

void SetupPCF();
void DisablePCF();
void FixVectorForSpotlight(vec3_t &vec);
void R_DrawBrushModelShadow(cl_entity_t *e, DynamicLight *light);
void R_RotateForEntity(cl_entity_t *e);
void RecursiveDrawWorldShadow(mnode_t *node);
void DrawLightShadows();

extern bool shadowPCFsupport;
extern unsigned int m_iShadowFragmentID;
#endif