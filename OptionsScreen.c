#include "Main.h"

#include "Options.h"

// Options Screen

MENUITEM gMI_OptionsScreen_SFXVolume = { "SFX Volume:", (GAME_RES_WIDTH / 2) - ((11 * 6) / 2), 100,  TRUE, MenuItem_OptionsScreen_SFXVolume };

MENUITEM gMI_OptionsScreen_MusicVolume = { "Music Volume:", (GAME_RES_WIDTH / 2) - ((13 * 6) / 2), 120, TRUE, MenuItem_OptionsScreen_MusicVolume };

MENUITEM gMI_OptionsScreen_ScreenSize = { "Screen Size:", (GAME_RES_WIDTH / 2) - ((12 * 6) / 2), 140, TRUE, MenuItem_OptionsScreen_ScreenSize };

MENUITEM gMI_OptionsScreen_Back = { "Back", (GAME_RES_WIDTH / 2) - (4 * 6) / 2, 160, TRUE, MenuItem_OptionsScreen_Back };

MENUITEM* gMI_OptionsScreenItems[] = { &gMI_OptionsScreen_SFXVolume, &gMI_OptionsScreen_MusicVolume, &gMI_OptionsScreen_ScreenSize, &gMI_OptionsScreen_Back };

MENU gMenu_OptionsScreen = { "Options", 0, _countof(gMI_OptionsScreenItems), gMI_OptionsScreenItems };

//

void DrawOptionsScreen(void)
{
    PIXEL32 Gray = { 0x6f, 0x6f, 0x6f, 0x06f };

    static uint64_t LocalFrameCounter;

    static uint64_t LastFrameSeen;

    static PIXEL32 TextColor = { 0x00, 0x00, 0x00, 0x00 };

    char ScreenSizeString[64] = { 0 };

    if (gPerformanceData.TotalFramesRednered > (LastFrameSeen + 1))
    {
        LocalFrameCounter = 0;

        TextColor.Red = 0x00;

        TextColor.Blue = 0x00;

        TextColor.Green = 0x00;

        gMenu_OptionsScreen.SelectedItem = 0;

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

    for (uint8_t MenuItem = 0; MenuItem < gMenu_OptionsScreen.ItemCount; MenuItem++)
    {
        if (gMenu_OptionsScreen.Items[MenuItem]->Enabled == TRUE)
        {
            BlitStringToBuffer(gMenu_OptionsScreen.Items[MenuItem]->Name, &g6x7Font, TextColor, gMenu_OptionsScreen.Items[MenuItem]->x, gMenu_OptionsScreen.Items[MenuItem]->y);
        }
    }

    for (uint8_t Volume = 0; Volume < 10; Volume++)
    {
        if (Volume >= (uint8_t)(gSFXVolume * 10))
        {
            if (TextColor.Red == 255)
            {
                BlitStringToBuffer("\xf2", &g6x7Font, Gray, 240 + (Volume * 6), 100);
            }
        }
        else
        {
            BlitStringToBuffer("\xf2", &g6x7Font, TextColor, 240 + (Volume * 6), 100);

        }
    }

    for (uint8_t Volume = 0; Volume < 10; Volume++)
    {
        if (Volume >= (uint8_t)(gMusicVolume * 10))
        {
            if (TextColor.Red == 255)
            {
                BlitStringToBuffer("\xf2", &g6x7Font, Gray, 240 + (Volume * 6), 120);
            }
        }
        else
        {
            BlitStringToBuffer("\xf2", &g6x7Font, TextColor, 240 + (Volume * 6), 120);

        }
    }

    snprintf(ScreenSizeString, sizeof(ScreenSizeString), "%dx%d", GAME_RES_WIDTH * gPerformanceData.CurrentScaleFactor, GAME_RES_HEIGHT * gPerformanceData.CurrentScaleFactor);

    BlitStringToBuffer(ScreenSizeString, &g6x7Font, TextColor, 240, gMI_OptionsScreen_ScreenSize.y);

    BlitStringToBuffer("\xbb", &g6x7Font, TextColor, gMenu_OptionsScreen.Items[gMenu_OptionsScreen.SelectedItem]->x - 6, gMenu_OptionsScreen.Items[gMenu_OptionsScreen.SelectedItem]->y);


    LocalFrameCounter++;

    LastFrameSeen = gPerformanceData.TotalFramesRednered;
}

void PPI_OptionsScreen(void)
{
    if (gGameInput.DownKeyIsDown && !gGameInput.DownKeyWasDown)
    {
        if (gMenu_OptionsScreen.SelectedItem < gMenu_OptionsScreen.ItemCount - 1)
        {
            gMenu_OptionsScreen.SelectedItem++;

            PlayGameSound(&gSoundMenuNavigate);
        }
    }

    if (gGameInput.UpKeyIsDown && !gGameInput.UpKeyWasDown)
    {
        if (gMenu_OptionsScreen.SelectedItem > 0)
        {
            gMenu_OptionsScreen.SelectedItem--;

            PlayGameSound(&gSoundMenuNavigate);
        }
    }

    if (gGameInput.ChooseKeyIsDown && !gGameInput.ChooseKeyWasDown)
    {

        gMenu_OptionsScreen.Items[gMenu_OptionsScreen.SelectedItem]->Action();

        PlayGameSound(&gSoundMenuChoose);
    }
}

void MenuItem_OptionsScreen_SFXVolume(void)
{
    gSFXVolume += 0.1f;

    if ((uint8_t)(gSFXVolume * 10) > 10)
    {
        gSFXVolume = 0.f;
    }

    for (uint8_t Counter = 0; Counter < NUMBER_OF_SFX_SOURCE_VOICES; Counter++)
    {
        gXAudioSFXSorceVoice[Counter]->lpVtbl->SetVolume(gXAudioSFXSorceVoice[Counter], gSFXVolume, XAUDIO2_COMMIT_NOW);
    }
}

void MenuItem_OptionsScreen_MusicVolume(void)
{
    gMusicVolume += 0.1f;

    if ((uint8_t)(gMusicVolume * 10) > 10)
    {
        gMusicVolume = 0.f;
    }

    gXAudioMusicSourceVoice->lpVtbl->SetVolume(gXAudioMusicSourceVoice, gMusicVolume, XAUDIO2_COMMIT_NOW);
}

void MenuItem_OptionsScreen_ScreenSize(void)
{
    if (gPerformanceData.CurrentScaleFactor < gPerformanceData.MaxScaleFactor)
    {
        gPerformanceData.CurrentScaleFactor++;
    }
    else
    {
        gPerformanceData.CurrentScaleFactor = 1;
    }

    InvalidateRect(gGameWindow, NULL, TRUE);
}

void MenuItem_OptionsScreen_Back(void)
{
    gPreviousGameState = gCurrentGameState;

    gCurrentGameState = GAMESTATE_TITLESCREEN;

    if (SaveRegistryParametars() != ERROR_SUCCESS)
    {
        LogMessageA(Error, "[%s] SaveRegistryParameters failed!", __FUNCTION__);
    }
}