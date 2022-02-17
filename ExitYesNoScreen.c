#include "Main.h"

#include "ExitYesNo.h"

// Exit Yes or No Screen

MENUITEM gMI_ExitYesNo_Yes = { "Yes", (GAME_RES_WIDTH / 2) - ((3 * 6) / 2), 100, TRUE, MenuItem_ExitYesNo_Yes };

MENUITEM gMI_ExitYesNo_No = { "No", (GAME_RES_WIDTH / 2) - ((2 * 6) / 2), 120, TRUE, MenuItem_ExitYesNo_No };

MENUITEM* gMI_ExitYesNoItems[] = { &gMI_ExitYesNo_Yes, &gMI_ExitYesNo_No };

MENU gMenu_ExitYesNo = { "Are you sure you want to exit?", 1, _countof(gMI_ExitYesNoItems), gMI_ExitYesNoItems };
//

void DrawExitYesNoExitScreen()
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

        gInputEnabled =  FALSE;
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

    BlitStringToBuffer(gMenu_ExitYesNo.Name, &g6x7Font, TextColor, (GAME_RES_WIDTH / 2) - ((uint16_t)strlen(gMenu_ExitYesNo.Name) * 6) / 2, 60);

    BlitStringToBuffer(gMenu_ExitYesNo.Items[0]->Name, &g6x7Font, TextColor, (GAME_RES_WIDTH / 2) - ((uint16_t)strlen(gMenu_ExitYesNo.Items[0]->Name) * 6) / 2, 100);

    BlitStringToBuffer(gMenu_ExitYesNo.Items[1]->Name, &g6x7Font, TextColor, (GAME_RES_WIDTH / 2) - ((uint16_t)strlen(gMenu_ExitYesNo.Items[1]->Name) * 6) / 2, 120);

    BlitStringToBuffer("\xbb", &g6x7Font, TextColor, gMenu_ExitYesNo.Items[gMenu_ExitYesNo.SelectedItem]->x - 6, gMenu_ExitYesNo.Items[gMenu_ExitYesNo.SelectedItem]->y);

    LocalFrameCounter++;

    LastFrameSeen = gPerformanceData.TotalFramesRednered;
}

void PPI_ExitYesNo(void)
{
    if (gGameInput.DownKeyIsDown && !gGameInput.DownKeyWasDown)
    {
        if (gMenu_ExitYesNo.SelectedItem < gMenu_ExitYesNo.ItemCount - 1)
        {
            gMenu_ExitYesNo.SelectedItem++;

            PlayGameSound(&gSoundMenuNavigate);
        }
    }

    if (gGameInput.UpKeyIsDown && !gGameInput.UpKeyWasDown)
    {
        if (gMenu_ExitYesNo.SelectedItem > 0)
        {
            gMenu_ExitYesNo.SelectedItem--;

            PlayGameSound(&gSoundMenuNavigate);
        }
    }

    if (gGameInput.ChooseKeyIsDown && !gGameInput.ChooseKeyWasDown)
    {
        gMenu_ExitYesNo.Items[gMenu_ExitYesNo.SelectedItem]->Action();

        PlayGameSound(&gSoundMenuChoose);
    }
}

void MenuItem_ExitYesNo_Yes(void)
{
    SendMessageA(gGameWindow, WM_CLOSE, 0, 0);
}

void MenuItem_ExitYesNo_No(void)
{
    gPreviousGameState = gCurrentGameState;

    gCurrentGameState = GAMESTATE_TITLESCREEN;

}
