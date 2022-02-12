#include "Main.h"

#include "CharacterNaming.h"

// Character Naming Menu

MENUITEM gMI_CharacterNamingScreen_A = { "A", 105, 100, TRUE, MenuItem_CharacterNaming_Add };

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

MENUITEM gMI_CharacterNamingScreen_P = { "P", 133, 110, TRUE, MenuItem_CharacterNaming_Add };

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

MENUITEM gMI_CharacterNamingScreen_OK = { "OK", 265, 140, TRUE, MenuItem_CharacterNaming_OK };

MENUITEM* gMI_CharacterNamingItems[] = { &gMI_CharacterNamingScreen_A, &gMI_CharacterNamingScreen_B, &gMI_CharacterNamingScreen_C, &gMI_CharacterNamingScreen_D, &gMI_CharacterNamingScreen_E, &gMI_CharacterNamingScreen_F, &gMI_CharacterNamingScreen_G, &gMI_CharacterNamingScreen_H, &gMI_CharacterNamingScreen_I, &gMI_CharacterNamingScreen_J, &gMI_CharacterNamingScreen_K, &gMI_CharacterNamingScreen_L, &gMI_CharacterNamingScreen_M, &gMI_CharacterNamingScreen_N, &gMI_CharacterNamingScreen_O, &gMI_CharacterNamingScreen_P, &gMI_CharacterNamingScreen_Q, &gMI_CharacterNamingScreen_R, &gMI_CharacterNamingScreen_S, &gMI_CharacterNamingScreen_T, &gMI_CharacterNamingScreen_U, &gMI_CharacterNamingScreen_V, &gMI_CharacterNamingScreen_W, &gMI_CharacterNamingScreen_X, &gMI_CharacterNamingScreen_Y, &gMI_CharacterNamingScreen_Z,
										&gMI_CharacterNamingScreen_a, &gMI_CharacterNamingScreen_b, &gMI_CharacterNamingScreen_c, &gMI_CharacterNamingScreen_d, &gMI_CharacterNamingScreen_e, &gMI_CharacterNamingScreen_f, &gMI_CharacterNamingScreen_g, &gMI_CharacterNamingScreen_h, &gMI_CharacterNamingScreen_i, &gMI_CharacterNamingScreen_j, &gMI_CharacterNamingScreen_k, &gMI_CharacterNamingScreen_l, &gMI_CharacterNamingScreen_m, &gMI_CharacterNamingScreen_n, &gMI_CharacterNamingScreen_o, &gMI_CharacterNamingScreen_p, &gMI_CharacterNamingScreen_q, &gMI_CharacterNamingScreen_r, &gMI_CharacterNamingScreen_s, &gMI_CharacterNamingScreen_t, &gMI_CharacterNamingScreen_u, &gMI_CharacterNamingScreen_v, &gMI_CharacterNamingScreen_w, &gMI_CharacterNamingScreen_x, &gMI_CharacterNamingScreen_y, &gMI_CharacterNamingScreen_z,
										&gMI_CharacterNamingScreen_Back, &gMI_CharacterNamingScreen_OK };

MENU gMenu_CharacterNaming = { "What is your name, hero?", 0, _countof(gMI_CharacterNamingItems), gMI_CharacterNamingItems };

//


void DrawCharacterNaming(void)
{
    static uint64_t LocalFrameCounter;

    static uint64_t LastFrameSeen;

    static PIXEL32 TextColor = { 0x00, 0x00, 0x00, 0x00 };

    if (gPerformanceData.TotalFramesRednered > (LastFrameSeen + 1))
    {
        LocalFrameCounter = 0;

        TextColor.Red = 0x00;

        TextColor.Blue = 0x00;

        TextColor.Green = 0x00;

        gMenu_CharacterNaming.SelectedItem = 0;

    }

    memset(gBackBuffer.Memory, 0, GAME_DRAWING_AREA_MEMORY_SIZE);

    if (LocalFrameCounter >= 20)
    {
        if (LocalFrameCounter % 10 == 0 && LocalFrameCounter < 80)
        {
            TextColor.Red += 32;

            TextColor.Blue += 32;

            TextColor.Green += 32;
        }

        if (LocalFrameCounter == 70)
        {
            TextColor.Red = 255;

            TextColor.Blue = 255;

            TextColor.Green = 255;
        }
    }

    BlitStringToBuffer(gMenu_CharacterNaming.Name, &g6x7Font, TextColor, (GAME_RES_WIDTH / 2) - (strlen(gMenu_CharacterNaming.Name) * 6) / 2, 30);

    if (LocalFrameCounter >= 50)
    {
        Blit32BppBitmapToBuffer(&gPlayer.Sprite[SUIT_0][FACING_DOWN_0], 152, 62);
    }

    for (uint8_t Counter = 0; Counter < 8; Counter++)
    {
        if (gPlayer.Name[Counter] == '\0')
        {
            BlitStringToBuffer("_", &g6x7Font, TextColor, 170 + Counter * 6, 65);
        }
        else
        {
            BlitStringToBuffer(&gPlayer.Name[Counter], &g6x7Font, TextColor, 170 + Counter * 6, 65);
        }
    }

    for (uint8_t Counter = 0; Counter < gMenu_CharacterNaming.ItemCount; Counter++)
    {
        BlitStringToBuffer(gMenu_CharacterNaming.Items[Counter]->Name, &g6x7Font, TextColor, gMenu_CharacterNaming.Items[Counter]->x, gMenu_CharacterNaming.Items[Counter]->y);
    }

    BlitStringToBuffer("\xbb", &g6x7Font, TextColor, gMenu_CharacterNaming.Items[gMenu_CharacterNaming.SelectedItem]->x - 6, gMenu_CharacterNaming.Items[gMenu_CharacterNaming.SelectedItem]->y);

    LocalFrameCounter++;

    LastFrameSeen = gPerformanceData.TotalFramesRednered;
}


void PPI_CharacterNaming(void)
{
    if (gGameInput.UpKeyIsDown && !gGameInput.UpKeyWasDown)
    {
        if (gMenu_CharacterNaming.SelectedItem > 12)
        {
            gMenu_CharacterNaming.SelectedItem -= 13;

            PlayGameSound(&gSoundMenuNavigate);
        }

        else
        {
            gMenu_CharacterNaming.SelectedItem = 52;

            PlayGameSound(&gSoundMenuNavigate);
        }
    }

    if (gGameInput.DownKeyIsDown && !gGameInput.DownKeyWasDown)
    {
        if (gMenu_CharacterNaming.SelectedItem < 39)
        {
            gMenu_CharacterNaming.SelectedItem += 13;

            PlayGameSound(&gSoundMenuNavigate);
        }
        else
        {
            gMenu_CharacterNaming.SelectedItem = 53;

            PlayGameSound(&gSoundMenuNavigate);
        }
    }

    if (gGameInput.LeftKeyIsDown && !gGameInput.LeftKeyWasDown)
    {
        if (gMenu_CharacterNaming.SelectedItem == 0 ||
            gMenu_CharacterNaming.SelectedItem == 13 ||
            gMenu_CharacterNaming.SelectedItem == 26 ||
            gMenu_CharacterNaming.SelectedItem == 39)
        {
            gMenu_CharacterNaming.SelectedItem += 12;
            PlayGameSound(&gSoundMenuNavigate);
        }

        else if (gMenu_CharacterNaming.SelectedItem > 0 && gMenu_CharacterNaming.SelectedItem != 52)
        {
            gMenu_CharacterNaming.SelectedItem--;

            PlayGameSound(&gSoundMenuNavigate);
        }
        else if (gMenu_CharacterNaming.SelectedItem == 52)
        {
            gMenu_CharacterNaming.SelectedItem++;
        }
    }

    if (gGameInput.RightKeyIsDown && !gGameInput.RightKeyWasDown)
    {
        if (gMenu_CharacterNaming.SelectedItem == 12 ||
            gMenu_CharacterNaming.SelectedItem == 25 ||
            gMenu_CharacterNaming.SelectedItem == 38 ||
            gMenu_CharacterNaming.SelectedItem == 51)
        {
            gMenu_CharacterNaming.SelectedItem -= 12;
        }

        else if (gMenu_CharacterNaming.SelectedItem < 53)
        {
            gMenu_CharacterNaming.SelectedItem++;

        }
        else if (gMenu_CharacterNaming.SelectedItem == 53)
        {
            gMenu_CharacterNaming.SelectedItem--;
        }
    }

    if (gGameInput.ChooseKeyIsDown && !gGameInput.ChooseKeyWasDown)
    {
        gMenu_CharacterNaming.Items[gMenu_CharacterNaming.SelectedItem]->Action();

        PlayGameSound(&gSoundMenuChoose);
    }

    if (gGameInput.EscapeKeyIsDown && !gGameInput.EscapeKeyWasDown)
    {
        MenuItem_CharacterNaming_Back();
    }
}

void MenuItem_CharacterNaming_Add(void)
{

    if (strlen(gPlayer.Name) < 8)
    {
        gPlayer.Name[strlen(gPlayer.Name)] = gMenu_CharacterNaming.Items[gMenu_CharacterNaming.SelectedItem]->Name[0];

        PlayGameSound(&gSoundMenuChoose);
    }
}

void MenuItem_CharacterNaming_Back(void)
{
    if (strlen(gPlayer.Name) < 1)
    {
        gPreviousGameState = gCurrentGameState;

        gCurrentGameState = GAMESTATE_TITLESCREEN;
    }
    else
    {
        gPlayer.Name[strlen(gPlayer.Name) - 1] = '\0';
    }
}

void MenuItem_CharacterNaming_OK(void)
{
    if (strlen(gPlayer.Name) > 0)
    {
        gPreviousGameState = gCurrentGameState;

        gCurrentGameState = GAMESTATE_OVERWORLD;

        gPlayer.Active = TRUE;

        PlayGameSound(&gSoundMenuChoose);
    }
}