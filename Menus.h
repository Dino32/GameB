#pragma once


typedef struct MENUITEM
{
	char* Name;

	int16_t x;

	int16_t y;

	void(*Action)(void);

} MENUITEM;

typedef struct MENU
{
	char* Name;

	uint8_t SelectedItem;

	uint8_t ItemCount;

	MENUITEM** Items;

} MENU;

void MenuItem_TitleScreen_Resume(void);

void MenuItem_TitleScree_StartNew(void);

void MenuItem_TitleScree_Options(void);

void MenuItem_TitleScree_Exit(void);

// Title Screen

MENUITEM gMI_ResumeGame = { "Resume", (GAME_RES_WIDTH / 2 ) - ((6 *6) /2), 100, MenuItem_TitleScreen_Resume};

MENUITEM gMI_StartNewGame = { "Start New Game", (GAME_RES_WIDTH / 2) - ((14 * 6) / 2), 120, MenuItem_TitleScree_StartNew};

MENUITEM gMI_Options = { "Options", (GAME_RES_WIDTH / 2) - ((7 * 6) / 2), 140, MenuItem_TitleScree_Options };

MENUITEM gMI_Exit = { "Exit", (GAME_RES_WIDTH/2) - ((5*6) / 2), 160, MenuItem_TitleScree_Exit };

MENUITEM* gMI_TitleScreenItems[] = { &gMI_ResumeGame, &gMI_StartNewGame, &gMI_Options, &gMI_Exit };

MENU gMenu_TitleScreen = { "Title Screen Menu", 0, _countof(gMI_TitleScreenItems), gMI_TitleScreenItems };

//