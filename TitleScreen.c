#include "Main.h"
#include "Title.h"

// Title Screen


MENUITEM gMI_StartNewGame = { "Start New Game", (GAME_RES_WIDTH / 2) - ((14 * 6) / 2), 120, TRUE,  MenuItem_TitleScree_StartNew };

MENUITEM gMI_Options = { "Options", (GAME_RES_WIDTH / 2) - ((7 * 6) / 2), 140, TRUE,  MenuItem_TitleScree_Options };

MENUITEM gMI_Exit = { "Exit", (GAME_RES_WIDTH / 2) - ((5 * 6) / 2), 160, TRUE, MenuItem_TitleScree_Exit };

MENUITEM gMI_Save = { "Save Game", (GAME_RES_WIDTH / 2) - (9 * 6) / 2, 180, TRUE, MenuItem_TitleScreen_Save};

MENUITEM gMI_ContinueGame = { "Continue Game", (GAME_RES_WIDTH / 2) - (13 * 6) / 2, 100, TRUE, MenuItem_TitleScreen_LoadGame};

MENUITEM* gMI_TitleScreenItems[] = { &gMI_ContinueGame, &gMI_StartNewGame, &gMI_Options, &gMI_Exit, &gMI_Save };

MENU gMenu_TitleScreen = { "Title Screen Menu", 0, _countof(gMI_TitleScreenItems), gMI_TitleScreenItems };

//

void DrawTitleScreen(void)
{
    static uint64_t LocalFrameCounter;

    static uint64_t LastFrameSeen;

    static PIXEL32 TextColor = { 0x00, 0x00, 0x00, 0x00 };

    if (gPreviousGameState == GAMESTATE_OVERWORLD)
    {
        PlayGameMusic(&gMusicNoSound);
    }

    if (gPerformanceData.TotalFramesRednered > (LastFrameSeen + 1))
    {
        LocalFrameCounter = 0;

        TextColor.Red = 0x00;

        TextColor.Blue = 0x00;

        TextColor.Green = 0x00;

        if (!gPlayer.Active && gRegistryParams.GameSaved != 1)
        {
            gMI_ContinueGame.Enabled = FALSE;

            gMI_Save.Enabled = FALSE;

            gMenu_TitleScreen.SelectedItem = 1;
        }
        else if (!gPlayer.Active && gRegistryParams.GameSaved == 1)
        {
            gMI_ContinueGame.Enabled = TRUE;

            gMI_Save.Enabled = FALSE;

            gMenu_TitleScreen.SelectedItem = 0;
        } 
        else if (gPlayer.Active && gRegistryParams.GameSaved != 1)
        {
            gMI_ContinueGame.Enabled = TRUE;

            gMI_Save.Enabled = TRUE;

            gMenu_TitleScreen.SelectedItem = 0;
        } 
        else if (gPlayer.Active && gRegistryParams.GameSaved == 1)
        {
            gMI_ContinueGame.Enabled = TRUE;

            gMI_Save.Enabled = TRUE;

            gMenu_TitleScreen.SelectedItem = 0;
        }

        if (gPreviousGameState == GAMESTATE_NEWGAMEPROMPT && gRegistryParams.GameSaved == 1)
        {
            gMI_ContinueGame.Enabled = TRUE;

            gMI_Save.Enabled = FALSE;

            gMenu_TitleScreen.SelectedItem = 0;
        }

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

    BlitStringToBuffer(GAME_NAME, &g6x7Font, TextColor, (GAME_RES_WIDTH / 2) - ((strlen(GAME_NAME) * 6) / 2), 60);

    for (uint8_t MenuItem = 0; MenuItem < gMenu_TitleScreen.ItemCount; MenuItem++)
    {
        if (gMenu_TitleScreen.Items[MenuItem]->Enabled == TRUE)
        {
            BlitStringToBuffer(gMenu_TitleScreen.Items[MenuItem]->Name, &g6x7Font, TextColor, gMenu_TitleScreen.Items[MenuItem]->x, gMenu_TitleScreen.Items[MenuItem]->y);
        }
        /*if (gPreviousGameState == GAMESTATE_CHARACTERNAMING && MenuItem == 0)
        {
            BlitStringToBuffer(gMenu_TitleScreen.Items[MenuItem]->Name, &g6x7Font, TextColor, gMenu_TitleScreen.Items[MenuItem]->x, gMenu_TitleScreen.Items[MenuItem]->y);
        }*/
    }

    if (gMenu_TitleScreen.Items[gMenu_TitleScreen.SelectedItem]->Enabled)
    {
        BlitStringToBuffer("\xbb", &g6x7Font, TextColor, gMenu_TitleScreen.Items[gMenu_TitleScreen.SelectedItem]->x - 6, gMenu_TitleScreen.Items[gMenu_TitleScreen.SelectedItem]->y);
    }

    LocalFrameCounter++;

    LastFrameSeen = gPerformanceData.TotalFramesRednered;
}

void PPI_TitleScreen(void)
{


    if (gGameInput.DownKeyIsDown && !gGameInput.DownKeyWasDown)
    {
        if (gMenu_TitleScreen.SelectedItem < gMenu_TitleScreen.ItemCount - 1 && gMenu_TitleScreen.Items[gMenu_TitleScreen.SelectedItem + 1]->Enabled)
        {
            gMenu_TitleScreen.SelectedItem++;

            PlayGameSound(&gSoundMenuNavigate);
        }
    }

    if (gGameInput.UpKeyIsDown && !gGameInput.UpKeyWasDown)
    {
        if (gMenu_TitleScreen.SelectedItem > 0)
        {
            if (gMenu_TitleScreen.SelectedItem == 1 && gRegistryParams.GameSaved == 1)
            {
                gMenu_TitleScreen.SelectedItem--;

                PlayGameSound(&gSoundMenuNavigate);
            }
            else if (gMenu_TitleScreen.SelectedItem == 1 && gRegistryParams.GameSaved == 0)
            {

            }
            else
            {
                gMenu_TitleScreen.SelectedItem--;

                PlayGameSound(&gSoundMenuNavigate);
            }
        }

        
    }

    if (gGameInput.ChooseKeyIsDown && !gGameInput.ChooseKeyWasDown)
    {
        gMenu_TitleScreen.Items[gMenu_TitleScreen.SelectedItem]->Action();

        PlayGameSound(&gSoundMenuChoose);
    }
}

void MenuItem_TitleScreen_Resume(void)
{
    gPreviousGameState = gCurrentGameState;

    gCurrentGameState = GAMESTATE_OVERWORLD;
}

void MenuItem_TitleScree_StartNew(void)
{
    // If a game i already in progress, this should prompt the user if they are sure that they want to start the new game first,
    // and lose any unsaved progress.
    // Otherwise, just go to the character naming screen.

    gPreviousGameState = gCurrentGameState;

    gCurrentGameState = GAMESTATE_NEWGAMEPROMPT;

    LogMessageA(Informational, "[%s] gPlayer.WorldPos.x is %d", __FUNCTION__, gPlayer.WorldPos.x);
}

void MenuItem_TitleScree_Options(void)
{
    gPreviousGameState = gCurrentGameState;

    gCurrentGameState = GAMESTATE_OPTIONSSCREEN;
}

void MenuItem_TitleScree_Exit(void)
{
    gPreviousGameState = gCurrentGameState;

    gCurrentGameState = GAMESTATE_EXITYESNOSCREEN;
}

void MenuItem_TitleScreen_Save(void)
{
    gRegistryParams.GameSaved = 1;

    if (SaveRegistryParametars() != ERROR_SUCCESS)
    {
        LogMessageA(Error, "[%s] SaveRegistryParameters failed!", __FUNCTION__);
    }

    gPreviousGameState = gCurrentGameState;

    gCurrentGameState = GAMESTATE_SAVEGAME;
}

void MenuItem_TitleScreen_LoadGame(void)
{
    gPlayer.Active = TRUE;

    gPreviousGameState = gCurrentGameState;

    gCurrentGameState = GAMESTATE_OVERWORLD;
}