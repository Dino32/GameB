
#include "Main.h"

#include "NewGamePrompt.h"

// Start New Game Screen

MENUITEM gMI_NewGame_Yes = { "Yes", (GAME_RES_WIDTH / 2) - ((3 * 6) / 2), 100, TRUE, MenuItem_NewGame_Yes };

MENUITEM gMI_NewGame_No = { "No", (GAME_RES_WIDTH / 2) - ((2 * 6) / 2), 120, TRUE, MenuItem_NewGame_No };

MENUITEM* gMI_NewGameItems[] = { &gMI_NewGame_Yes, &gMI_NewGame_No };

MENU gMenu_NewGameYesNo = { "Are you sure you want to start a new game?", 1, _countof(gMI_NewGameItems), gMI_NewGameItems };
//

void DrawNewGamePrompt()
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

    BlitStringToBuffer(gMenu_NewGameYesNo.Name, &g6x7Font, TextColor, (GAME_RES_WIDTH / 2) - ((uint16_t)strlen(gMenu_NewGameYesNo.Name) * 6) / 2, 60);

    BlitStringToBuffer(gMenu_NewGameYesNo.Items[0]->Name, &g6x7Font, TextColor, (GAME_RES_WIDTH / 2) - ((uint16_t)strlen(gMenu_NewGameYesNo.Items[0]->Name) * 6) / 2, 100);

    BlitStringToBuffer(gMenu_NewGameYesNo.Items[1]->Name, &g6x7Font, TextColor, (GAME_RES_WIDTH / 2) - ((uint16_t)strlen(gMenu_NewGameYesNo.Items[1]->Name) * 6) / 2, 120);

    BlitStringToBuffer("\xbb", &g6x7Font, TextColor, gMenu_NewGameYesNo.Items[gMenu_NewGameYesNo.SelectedItem]->x - 6, gMenu_NewGameYesNo.Items[gMenu_NewGameYesNo.SelectedItem]->y);

    LocalFrameCounter++;

    LastFrameSeen = gPerformanceData.TotalFramesRednered;
}

void PPI_NewGame(void)
{
    if (gGameInput.DownKeyIsDown && !gGameInput.DownKeyWasDown)
    {
        if (gMenu_NewGameYesNo.SelectedItem < gMenu_NewGameYesNo.ItemCount - 1)
        {
            gMenu_NewGameYesNo.SelectedItem++;

            PlayGameSound(&gSoundMenuNavigate);
        }
    }

    if (gGameInput.UpKeyIsDown && !gGameInput.UpKeyWasDown)
    {
        if (gMenu_NewGameYesNo.SelectedItem > 0)
        {
            gMenu_NewGameYesNo.SelectedItem--;

            PlayGameSound(&gSoundMenuNavigate);
        }
    }

    if (gGameInput.ChooseKeyIsDown && !gGameInput.ChooseKeyWasDown)
    {
        gMenu_NewGameYesNo.Items[gMenu_NewGameYesNo.SelectedItem]->Action();

        PlayGameSound(&gSoundMenuChoose);
    }
}

void MenuItem_NewGame_Yes(void)
{
    for (int i = 0; i < 8; i++)
    {
        gPlayer.Name[i] = '\0';
    }

    gPlayer.WorldPos.x = 64;

    gPlayer.WorldPos.y = 80;

    gPlayer.ScreenPos.x = 64;

    gPlayer.ScreenPos.y = 80;

    gCamera.x = 0;

    gCamera.y = 0;

    gPlayer.Direction = 0;

    gPlayer.MovementRemaining = 0;

    gPlayer.CurrentArmor = 0;

    gRegistryParams.GameSaved = 0;

    gCurrentArea = gOverwrldArea;

    gPreviousGameState = gCurrentGameState;

    gCurrentGameState = GAMESTATE_CHARACTERNAMING;
}

void MenuItem_NewGame_No(void)
{
    gPreviousGameState = gCurrentGameState;

    gCurrentGameState = GAMESTATE_TITLESCREEN;
}
