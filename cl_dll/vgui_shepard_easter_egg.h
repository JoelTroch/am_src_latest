/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

//
// vgui_shepard_easter_egg.h
//
// My little easter egg for ARRANGEMENT ^^
// When dead, making the Konami Code (Up, Up, Down, Down, Left, Right, Left, Right, B, A)
// will play a Metal Gear Solid like death scene I made ^^
//

#ifndef _SHEPARD_EASTER_EGG_H
#define _SHEPARD_EASTER_EGG_H

using namespace vgui;

class CShepardEasterEgg
{
public:
	CShepardEasterEgg();
	int KeyInput( int down, int keynum, const char *pszCurrentBinding );

private:
	void Reset();
	int m_iState;
};

#endif // _SHEPARD_EASTER_EGG_H
