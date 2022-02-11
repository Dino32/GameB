#pragma once


typedef struct MENUITEM
{
	char* Name;

	int16_t x;

	int16_t y;

	BOOL Enabled;

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

void MenuItem_ExitYesNo_Yes(void);

void MenuItem_ExitYesNo_No(void);

void MenuItem_OptionsScreen_SFXVolume(void);

void MenuItem_OptionsScreen_MusicVolume(void);

void MenuItem_OptionsScreen_ScreenSize(void);

void MenuItem_OptionsScreen_Back(void);

void MenuItem_CharacterNaming_Add(void);

void MenuItem_CharacterNaming_Back(void);

void MenuItem_CharacterNaming_OK(void);


// Title Screen

MENUITEM gMI_ResumeGame = { "Resume", (GAME_RES_WIDTH / 2 ) - ((6 *6) /2), 100, FALSE,  MenuItem_TitleScreen_Resume};

MENUITEM gMI_StartNewGame = { "Start New Game", (GAME_RES_WIDTH / 2) - ((14 * 6) / 2), 120, TRUE,  MenuItem_TitleScree_StartNew};

MENUITEM gMI_Options = { "Options", (GAME_RES_WIDTH / 2) - ((7 * 6) / 2), 140, TRUE,  MenuItem_TitleScree_Options };

MENUITEM gMI_Exit = { "Exit", (GAME_RES_WIDTH/2) - ((5*6) / 2), 160, TRUE, MenuItem_TitleScree_Exit };

MENUITEM* gMI_TitleScreenItems[] = { &gMI_ResumeGame, &gMI_StartNewGame, &gMI_Options, &gMI_Exit };

MENU gMenu_TitleScreen = { "Title Screen Menu", 1, _countof(gMI_TitleScreenItems), gMI_TitleScreenItems };

//

// Exit Yes or No Screen

MENUITEM gMI_ExitYesNo_Yes = { "Yes", (GAME_RES_WIDTH / 2) - ((3 * 6) / 2), 100, TRUE, MenuItem_ExitYesNo_Yes};

MENUITEM gMI_ExitYesNo_No = { "No", (GAME_RES_WIDTH / 2) - ((2 * 6) / 2), 120, TRUE, MenuItem_ExitYesNo_No};

MENUITEM* gMI_ExitYesNoItems[] = { &gMI_ExitYesNo_Yes, &gMI_ExitYesNo_No };

MENU gMenu_ExitYesNo = { "Are you sure you want to exit?", 1, _countof(gMI_ExitYesNoItems), gMI_ExitYesNoItems };
//


// Options Screen

MENUITEM gMI_OptionsScreen_SFXVolume = { "SFX Volume:", (GAME_RES_WIDTH/2) - ((11 * 6) / 2), 100,  TRUE, MenuItem_OptionsScreen_SFXVolume};

MENUITEM gMI_OptionsScreen_MusicVolume = { "Music Volume:", (GAME_RES_WIDTH / 2) - ((13*6)/2), 120, TRUE, MenuItem_OptionsScreen_MusicVolume};
 
MENUITEM gMI_OptionsScreen_ScreenSize = { "Screen Size:", (GAME_RES_WIDTH/2)- ((12*6)/2), 140, TRUE, MenuItem_OptionsScreen_ScreenSize};

MENUITEM gMI_OptionsScreen_Back = { "Back", (GAME_RES_WIDTH/2) - (4*6)/2, 160, TRUE, MenuItem_OptionsScreen_Back};

MENUITEM* gMI_OptionsScreenItems[] = { &gMI_OptionsScreen_SFXVolume, &gMI_OptionsScreen_MusicVolume, &gMI_OptionsScreen_ScreenSize, &gMI_OptionsScreen_Back };

MENU gMenu_OptionsScreen = { "Options", 0, _countof(gMI_OptionsScreenItems), gMI_OptionsScreenItems};

//

// Character Naming Menu

MENUITEM gMI_CharacterNamingScreen_A = { "A", 105, 100, TRUE, MenuItem_CharacterNaming_Add};

MENUITEM gMI_CharacterNamingScreen_B = { "B", 119, 100, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNamingScreen_C = { "C", 133, 100, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNamingScreen_D = { "D", 147, 100, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNamingScreen_E = { "E", 161, 100, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNamingScreen_F = { "F", 175, 100, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNamingScreen_G = { "G", 189, 100, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNamingScreen_H = { "H", 203, 100, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNamingScreen_I = { "I", 217, 100, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNamingScreen_J = { "J", 231, 100, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNamingScreen_K = { "K", 245, 100, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNamingScreen_L = { "L", 259, 100, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNamingScreen_M = { "M", 273, 100, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNamingScreen_N = { "N", 105, 110, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNamingScreen_O = { "O", 119, 110, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNamingScreen_P = { "P", 133, 110, TRUE, MenuItem_CharacterNaming_Add};

MENUITEM gMI_CharacterNamingScreen_Q = { "Q", 147, 110, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNamingScreen_R = { "R", 161, 110, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNamingScreen_S = { "S", 175, 110, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNamingScreen_T = { "T", 189, 110, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNamingScreen_U = { "U", 203, 110, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNamingScreen_V = { "V", 217, 110, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNamingScreen_W = { "W", 231, 110, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNamingScreen_X = { "X", 245, 110, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNamingScreen_Y = { "Y", 259, 110, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNamingScreen_Z = { "Z", 273, 110, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNamingScreen_a = { "a", 105, 120, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNamingScreen_b = { "b", 119, 120, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNamingScreen_c = { "c", 133, 120, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNamingScreen_d = { "d", 147, 120, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNamingScreen_e = { "e", 161, 120, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNamingScreen_f = { "f", 175, 120, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNamingScreen_g = { "g", 189, 120, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNamingScreen_h = { "h", 203, 120, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNamingScreen_i = { "i", 217, 120, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNamingScreen_j = { "j", 231, 120, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNamingScreen_k = { "k", 245, 120, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNamingScreen_l = { "l", 259, 120, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNamingScreen_m = { "m", 273, 120, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNamingScreen_n = { "n", 105, 130, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNamingScreen_o = { "o", 119, 130, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNamingScreen_p = { "p", 133, 130, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNamingScreen_q = { "q", 147, 130, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNamingScreen_r = { "r", 161, 130, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNamingScreen_s = { "s", 175, 130, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNamingScreen_t = { "t", 189, 130, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNamingScreen_u = { "u", 203, 130, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNamingScreen_v = { "v", 217, 130, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNamingScreen_w = { "w", 231, 130, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNamingScreen_x = { "x", 245, 130, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNamingScreen_y = { "y", 259, 130, TRUE, MenuItem_CharacterNaming_Add };

MENUITEM gMI_CharacterNamingScreen_z = { "z", 273, 130, TRUE, MenuItem_CharacterNaming_Add };


MENUITEM gMI_CharacterNamingScreen_Back = { "Back", 105, 140, TRUE, MenuItem_CharacterNaming_Back };

MENUITEM gMI_CharacterNamingScreen_OK = { "OK", 265, 140, TRUE, MenuItem_CharacterNaming_OK};

MENUITEM* gMI_CharacterNamingItems[] = { &gMI_CharacterNamingScreen_A, &gMI_CharacterNamingScreen_B, &gMI_CharacterNamingScreen_C, &gMI_CharacterNamingScreen_D, &gMI_CharacterNamingScreen_E, &gMI_CharacterNamingScreen_F, &gMI_CharacterNamingScreen_G, &gMI_CharacterNamingScreen_H, &gMI_CharacterNamingScreen_I, &gMI_CharacterNamingScreen_J, &gMI_CharacterNamingScreen_K, &gMI_CharacterNamingScreen_L, &gMI_CharacterNamingScreen_M, &gMI_CharacterNamingScreen_N, &gMI_CharacterNamingScreen_O, &gMI_CharacterNamingScreen_P, &gMI_CharacterNamingScreen_Q, &gMI_CharacterNamingScreen_R, &gMI_CharacterNamingScreen_S, &gMI_CharacterNamingScreen_T, &gMI_CharacterNamingScreen_U, &gMI_CharacterNamingScreen_V, &gMI_CharacterNamingScreen_W, &gMI_CharacterNamingScreen_X, &gMI_CharacterNamingScreen_Y, &gMI_CharacterNamingScreen_Z, 
										&gMI_CharacterNamingScreen_a, &gMI_CharacterNamingScreen_b, &gMI_CharacterNamingScreen_c, &gMI_CharacterNamingScreen_d, &gMI_CharacterNamingScreen_e, &gMI_CharacterNamingScreen_f, &gMI_CharacterNamingScreen_g, &gMI_CharacterNamingScreen_h, &gMI_CharacterNamingScreen_i, &gMI_CharacterNamingScreen_j, &gMI_CharacterNamingScreen_k, &gMI_CharacterNamingScreen_l, &gMI_CharacterNamingScreen_m, &gMI_CharacterNamingScreen_n, &gMI_CharacterNamingScreen_o, &gMI_CharacterNamingScreen_p, &gMI_CharacterNamingScreen_q, &gMI_CharacterNamingScreen_r, &gMI_CharacterNamingScreen_s, &gMI_CharacterNamingScreen_t, &gMI_CharacterNamingScreen_u, &gMI_CharacterNamingScreen_v, &gMI_CharacterNamingScreen_w, &gMI_CharacterNamingScreen_x, &gMI_CharacterNamingScreen_y, &gMI_CharacterNamingScreen_z, 
										&gMI_CharacterNamingScreen_Back, & gMI_CharacterNamingScreen_OK};

MENU gMenu_CharacterNaming = { "What is your name, hero?", 0, _countof(gMI_CharacterNamingItems), gMI_CharacterNamingItems};

//