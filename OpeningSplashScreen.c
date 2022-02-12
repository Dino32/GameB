#include "Main.h"

#include "OpeningSplashScreen.h"


void PPI_OpeningSplasheScreen(void)
{
    if (gGameInput.EscapeKeyIsDown && !gGameInput.EscapeKeyWasDown)
    {
        gCurrentGameState = GAMESTATE_TITLESCREEN;

        gPreviousGameState = GAMESTATE_OPENINGSPLASHSCREEN;
    }
}

void DrawOpeningSplashScreen(void)
{
    static uint64_t LocalFrameCounter;

    static uint64_t LastFrameSeen;

    static uint8_t BrightnessModifier = 0;

    static BOOL FadingSoundTunredOn = FALSE;

    if (gPerformanceData.TotalFramesRednered > (LastFrameSeen + 1)) // we have left the screen and came bac
    {
        LocalFrameCounter = 0;
    }

    if (LocalFrameCounter == 60)
    {
        PlayGameSound(&gSoundSplashScreen);
    }

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


        if (LocalFrameCounter == 260)
        {
            gPreviousGameState = gCurrentGameState;

            gCurrentGameState = GAMESTATE_TITLESCREEN;
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