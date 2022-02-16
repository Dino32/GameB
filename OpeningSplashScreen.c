#include "Main.h"

#include "OpeningSplashScreen.h"


void PPI_OpeningSplasheScreen(void)
{
    if (gGameInput.EscapeKeyIsDown && !gGameInput.EscapeKeyWasDown)
    {
        if (WaitForSingleObject(gAssetLoadingThreadHandle, 0) == WAIT_OBJECT_0)
        {
            DWORD ThreadExitCode = ERROR_SUCCESS;

            GetExitCodeThread(gAssetLoadingThreadHandle, &ThreadExitCode);

            if (ThreadExitCode == ERROR_SUCCESS)
            {
                gPreviousGameState = gCurrentGameState;

                gCurrentGameState = GAMESTATE_TITLESCREEN;

            }

        }
    }
}

void DrawOpeningSplashScreen(void)
{
    static uint64_t LocalFrameCounter;

    static uint64_t LastFrameSeen;

    static uint8_t BrightnessModifier = 0;

    static BOOL FadingSoundTunredOn = FALSE;

    static uint64_t Blink;


    if (WaitForSingleObject(gEssentialAssetsLoadedEvent, 0) != WAIT_OBJECT_0)
    {
        return;
    }

    if (gPerformanceData.TotalFramesRednered > (LastFrameSeen + 1)) // we have left the screen and came bac
    {
        LocalFrameCounter = 0;
    }

    if (LocalFrameCounter == 60)
    {
        PlayGameSound(&gSoundSplashScreen);
    }

    if ((Blink % 10) == 0)
    {
        BlitStringToBuffer("o", &g6x7Font, (PIXEL32) { 0x00, 0xff, 0x00, 0xff }, GAME_RES_WIDTH - 6, GAME_RES_HEIGHT - 7);
    }
    
    if ((Blink % 30) == 0)
    {
        BlitStringToBuffer("o", &g6x7Font, (PIXEL32) { 0x00, 0x00, 0x00, 0xff }, GAME_RES_WIDTH - 6, GAME_RES_HEIGHT - 7);
    }

    Blink++;

    if (LocalFrameCounter > 60)
    {
        if (LocalFrameCounter >= 150 && LocalFrameCounter % 10 == 0 && LocalFrameCounter <= 250)
        {
            BrightnessModifier += 16;
        }
        if (LocalFrameCounter == 255 || LocalFrameCounter == 260 || LocalFrameCounter == 265 || LocalFrameCounter == 270)
        {
            BrightnessModifier += 18;
        }


        if (LocalFrameCounter >= 260)
        {
            if (WaitForSingleObject(gAssetLoadingThreadHandle, 0) == WAIT_OBJECT_0)
            {
                DWORD ThreadExitCode = ERROR_SUCCESS;

                GetExitCodeThread(gAssetLoadingThreadHandle, &ThreadExitCode);

                if (ThreadExitCode != ERROR_SUCCESS)
                {
                    LogMessageA(Error, "[%s] Asset Loading Thread failed with 0x%08lx!", __FUNCTION__, ThreadExitCode);
                
                    gGameIsRunning = FALSE;

                    MessageBoxA(gGameWindow, "Asset Loading Failed.", "Error!", MB_ICONERROR | MB_OK);
                    
                    return;
                }
                gPreviousGameState = gCurrentGameState;

                gCurrentGameState = GAMESTATE_TITLESCREEN;
            }
        }

        BlitStringToBuffer("- Game Studio -", &g6x7Font, ((PIXEL32){0xFF - BrightnessModifier, 0xFF - BrightnessModifier, 0xFF - BrightnessModifier, 0xFF}), (GAME_RES_WIDTH / 2) - (15 * 6) / 2, 110);

        BlitStringToBuffer("Presents", &g6x7Font, ((PIXEL32){0xFF - BrightnessModifier, 0xFF - BrightnessModifier, 0xFF - BrightnessModifier, 0xFF}), (GAME_RES_WIDTH / 2) - (8 * 6) / 2, 125);
    }

    if (LocalFrameCounter >= 140 && !FadingSoundTunredOn)
    {
        FadingSoundTunredOn = TRUE;

        PlayGameSound(&gSoundFadingScreen);
    }


    LocalFrameCounter++;

    LastFrameSeen = gPerformanceData.TotalFramesRednered;
}