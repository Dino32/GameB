#pragma once

typedef struct PORTAL
{
	// Where is this portal located in world coordinates
	UPOINT WorldPos;

	//  Where should portal take the player in world coordinates
	UPOINT WorldDest;

	// Where will player be teleported
	UPOINT ScreenPos;

	// where the camera will be moved
	UPOINT CameraPos;

	// in which area the player will be moved
	RECT Area;

} PORTAL;

PORTAL gPortal001;

PORTAL gPortal002;

PORTAL gPortals[2];

RECT gCurrentArea;

RECT gOverwrldArea;

RECT gDungeon1Area;

void DrawOverworld(void);

void PPI_Overworld(void);

void PortalHandler(void);