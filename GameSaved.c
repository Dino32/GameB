#include "Main.h"

#include "GameSaved.h"

MENUITEM gMI_OK = { "OK", (GAME_RES_WIDTH / 2) - (2 * 6) / 2, 130, TRUE, MenuItem_SaveScreen_OK};

MENUITEM* gMI_SaveGameItems[] = {&gMI_OK};

MENU gMenu_SaveScreen = { "Progress was successfully saved!", 0, _countof(gMI_SaveGameItems), gMI_SaveGameItems};

void DrawGameSavedScreen(void)
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

        gInputEnabled = FALSE;
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

            gInputEnabled = TRUE;
        }
    }

    BlitStringToBuffer(gMenu_SaveScreen.Name, &g6x7Font, TextColor, (GAME_RES_WIDTH / 2) - (6 * strlen(gMenu_SaveScreen.Name)) / 2, 100);
    
    BlitStringToBuffer(gMenu_SaveScreen.Items[0]->Name, &g6x7Font, TextColor, gMenu_SaveScreen.Items[0]->x, gMenu_SaveScreen.Items[0]->y);

    BlitStringToBuffer("\xbb", &g6x7Font, TextColor, gMenu_SaveScreen.Items[gMenu_SaveScreen.SelectedItem]->x - 6, gMenu_SaveScreen.Items[gMenu_SaveScreen.SelectedItem]->y);

    LocalFrameCounter++;

    LastFrameSeen = gPerformanceData.TotalFramesRednered;
}

void PPI_GameSavedScreen(void)
{
    if (gGameInput.ChooseKeyIsDown && !gGameInput.ChooseKeyWasDown)
    {
        gMenu_SaveScreen.Items[gMenu_SaveScreen.SelectedItem]->Action();

        PlayGameSound(&gSoundMenuChoose);
    }
}

void MenuItem_SaveScreen_OK(void)
{
	gPreviousGameState = gCurrentGameState;

	gCurrentGameState = GAMESTATE_TITLESCREEN;
}