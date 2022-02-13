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

    if (LocalFrameCounter == 60)
    {
        PlayGameMusic(&gMusicOverworld01);
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
            BOOL CanMoveToDesiredTile = FALSE;

            for (uint8_t Counter = 0; Counter < _countof(gPassablieTiles); Counter++)
            {
                if (gOverWorld01.TIleMap.Map[gPlayer.WorldPos.y / 16 + 1][gPlayer.WorldPos.x / 16] == gPassablieTiles[Counter])
                {
                    CanMoveToDesiredTile = TRUE;

                    break;
                }
            }

            if (CanMoveToDesiredTile)
            {
                

                if (gPlayer.ScreenPos.y < GAME_RES_HEIGHT - 16)
                {
                    gPlayer.Direction = DIRECTION_DOWN;

                    gPlayer.MovementRemaining = 16;
                }
            }

        }

        else if (gGameInput.LeftKeyIsDown)
        {
            BOOL CanMoveToDesiredTile = FALSE;

            for (uint8_t Counter = 0; Counter < _countof(gPassablieTiles); Counter++)
            {
                if (gOverWorld01.TIleMap.Map[gPlayer.WorldPos.y / 16][gPlayer.WorldPos.x / 16 - 1] == gPassablieTiles[Counter])
                {
                    CanMoveToDesiredTile = TRUE;

                    break;
                }
            }

            if (CanMoveToDesiredTile)
            {
                if (gPlayer.ScreenPos.x > 0)
                {
                    gPlayer.Direction = DIRECTION_LEFT;

                    gPlayer.MovementRemaining = 16;
                }
            }
        }
        else if (gGameInput.RightKeyIsDown)
        {
            BOOL CanMoveToDesiredTile = FALSE;

            for (uint8_t Counter = 0; Counter < _countof(gPassablieTiles); Counter++)
            {
                if (gOverWorld01.TIleMap.Map[gPlayer.WorldPos.y / 16][gPlayer.WorldPos.x / 16 + 1] == gPassablieTiles[Counter])
                {
                    CanMoveToDesiredTile = TRUE;

                    break;
                }
            }

            if (CanMoveToDesiredTile)
            {

                if (gPlayer.ScreenPos.x < GAME_RES_WIDTH - 16)
                {
                    gPlayer.Direction = DIRECTION_RIGTH;

                    gPlayer.MovementRemaining = 16;
                }
            }
        }
        else if (gGameInput.UpKeyIsDown)
        {
            BOOL CanMoveToDesiredTile = FALSE;

            for (uint8_t Counter = 0; Counter < _countof(gPassablieTiles); Counter++)
            {
                if (gPlayer.WorldPos.y > 0 && gOverWorld01.TIleMap.Map[gPlayer.WorldPos.y / 16 - 1][gPlayer.WorldPos.x / 16] == gPassablieTiles[Counter])
                {
                    CanMoveToDesiredTile = TRUE;

                    break;
                }
            }

            if (CanMoveToDesiredTile)
            {
                if (gPlayer.ScreenPos.y > 0)
                {
                    gPlayer.Direction = DIRECTION_UP;

                    gPlayer.MovementRemaining = 16;
                }
            }
        }
    }
    else
    {
        gPlayer.MovementRemaining--;

        if (gPlayer.Direction == DIRECTION_DOWN)
        {
            if (gPlayer.ScreenPos.y < GAME_RES_HEIGHT - 64)
            {
                gPlayer.ScreenPos.y++;
            }
            else
            {
                gCamera.y++;
            }

            gPlayer.WorldPos.y++;
        }
        else if (gPlayer.Direction == DIRECTION_LEFT)
        {
            if (gPlayer.ScreenPos.x > 64)
            {
                gPlayer.ScreenPos.x--;
            }
            else
            {
                if (gCamera.x > 0)
                {
                    gCamera.x--;
                }
                else
                {
                    gPlayer.ScreenPos.x--;
                }
            }
            gPlayer.WorldPos.x--;
        }
        else if (gPlayer.Direction == DIRECTION_RIGTH)
        {
            if (gPlayer.ScreenPos.x < GAME_RES_WIDTH - 64)
            {
                gPlayer.ScreenPos.x++;
            }
            else 
            {
                if (gCamera.x < gOverWorld01.TIleMap.Width * 16- 320)
                {
                    gCamera.x++;
                }
                else
                {
                    gPlayer.ScreenPos.x++;
                }
            }

            gPlayer.WorldPos.x++;
        }
        else if (gPlayer.Direction == DIRECTION_UP)
        {
            if (gPlayer.ScreenPos.y  > 64)
            {
                gPlayer.ScreenPos.y--;
            }
            else 
            {
                if (gCamera.y > 0)
                {
                    gCamera.y--;
                }
                else
                {
                    gPlayer.ScreenPos.y--;
                }
            }

            gPlayer.WorldPos.y--;
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