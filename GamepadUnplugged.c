#include "Main.h"

#include "GamepadUnplugged.h"

void DrawGamepadUnplugged(void)
{
    PIXEL32 White = { 0xff, 0xff, 0xff, 0xff };

    memset(gBackBuffer.Memory, 0, GAME_DRAWING_AREA_MEMORY_SIZE);

    BlitStringToBuffer(GAMEPADUNPLUGGEDSTRING1, &g6x7Font, White, (GAME_RES_WIDTH / 2) - ((strlen(GAMEPADUNPLUGGEDSTRING1) * 6) / 2), 100);

    BlitStringToBuffer(GAMEPADUNPLUGGEDSTRING2, &g6x7Font, White, (GAME_RES_WIDTH / 2) - ((strlen(GAMEPADUNPLUGGEDSTRING2) * 6) / 2), 120);

}

void PPI_GamepadUnplugged(void)
{
    if (gGamepadID > -1 || (gGameInput.EscapeKeyIsDown) && !gGameInput.EscapeKeyIsDown)
    {
        gCurrentGameState = gPreviousGameState;

        gPreviousGameState = GAMESTATE_GAMEPADUNPLUGGED;
    }

}