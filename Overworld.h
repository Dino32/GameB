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

	char* AreaName;

	int16_t ID;

} PORTAL;

typedef struct GAMEAREA
{
	char* Name;

	RECT Area;

	GAMESOUND Music;

} GAMEAREA;

PORTAL gPortal001;

PORTAL gPortal002;

PORTAL gPortals[2];

BOOL PortalIsUsed;

BOOL AnimationAfterPortal;

int16_t PortalId;

void DrawOverworld(void);

void PPI_Overworld(void);

void PortalHandler(void);