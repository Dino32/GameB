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

    if (gPerformanceData.DisplayDegubInfo)
    {
        char* Buffer[4] = { 0 };

        _itoa_s(gOverWorld01.TIleMap.Map[gPlayer.WorldPos.y / 16][gPlayer.WorldPos.x / 16], Buffer, sizeof(Buffer), 10);

        BlitStringToBuffer(Buffer, &g6x7Font, (PIXEL32) { 0xff, 0xff, 0xff, 0xff }, (gPlayer.ScreenPos.x) + 5, gPlayer.ScreenPos.y + 4);

        if (gPlayer.WorldPos.x <= gCurrentArea.right -32)
        {
            _itoa_s(gOverWorld01.TIleMap.Map[(gPlayer.WorldPos.y) / 16][(gPlayer.WorldPos.x + 16) / 16], Buffer, sizeof(Buffer), 10);

            BlitStringToBuffer(Buffer, &g6x7Font, (PIXEL32) { 0xff, 0xff, 0xff, 0xff }, (gPlayer.ScreenPos.x + 16) + 5, gPlayer.ScreenPos.y + 4);
        }

        if (gPlayer.WorldPos.x >= gCurrentArea.left)
        {
            _itoa_s(gOverWorld01.TIleMap.Map[(gPlayer.WorldPos.y) / 16][(gPlayer.WorldPos.x - 16) / 16], Buffer, sizeof(Buffer), 10);

            BlitStringToBuffer(Buffer, &g6x7Font, (PIXEL32) { 0xff, 0xff, 0xff, 0xff }, (gPlayer.ScreenPos.x - 16) + 5, gPlayer.ScreenPos.y + 4);
        }

        if (gPlayer.WorldPos.y <= gCurrentArea.bottom - 32)
        {
            _itoa_s(gOverWorld01.TIleMap.Map[(gPlayer.WorldPos.y + 16) / 16][(gPlayer.WorldPos.x) / 16], Buffer, sizeof(Buffer), 10);

            BlitStringToBuffer(Buffer, &g6x7Font, (PIXEL32) { 0xff, 0xff, 0xff, 0xff }, (gPlayer.ScreenPos.x) + 5, gPlayer.ScreenPos.y + 16 + 4);
        }

        if (gPlayer.WorldPos.y >= 16)
        {
            _itoa_s(gOverWorld01.TIleMap.Map[(gPlayer.WorldPos.y - 16) / 16][(gPlayer.WorldPos.x) / 16], Buffer, sizeof(Buffer), 10);

            BlitStringToBuffer(Buffer, &g6x7Font, (PIXEL32) { 0xff, 0xff, 0xff, 0xff }, (gPlayer.ScreenPos.x) + 5, gPlayer.ScreenPos.y - 16 + 4);
        }
    }

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
                if (gPlayer.WorldPos.y < gCurrentArea.bottom - 16 && gOverWorld01.TIleMap.Map[gPlayer.WorldPos.y / 16 + 1][gPlayer.WorldPos.x / 16] == gPassablieTiles[Counter])
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
                if (gCamera.y < gCurrentArea.bottom - GAME_RES_HEIGHT)
                {
                    gCamera.y++;
                }
                else
                {
                    gPlayer.ScreenPos.y++;
                }
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
                if (gCamera.x > gCurrentArea.left)
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
                if (gCamera.x < gCurrentArea.right - GAME_RES_WIDTH)
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
            if (gPlayer.ScreenPos.y > 64)
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
            // MovmentRemaining will be at most 15
            gPlayer.HasPlayerMoveSincePortal = TRUE;

            gPlayer.SpriteIndex = 0;

            break;
        }
        case 15:
        {
            gPlayer.HasPlayerMoveSincePortal = TRUE;

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

            // Is the player standing on a portal
            if (gOverWorld01.TIleMap.Map[gPlayer.WorldPos.y / 16][gPlayer.WorldPos.x / 16] == TILE_PORTAL_01)
            {
                if (gPlayer.HasPlayerMoveSincePortal)
                {
                    PortalHandler();
                }
            }

            break;
        }
        default:
        {

        }

        }
        
    }
}

void PortalHandler(void)
{
    gPlayer.HasPlayerMoveSincePortal = FALSE;

    BOOL PortalFound = FALSE;

    for (uint16_t Counter = 0; Counter < _countof(gPortals); Counter++)
    {
        if (gPlayer.WorldPos.x == gPortals[Counter].WorldPos.x && gPlayer.WorldPos.y == gPortals[Counter].WorldPos.y)
        {
            PortalFound = TRUE;

            gPlayer.WorldPos.x = gPortals[Counter].WorldDest.x;

            gPlayer.WorldPos.y = gPortals[Counter].WorldDest.y;

            gPlayer.ScreenPos.x = gPortals[Counter].ScreenPos.x;

            gPlayer.ScreenPos.y = gPortals[Counter].ScreenPos.y;

            gCamera.x = gPortals[Counter].CameraPos.x;

            gCamera.y = gPortals[Counter].CameraPos.y;

            gCurrentArea = gPortals[Counter].Area;
            break;
        }
    }

    if (!PortalFound)
    {
        ASSERT(FALSE, "Player is standing on the portal but we don't have it implemented.");
    }
}