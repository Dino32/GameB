#include <stdio.h>

#pragma warning(push, 3)
// with pragma we are telling a compailer to behave in a centire way
// disable all warnings appliead to windows.h
#include <windows.h>
// pop out current level of warning (from  to level 4)
#pragma warning(pop)
// its important that this is included after windows.h
#include <stdint.h>
#include "Main.h"
#pragma warning (disable: 4668)

#define GAME_NAME "GAMEB"
#define GAME_RES_WIDTH   384
#define GAME_RES_HEIGHT   240
#define GAME_BPP 32
#define GAME_DRAWING_AREA_MEMORY_SIZE (GAME_RES_WIDTH  * GAME_RES_HEIGHT * (GAME_BPP  / 8))
#define CALCULATE_AVG_FPS_EVERY_X_FRAMES 100

BOOL gGameIsRunning; // gloabal variable, its automatically initialized to zero (false)

HWND gGameWindow;

GAMEBITMAP gBackBuffer;

GAMEPERFDATA gPerformanceData;


INT WINAPI WinMain(HINSTANCE Instance, HINSTANCE PreviousInstance,
    PSTR CommandLine, INT CommandShow) {

    UNREFERENCED_PARAMETER(PreviousInstance);
    UNREFERENCED_PARAMETER(CommandLine);
    UNREFERENCED_PARAMETER(Instance);


    if (GameIsAlreadyRunning()) {
        MessageBoxA(NULL, "Another instance of this program is already running!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    if (CreateMainGameWindow() != ERROR_SUCCESS) {
        goto Exit;
    }

    QueryPerformanceFrequency(&gPerformanceData.Perffrequency);

    gBackBuffer.Bitmapinfo.bmiHeader.biSize = sizeof(gBackBuffer.Bitmapinfo.bmiHeader);
    gBackBuffer.Bitmapinfo.bmiHeader.biWidth = GAME_RES_WIDTH;
    gBackBuffer.Bitmapinfo.bmiHeader.biHeight = GAME_RES_HEIGHT;
    gBackBuffer.Bitmapinfo.bmiHeader.biBitCount = GAME_BPP;
    gBackBuffer.Bitmapinfo.bmiHeader.biCompression = BI_RGB;
    gBackBuffer.Bitmapinfo.bmiHeader.biPlanes = 1;

    gBackBuffer.Memory = VirtualAlloc(NULL, GAME_DRAWING_AREA_MEMORY_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (gBackBuffer.Memory == NULL)
    {
        MessageBoxA(NULL, "Faild to allocate memory for drawing surface!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto Exit;
    }
 
    MSG Message = { 0 };

    gGameIsRunning = TRUE;

    // while getMessage > 0 dispatche message

    while (gGameIsRunning)
    {
        QueryPerformanceCounter(&gPerformanceData.FrameStart);

        while (PeekMessageA(&Message, gGameWindow, 0, 0, PM_REMOVE)) 
        {
            DispatchMessageA(&Message);
        }

        ProcessPlayerInput();

        RednerFrameGraphics();

        QueryPerformanceCounter(&gPerformanceData.FrameEnd);

        gPerformanceData.ElapsedMicrosecondsPerFrame.QuadPart = gPerformanceData.FrameEnd.QuadPart - gPerformanceData.FrameStart.QuadPart;

        gPerformanceData.ElapsedMicrosecondsPerFrame.QuadPart *= 1000000;
        gPerformanceData.ElapsedMicrosecondsPerFrame.QuadPart /= gPerformanceData.Perffrequency.QuadPart;

        Sleep(1);

        gPerformanceData.TotalFramesRednered++;

        if ((gPerformanceData.TotalFramesRednered % CALCULATE_AVG_FPS_EVERY_X_FRAMES) == 0)
        {
            char str[64] = { 0 };

            _snprintf_s(str,    _countof(str), _TRUNCATE, "Elapsed mircoseconds: %lli\n", gPerformanceData.ElapsedMicrosecondsPerFrame.QuadPart);

            OutputDebugStringA(str);
        }

    }
Exit: return 0;
}

LRESULT CALLBACK MainWndowProc(_In_ HWND WindowHandle, _In_ UINT Message, _In_ WPARAM wParam, _In_ LPARAM lParam)
{

    LRESULT Result = 0;

    switch (Message)
    {
        case WM_CLOSE:
        {
            PostQuitMessage(0);
            gGameIsRunning = FALSE;

            break;
        }
        default: 
        {
            Result = DefWindowProcA(WindowHandle, Message, wParam, lParam);
        }
    }

    return Result;
}

DWORD CreateMainGameWindow(void) {
    DWORD Result = ERROR_SUCCESS;

    WNDCLASSEXA WindowClass = { 0 }; // data structure

    // filling the WindowClass
    WindowClass.cbSize = sizeof(WNDCLASSEXA);
    WindowClass.style = 0;
    WindowClass.lpfnWndProc = MainWndowProc;
    WindowClass.cbClsExtra = 0;
    WindowClass.cbWndExtra = 0;
    WindowClass.hInstance = GetModuleHandleA(NULL); // give me the handle of myself, same as Instance
    WindowClass.hIcon = LoadIconA(NULL, IDI_APPLICATION);
    WindowClass.hIconSm = LoadIconA(NULL, IDI_APPLICATION);
    WindowClass.hCursor = LoadCursorA(NULL, IDC_ARROW);
    WindowClass.hbrBackground = CreateSolidBrush(RGB(255, 0, 255)); // changing background color
    WindowClass.lpszMenuName = NULL;
    WindowClass.lpszClassName = GAME_NAME  "_WINDOWCLASS";

    //SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    if (RegisterClassExA(&WindowClass) == 0)
    {
        Result = GetLastError();
        MessageBoxA(NULL, "Window Registration Failed!", "Error!",MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    gGameWindow = CreateWindowExA(0, WindowClass.lpszClassName, "Window Title", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 640,480, NULL, NULL, GetModuleHandleA(NULL), NULL);

    if (gGameWindow == NULL)
    {
        Result = GetLastError();
        MessageBoxA(NULL, "Window Creation Failed!", "Error!",
            MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    gPerformanceData.MonitorInfo.cbSize = sizeof(MONITORINFO);

    if (GetMonitorInfoA(MonitorFromWindow(gGameWindow, MONITOR_DEFAULTTOPRIMARY), &gPerformanceData.MonitorInfo) == 0) 
    {
        Result = ERROR_MONITOR_NO_DESCRIPTOR;

        goto Exit;
    }

    gPerformanceData.MonitorWidth = gPerformanceData.MonitorInfo.rcMonitor.right - gPerformanceData.MonitorInfo.rcMonitor.left;
    gPerformanceData.MonitorHeight = gPerformanceData.MonitorInfo.rcMonitor.bottom - gPerformanceData.MonitorInfo.rcMonitor.top;

    if (SetWindowLongPtrA(gGameWindow, GWL_STYLE, (WS_OVERLAPPEDWINDOW | WS_VISIBLE) & ~WS_OVERLAPPEDWINDOW) == 0)
    {
        Result = GetLastError();

        goto Exit;
    }// removing the title bar

    if (SetWindowPos(gGameWindow, HWND_TOP, gPerformanceData.MonitorInfo.rcMonitor.left, gPerformanceData.MonitorInfo.rcMonitor.top, gPerformanceData.MonitorWidth, gPerformanceData.MonitorHeight, SWP_NOOWNERZORDER | SWP_FRAMECHANGED) == 0)
    {
        Result = GetLastError();

        goto Exit;
    }

    Exit: return Result;
}

BOOL GameIsAlreadyRunning(void) {
    HANDLE Mutex = NULL;
    
    Mutex = CreateMutexA(NULL, FALSE, GAME_NAME "_GameMutex");// Mutex decides which thread can be open

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        return TRUE;
    }
    else {
        return FALSE;
    }
}

void ProcessPlayerInput(void)
{
    int16_t EscapeKeyIsDown = GetAsyncKeyState(VK_ESCAPE);

    if (EscapeKeyIsDown) 
    {
        SendMessageA(gGameWindow, WM_CLOSE, 0, 0);
    }
}

void RednerFrameGraphics(void)
{
    //memset(gBackBuffer.Memory, 0xFF, (GAME_RES_WIDTH * 4) * GAME_RES_HEIGHT);

    PIXEL32 Pixel = { 0 };

    Pixel.Blue = 0x7f;
    Pixel.Green = 0;
    Pixel.Red = 0;
    Pixel.Alpha = 0xFF;

    for (int i = 0; i < (GAME_RES_HEIGHT * GAME_RES_WIDTH); i++) 
    {
        memcpy_s((PIXEL32*)gBackBuffer.Memory  + i, sizeof(PIXEL32), &Pixel, sizeof(PIXEL32));
    }

    HDC DeviceContext = GetDC(gGameWindow);

    StretchDIBits(DeviceContext, 0, 0, gPerformanceData.MonitorWidth, gPerformanceData.MonitorHeight, 0, 0, GAME_RES_WIDTH, GAME_RES_HEIGHT, gBackBuffer.Memory, &gBackBuffer.Bitmapinfo, DIB_RGB_COLORS, SRCCOPY);

    ReleaseDC(gGameWindow, DeviceContext);
}