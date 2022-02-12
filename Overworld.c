#include "Main.h"

#include "Overworld.h"

void DrawOverworld(void)
{
    static uint64_t LocalFrameCounter;

    static uint64_t LastFrameSeen;

    static PIXEL32 TextColor;

    if (gPerformanceData.TotalFramesRednered > (LastFrameSeen + 1))
    {
        LocalFrameCounter = 0;
    }

    BlitTileMapToBuffer(&gOverWorld01.GameBitmap);

    Blit32BppBitmapToBuffer(&gPlayer.Sprite[gPlayer.CurrentArmor][gPlayer.SpriteIndex + gPlayer.Direction], gPlayer.ScreenPos.x, gPlayer.ScreenPos.y);

    LocalFrameCounter++;

    LastFrameSeen = gPerformanceData.TotalFramesRednered;
}

void PPI_Overworld(void)
{
    if (gGameInput.EscapeKeyIsDown)
    {
        SendMessageA(gGameWindow, WM_CLOSE, 0, 0);
    }

    if (!gPlayer.MovementRemaining)
    {
        if (gGameInput.DownKeyIsDown)
        {
            if (gPlayer.ScreenPos.y < GAME_RES_HEIGHT - 16)
            {
                gPlayer.Direction = DIRECTION_DOWN;

                gPlayer.MovementRemaining = 16;


            }
        }
        else if (gGameInput.LeftKeyIsDown)
        {
            if (gPlayer.ScreenPos.x > 0)
            {
                gPlayer.Direction = DIRECTION_LEFT;

                gPlayer.MovementRemaining = 16;
            }
        }
        else if (gGameInput.RightKeyIsDown)
        {
            if (gPlayer.ScreenPos.x < GAME_RES_WIDTH - 16)
            {
                gPlayer.Direction = DIRECTION_RIGTH;

                gPlayer.MovementRemaining = 16;
            }
        }
        else if (gGameInput.UpKeyIsDown)
        {
            if (gPlayer.ScreenPos.y > 0)
            {
                gPlayer.Direction = DIRECTION_UP;

                gPlayer.MovementRemaining = 16;
            }
        }
    }
    else
    {
        gPlayer.MovementRemaining--;

        if (gPlayer.Direction == DIRECTION_DOWN)
        {
            gPlayer.ScreenPos.y++;
        }
        else if (gPlayer.Direction == DIRECTION_LEFT)
        {
            gPlayer.ScreenPos.x--;
        }
        else if (gPlayer.Direction == DIRECTION_RIGTH)
        {
            gPlayer.ScreenPos.x++;
        }
        else if (gPlayer.Direction == DIRECTION_UP)
        {
            gPlayer.ScreenPos.y--;
        }

        switch (gPlayer.MovementRemaining)
        {
        case 16:
        {
            gPlayer.SpriteIndex = 0;

            break;
        }
        case 12:
        {
            gPlayer.SpriteIndex = 1;

            break;
        }
        case 8:
        {
            gPlayer.SpriteIndex = 0;

            break;
        }
        case 4:
        {
            gPlayer.SpriteIndex = 2;

            break;
        }

        case 0:
        {
            gPlayer.SpriteIndex = 0;

            break;
        }
        default:
        {

        }
        }
    }
}