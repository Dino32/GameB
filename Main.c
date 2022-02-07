#include <stdio.h>

#pragma warning(push, 3)
// with pragma we are telling a compailer to behave in a centire way
// disable all warnings appliead to windows.h
#include <windows.h>
// pop out current level of warning (from  to level 4)

#include <Psapi.h>
#pragma warning(pop)
// its important that this is included after windows.h
#include <stdint.h>
#include <emmintrin.h>
#pragma comment(lib, "Winmm.lib")
#include "Main.h"
#pragma warning (disable: 4668)

#define GAME_NAME "GAMEB"
#define GAME_RES_WIDTH   384
#define GAME_RES_HEIGHT   240
#define GAME_BPP 32
#define GAME_DRAWING_AREA_MEMORY_SIZE (GAME_RES_WIDTH  * GAME_RES_HEIGHT * (GAME_BPP  / 8))
#define CALCULATE_AVG_FPS_EVERY_X_FRAMES 120
#define TARGET_MICROSECONDS_PER_FRAME 16667ULL
#define SIMD


BOOL gGameIsRunning; // gloabal variable, its automatically initialized to zero (false)

HWND gGameWindow;

GAMEBITMAP gBackBuffer;

GAMEPERFDATA gPerformanceData;

HERO gPlayer;

BOOL gWindowHasFocus;

GAMEBITMAP g6x7Font;


INT WINAPI WinMain(HINSTANCE Instance, HINSTANCE PreviousInstance,
    PSTR CommandLine, INT CommandShow) {

    UNREFERENCED_PARAMETER(PreviousInstance);
    UNREFERENCED_PARAMETER(CommandLine);
    UNREFERENCED_PARAMETER(Instance);

    MSG Message = { 0 };

    int64_t FrameStart = 0;

    int64_t FrameEnd = 0;

    int64_t ElapsedMicrosecondsPerFrameAccumulatorRaw = 0;

    int64_t ElapsedMicrosecondsPerFrame = 0;

    int64_t ElapsedMicrosecondsPerFrameAccumutalorCoocked = 0;

    HANDLE NtDllModuleHandle = NULL;

    FILETIME ProcessCreationTime = { 0 };

    FILETIME ProcessExitTime = { 0 };

    int64_t CurrentUserCPUTime = 0;

    int64_t CurrentKernelCPUTime = 0;

    int64_t PresviousUserCPUTime = 0;

    int64_t PreviousKernelCPUTime = 0;

    HANDLE ProcessHandle = GetCurrentProcess();

    if ((NtDllModuleHandle = GetModuleHandleA("ntdll.dll")) == NULL)
    {
        MessageBoxA(NULL, "Couldn't load ntdll.dll", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    if ((NtQueryTimerResolution = (_NtQueryTimerResolution)GetProcAddress(NtDllModuleHandle, "NtQueryTimerResolution")) == NULL) 
    {
        MessageBoxA(NULL, "Couldn't find the NtQueryTimerResolution function in ntdll.dll", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    NtQueryTimerResolution(&gPerformanceData.MinimumTimerResolution, &gPerformanceData.MaximumTimerResolution, &gPerformanceData.CurrentTimerResolution);
    
    GetSystemInfo(&gPerformanceData.SystemInfo);

    GetSystemTimeAsFileTime((FILETIME*)&gPerformanceData.PreviousSystemTime);


    if (GameIsAlreadyRunning()) {
        MessageBoxA(NULL, "Another instance of this program is already running!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    if (timeBeginPeriod(1) == TIMERR_NOCANDO) 
    {
        MessageBoxA(NULL, "Failed to set global timer reoslution!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto Exit;
    }

    if (SetPriorityClass(ProcessHandle, HIGH_PRIORITY_CLASS) == 0) 
    {
        MessageBoxA(NULL, "Failed to set process priority!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto Exit;
    }

    if (SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST) == 0)
    {
        MessageBoxA(NULL, "Failed to set thread priority!", "Error!", MB_ICONEXCLAMATION | MB_OK);


        goto Exit;
    }

    if (CreateMainGameWindow() != ERROR_SUCCESS) {
        goto Exit;
    }

    if ((Load32BppBitmapFromFile(".\\Assets\\6x7Font.bmpx", &g6x7Font)) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "Load32BppBitmapFromFile failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto Exit;
    }

    QueryPerformanceFrequency((LARGE_INTEGER*) & gPerformanceData.Perffrequency);

    gPerformanceData.DisplayDegubInfo = TRUE;

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

    if (InitializeHero() != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "Faild to initialize hero!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto Exit;
    }

    gGameIsRunning = TRUE;

    // while getMessage > 0 dispatche message

    while (gGameIsRunning)
    {
        QueryPerformanceCounter((LARGE_INTEGER*)&FrameStart);

        while (PeekMessageA(&Message, gGameWindow, 0, 0, PM_REMOVE)) 
        {
            DispatchMessageA(&Message);
        }

        ProcessPlayerInput();

        RednerFrameGraphics();

        QueryPerformanceCounter((LARGE_INTEGER*)&FrameEnd);

        ElapsedMicrosecondsPerFrame = FrameEnd - FrameStart;

        ElapsedMicrosecondsPerFrame *= 1000000;
        ElapsedMicrosecondsPerFrame /= gPerformanceData.Perffrequency;

        gPerformanceData.TotalFramesRednered++;

        ElapsedMicrosecondsPerFrameAccumulatorRaw += ElapsedMicrosecondsPerFrame;

        while (ElapsedMicrosecondsPerFrame < TARGET_MICROSECONDS_PER_FRAME) 
        {

            ElapsedMicrosecondsPerFrame = FrameEnd - FrameStart;

            ElapsedMicrosecondsPerFrame *= 1000000;

            ElapsedMicrosecondsPerFrame /= gPerformanceData.Perffrequency;
        
            QueryPerformanceCounter((LARGE_INTEGER*)&FrameEnd);
        
            if (ElapsedMicrosecondsPerFrame < TARGET_MICROSECONDS_PER_FRAME *0.75)
            {
                Sleep(1);
            }
        }

        ElapsedMicrosecondsPerFrameAccumutalorCoocked += ElapsedMicrosecondsPerFrame;


        if ((gPerformanceData.TotalFramesRednered % CALCULATE_AVG_FPS_EVERY_X_FRAMES) == 0)
        {
            GetSystemTimeAsFileTime((FILETIME*)&gPerformanceData.CurrentSystemTime);

            GetProcessTimes(ProcessHandle, &ProcessCreationTime, &ProcessExitTime, (FILETIME*)&CurrentKernelCPUTime, (FILETIME*)&CurrentUserCPUTime);

            gPerformanceData.CPUPercent = (CurrentKernelCPUTime - PreviousKernelCPUTime) + (CurrentUserCPUTime - PresviousUserCPUTime);

            gPerformanceData.CPUPercent /= (gPerformanceData.CurrentSystemTime - gPerformanceData.PreviousSystemTime);

            gPerformanceData.CPUPercent /= gPerformanceData.SystemInfo.dwNumberOfProcessors;

            gPerformanceData.CPUPercent *= 100;

            GetProcessHandleCount(ProcessHandle, &gPerformanceData.HandleCount);
            
            K32GetProcessMemoryInfo(ProcessHandle, (PROCESS_MEMORY_COUNTERS*)&gPerformanceData.MemInfo, sizeof(gPerformanceData.MemInfo));

            gPerformanceData.RawFPSAverage = 1.f / ((ElapsedMicrosecondsPerFrameAccumulatorRaw / CALCULATE_AVG_FPS_EVERY_X_FRAMES) * 0.000001f);

            gPerformanceData.CoockedFPSAverage = 1.f / ((ElapsedMicrosecondsPerFrameAccumutalorCoocked / CALCULATE_AVG_FPS_EVERY_X_FRAMES) * 0.000001f);

            ElapsedMicrosecondsPerFrameAccumulatorRaw = 0;
            ElapsedMicrosecondsPerFrameAccumutalorCoocked = 0;

            PreviousKernelCPUTime = CurrentKernelCPUTime;
            PresviousUserCPUTime = CurrentUserCPUTime;
            gPerformanceData.PreviousSystemTime = gPerformanceData.CurrentSystemTime;
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

        case WM_ACTIVATE:
        {
            ShowCursor(FALSE);

            if (wParam == 0)
            {
                // Our Window has lost focus

                gWindowHasFocus = FALSE;
            }
            else 
            {
                // Our window has gained focus

                gWindowHasFocus = TRUE;
            }

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
    if (gWindowHasFocus == FALSE)
    {
        return;
    }

    int16_t EscapeKeyIsDown = GetAsyncKeyState(VK_ESCAPE);

    int16_t DebugKeyIsDown = GetAsyncKeyState(VK_F1);

    int16_t LeftKeyIsDown = GetAsyncKeyState(VK_LEFT) | GetAsyncKeyState('A');

    int16_t RightKeyIsDown = GetAsyncKeyState(VK_RIGHT) | GetAsyncKeyState('D');

    int16_t UpKeyIsDown = GetAsyncKeyState('W') | GetAsyncKeyState(VK_UP);

    int16_t DownKeyIsDown = GetAsyncKeyState(VK_DOWN) | GetAsyncKeyState('S');

    static int16_t DebugKeyWasDown;

    static int16_t LeftKeyWasDown;

    static int16_t RightKeyWasDown;

    static int16_t UpKeyWasDown;

    static int16_t DownKeyWasDown;

    if (EscapeKeyIsDown) 
    {
        SendMessageA(gGameWindow, WM_CLOSE, 0, 0);
    }

    if (DebugKeyIsDown && !DebugKeyWasDown) 
    {
        gPerformanceData.DisplayDegubInfo = !gPerformanceData.DisplayDegubInfo;
    }

    if (!gPlayer.MovementRemaining)
    {
        if (DownKeyIsDown)
        {
            if (gPlayer.ScreenPosY < GAME_RES_HEIGHT - 16)
            {
                gPlayer.Direction = DIRECTION_DOWN;

                gPlayer.MovementRemaining = 16;
            }
        }
        else if (LeftKeyIsDown)
        {
            if (gPlayer.ScreenPosX > 0)
            {
                gPlayer.Direction = DIRECTION_LEFT;

                gPlayer.MovementRemaining = 16;
            }
        }
        else if (RightKeyIsDown)
        {
            if (gPlayer.ScreenPosX < GAME_RES_WIDTH - 16)
            {
                gPlayer.Direction = DIRECTION_RIGTH;

                gPlayer.MovementRemaining = 16;
            }
        }
        else if (UpKeyIsDown)
        {
            if (gPlayer.ScreenPosY > 0)
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
            gPlayer.ScreenPosY++;
        }
        else if (gPlayer.Direction == DIRECTION_LEFT)
        {
            gPlayer.ScreenPosX--;
        }
        else if (gPlayer.Direction == DIRECTION_RIGTH)
        {
            gPlayer.ScreenPosX++;
        }
        else if (gPlayer.Direction == DIRECTION_UP)
        {
            gPlayer.ScreenPosY--;
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


    DebugKeyWasDown = DebugKeyIsDown;

    LeftKeyWasDown = LeftKeyIsDown;

    RightKeyWasDown = RightKeyIsDown;

    UpKeyWasDown = UpKeyIsDown;

    DownKeyWasDown = DownKeyIsDown;
}

DWORD Load32BppBitmapFromFile(_In_ char* FileName, _Inout_ GAMEBITMAP* GameBitmap)
{
    DWORD Error = ERROR_SUCCESS;

    HANDLE FileHandle = INVALID_HANDLE_VALUE;

    WORD BitmapHeader = 0;

    DWORD PixelDataOffset = 0;

    DWORD NumberOfBytesRead = 2;

    if ((FileHandle = CreateFileA(FileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
    {
        Error = GetLastError();

        goto Exit;
    }

    if (ReadFile(FileHandle, &BitmapHeader, 2, &NumberOfBytesRead, NULL) == 0)
    {
        Error = GetLastError();

        goto Exit;
    }

    if (BitmapHeader != 0x4d42) // "BM" backwards
    {
        Error = ERROR_FILE_INVALID;

        goto Exit;
    }

    if (SetFilePointer(FileHandle, 0xA, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
    {
        Error = GetLastError();

        goto Exit;
    }

    if (ReadFile(FileHandle, &PixelDataOffset, sizeof(DWORD), &NumberOfBytesRead, NULL) == 0)
    {
        Error = GetLastError();

        goto Exit;
    }

    if (SetFilePointer(FileHandle, 0xE, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
    {
        Error = GetLastError();

        goto Exit;
    }

    if (ReadFile(FileHandle, &GameBitmap->Bitmapinfo.bmiHeader, sizeof(BITMAPINFOHEADER), &NumberOfBytesRead, NULL) == 0)
    {
        Error = GetLastError();

        goto Exit;
    }

    if ((GameBitmap->Memory = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, GameBitmap->Bitmapinfo.bmiHeader.biSizeImage)) == NULL)
    {
        Error = ERROR_NOT_ENOUGH_MEMORY;

        goto Exit;
    }

    if (SetFilePointer(FileHandle, PixelDataOffset, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
    {
        Error = GetLastError();

        goto Exit;
    }

    if (ReadFile(FileHandle, GameBitmap->Memory, GameBitmap->Bitmapinfo.bmiHeader.biSizeImage, &NumberOfBytesRead, NULL) == 0)
    {
        Error = GetLastError();

        goto Exit;
    }

Exit:

    if (FileHandle && (FileHandle != INVALID_HANDLE_VALUE))
    {
        CloseHandle(FileHandle);
    }

    return Error;
}


DWORD InitializeHero(void)
{
    DWORD Error = ERROR_SUCCESS;

    gPlayer.ScreenPosX = 32;

    gPlayer.ScreenPosY = 32;

    gPlayer.CurrentArmor = SUIT_0;

    gPlayer.Direction = DIRECTION_DOWN;

    gPlayer.MovementRemaining = 0;

    if ((Error = Load32BppBitmapFromFile(".\\Assets\\Hero_Suit0_Down_Standing.bmpx", &gPlayer.Sprite[SUIT_0][FACING_DOWN_0])) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "Load32BppBitmapFromFile failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto Exit;
    }

    if ((Error = Load32BppBitmapFromFile(".\\Assets\\Hero_Suit0_Down_Walk1.bmpx", &gPlayer.Sprite[SUIT_0][FACING_DOWN_1])) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "Load32BppBitmapFromFile failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto Exit;
    }

    if ((Error = Load32BppBitmapFromFile(".\\Assets\\Hero_Suit0_Down_Walk2.bmpx", &gPlayer.Sprite[SUIT_0][FACING_DOWN_2])) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "Load32BppBitmapFromFile failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto Exit;
    }

    if ((Error = Load32BppBitmapFromFile(".\\Assets\\Hero_Suit0_Left_Standing.bmpx", &gPlayer.Sprite[SUIT_0][FACING_LEFT_0])) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "Load32BppBitmapFromFile failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto Exit;
    }

    if ((Error = Load32BppBitmapFromFile(".\\Assets\\Hero_Suit0_Left_Walk1.bmpx", &gPlayer.Sprite[SUIT_0][FACING_LEFT_1])) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "Load32BppBitmapFromFile failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto Exit;
    }

    if ((Error = Load32BppBitmapFromFile(".\\Assets\\Hero_Suit0_Left_Walk2.bmpx", &gPlayer.Sprite[SUIT_0][FACING_LEFT_2])) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "Load32BppBitmapFromFile failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto Exit;
    }

    if ((Error = Load32BppBitmapFromFile(".\\Assets\\Hero_Suit0_Right_Standing.bmpx", &gPlayer.Sprite[SUIT_0][FACING_RIGHT_0])) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "Load32BppBitmapFromFile failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto Exit;
    }

    if ((Error = Load32BppBitmapFromFile(".\\Assets\\Hero_Suit0_Right_Walk1.bmpx", &gPlayer.Sprite[SUIT_0][FACING_RIGHT_1])) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "Load32BppBitmapFromFile failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto Exit;
    }

    if ((Error = Load32BppBitmapFromFile(".\\Assets\\Hero_Suit0_Right_Walk2.bmpx", &gPlayer.Sprite[SUIT_0][FACING_RIGHT_2])) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "Load32BppBitmapFromFile failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto Exit;
    }

    if ((Error = Load32BppBitmapFromFile(".\\Assets\\Hero_Suit0_Up_Standing.bmpx", &gPlayer.Sprite[SUIT_0][FACING_UPWARD_0])) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "Load32BppBitmapFromFile failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto Exit;
    }

    if ((Error = Load32BppBitmapFromFile(".\\Assets\\Hero_Suit0_Up_Walk1.bmpx", &gPlayer.Sprite[SUIT_0][FACING_UPWARD_1])) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "Load32BppBitmapFromFile failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto Exit;
    }

    if ((Error = Load32BppBitmapFromFile(".\\Assets\\Hero_Suit0_Up_Walk2.bmpx", &gPlayer.Sprite[SUIT_0][FACING_UPWARD_2])) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "Load32BppBitmapFromFile failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        goto Exit;
    }

Exit:
    return Error;
}

void Blit32BppBitmapToBuffer(_In_ GAMEBITMAP* GameBitmap, _In_ uint16_t x, _In_ uint16_t y)
{
    int32_t StartingScreenPixel = ((GAME_RES_WIDTH * GAME_RES_HEIGHT) - GAME_RES_WIDTH) - (GAME_RES_WIDTH * y) + x;

    int32_t StartingBitmapPixel = ((GameBitmap->Bitmapinfo.bmiHeader.biHeight * GameBitmap->Bitmapinfo.bmiHeader.biWidth) - GameBitmap->Bitmapinfo.bmiHeader.biWidth);


    int32_t MemoryOffset = 0;

    int32_t BitmapOffset = 0;

    PIXEL32 BitmapPixel = { 0 };

    PIXEL32 BackgroundPixel = { 0 };

    for (int16_t YPixel = 0; YPixel < (GameBitmap->Bitmapinfo.bmiHeader.biHeight); YPixel++)
    {
        for (int16_t XPixel = 0; XPixel < (GameBitmap->Bitmapinfo.bmiHeader.biWidth); XPixel++)
        {
            MemoryOffset = StartingScreenPixel + XPixel - (GAME_RES_WIDTH * YPixel);

            BitmapOffset = StartingBitmapPixel + XPixel - (GameBitmap->Bitmapinfo.bmiHeader.biWidth * YPixel);

            memcpy_s(&BitmapPixel, sizeof(PIXEL32), (PIXEL32*)GameBitmap->Memory + BitmapOffset, sizeof(PIXEL32));

            if (BitmapPixel.Alpha == 255)
            {
                memcpy_s((PIXEL32*)gBackBuffer.Memory + MemoryOffset, sizeof(PIXEL32), &BitmapPixel, sizeof(PIXEL32));
            }
        }
    }
}

void BlitStringToBuffer(_In_ char* String, _In_ GAMEBITMAP* GameBitmap, _In_ uint16_t x, _In_ uint16_t y)
{
    int CharWidth = GameBitmap->Bitmapinfo.bmiHeader.biWidth / FONT_SHEET_CHARACTERS_PER_ROW;

    int CharHeight = GameBitmap->Bitmapinfo.bmiHeader.biHeight;

    int BytesPerCharacter = CharWidth * CharHeight * (GameBitmap->Bitmapinfo.bmiHeader.biBitCount / 8);

    int StringLength = strlen(String);

    GAMEBITMAP StringBitmap = { 0 };

    StringBitmap.Bitmapinfo.bmiHeader.biBitCount = GAME_BPP;

    StringBitmap.Bitmapinfo.bmiHeader.biHeight = CharHeight;

    StringBitmap.Bitmapinfo.bmiHeader.biWidth = CharWidth * StringLength;

    StringBitmap.Bitmapinfo.bmiHeader.biPlanes = 1;

    StringBitmap.Bitmapinfo.bmiHeader.biCompression = BI_RGB;

    StringBitmap.Memory = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, BytesPerCharacter * StringLength);

    for (int Character = 0; Character < StringLength; Character++)
    {
        int StartingFontSheetByte = 0;

        int FontSheetOffset = 0;

        int StringBitmapOffset = 0;
    
        PIXEL32 FontSheetPixel = { 0 };

        switch (String[Character])
        {
            case 'A':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth;
                
                break;
            }
            case 'B':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + CharWidth;

                break;
            }
            case 'C':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 2*CharWidth;
            
                break;
            }
            case 'D':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 3 * CharWidth;

                break;
            }

            case 'E':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 4 * CharWidth;

                break;
            }

            case 'F':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 5 * CharWidth;

                break;
            }

            case 'G':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 6 * CharWidth;

                break;
            }

            case 'H':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 7 * CharWidth;

                break;
            }

            case 'I':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 8 * CharWidth;

                break;
            }

            case 'J':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 9 * CharWidth;

                break;
            }

            case 'K':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 10 * CharWidth;

                break;
            }

            case 'L':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 11 * CharWidth;

                break;
            }

            case 'M':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 12 * CharWidth;

                break;
            }

            case 'N':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 13 * CharWidth;

                break;
            }

            case 'O':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 14 * CharWidth;

                break;
            }

            case 'P':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 15 * CharWidth;

                break;
            }

            case 'Q':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 16 * CharWidth;

                break;
            }

            case 'R':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 17 * CharWidth;

                break;
            }

            case 'S':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 18 * CharWidth;

                break;
            }

            case 'T':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 19 * CharWidth;

                break;
            }

            case 'U':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 20 * CharWidth;

                break;
            }

            case 'V':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 21 * CharWidth;

                break;
            }

            case 'W':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 22 * CharWidth;

                break;
            }

            case 'X':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 23 * CharWidth;

                break;
            }

            case 'Y':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 24 * CharWidth;

                break;
            }

            case 'Z':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 25 * CharWidth;

                break;
            }

            case 'a':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 26 * CharWidth;

                break;
            }
            case 'b':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 27 * CharWidth;

                break;
            }
            case 'c':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 28 * CharWidth;

                break;
            }
            case 'd':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 29 * CharWidth;

                break;
            }

            case 'e':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 30 * CharWidth;

                break;
            }

            case 'f':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 31 * CharWidth;

                break;
            }

            case 'g':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 32 * CharWidth;

                break;
            }

            case 'h':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 33 * CharWidth;

                break;
            }

            case 'i':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 34 * CharWidth;

                break;
            }

            case 'j':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 35 * CharWidth;

                break;
            }

            case 'k':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 36 * CharWidth;

                break;
            }

            case 'l':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 37 * CharWidth;

                break;
            }

            case 'm':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 38 * CharWidth;

                break;
            }

            case 'n':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 39 * CharWidth;

                break;
            }

            case 'o':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 40 * CharWidth;

                break;
            }

            case 'p':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 41 * CharWidth;

                break;
            }

            case 'q':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 42 * CharWidth;

                break;
            }

            case 'r':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 43 * CharWidth;

                break;
            }

            case 's':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 44 * CharWidth;

                break;
            }

            case 't':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 45 * CharWidth;

                break;
            }

            case 'u':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 46 * CharWidth;

                break;
            }

            case 'v':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 47 * CharWidth;

                break;
            }

            case 'w':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 48 * CharWidth;

                break;
            }

            case 'x':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 49 * CharWidth;

                break;
            }

            case 'y':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 50 * CharWidth;

                break;
            }

            case 'z':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 51 * CharWidth;

                break;
            }

            case '0':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 52 * CharWidth;

                break;
            }

            case '1':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 53 * CharWidth;

                break;
            }

            case '2':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 54 * CharWidth;

                break;
            }

            case '3':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 55 * CharWidth;

                break;
            }

            case '4':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 56 * CharWidth;

                break;
            }

            case '5':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 57 * CharWidth;

                break;
            }

            case '6':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 58 * CharWidth;

                break;
            }

            case '7':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 59 * CharWidth;

                break;
            }

            case '8':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 60 * CharWidth;

                break;
            }

            case '9':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 61 * CharWidth;

                break;
            }

            case '`':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 62 * CharWidth;

                break;
            }
            case '~':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 63 * CharWidth;

                break;
            }
            case '!':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 64 * CharWidth;

                break;
            }
            case '@':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 65 * CharWidth;

                break;
            }
            case '#':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 66 * CharWidth;

                break;
            }
            case '$':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 67 * CharWidth;

                break;
            }
            case '%':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 68 * CharWidth;

                break;
            }
            case '^':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 69 * CharWidth;

                break;
            }
            case '&':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 70 * CharWidth;

                break;
            }
            case '*':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 71 * CharWidth;

                break;
            }
            case '(':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 72 * CharWidth;

                break;
            }
            case ')':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 73 * CharWidth;

                break;
            }
            case '-':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 74 * CharWidth;

                break;
            }
            case '=':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 75 * CharWidth;

                break;
            }
            case '_':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 76 * CharWidth;

                break;
            }
            case '+':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 77 * CharWidth;

                break;
            }
            case '\\':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 78 * CharWidth;

                break;
            }
            case '|':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 79 * CharWidth;

                break;
            }
            case '[':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 80 * CharWidth;

                break;
            }
            case ']':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 81 * CharWidth;

                break;
            }
            case '{':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 82 * CharWidth;

                break;
            }
            case '}':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 83 * CharWidth;

                break;
            }
            case ';':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 84 * CharWidth;

                break;
            }
            case '\xF':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 85 * CharWidth;

                break;
            }
            case ':':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 86 * CharWidth;

                break;
            }
            case '"':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 87 * CharWidth;

                break;
            }
            case ',':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 88 * CharWidth;

                break;
            }
            case '<':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 89 * CharWidth;

                break;
            }
            case '>':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 90 * CharWidth;

                break;
            }
            case '.':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 91 * CharWidth;

                break;
            }
            case '/':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 92 * CharWidth;

                break;
            }
            case '?':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 93 * CharWidth;

                break;
            }
            case ' ':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 94 * CharWidth;

                break;
            }
            case '\xbb':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 95 * CharWidth;

                break;
            }
            case '\xAB':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 96 * CharWidth;

                break;
            }
            case '\xf2':
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 97 * CharWidth;

                break;
            }

            default:
            {
                StartingFontSheetByte = (GameBitmap->Bitmapinfo.bmiHeader.biWidth * GameBitmap->Bitmapinfo.bmiHeader.biHeight) - GameBitmap->Bitmapinfo.bmiHeader.biWidth + 93 * CharWidth; // question mark
            }
        }

        for (int YPixel = 0; YPixel < CharHeight; YPixel++)
        {
            for (int XPixel = 0; XPixel < CharWidth; XPixel++)
            {
                FontSheetOffset = StartingFontSheetByte + XPixel - (GameBitmap->Bitmapinfo.bmiHeader.biWidth * YPixel);

                StringBitmapOffset = (Character * CharWidth) + ((StringBitmap.Bitmapinfo.bmiHeader.biWidth * StringBitmap.Bitmapinfo.bmiHeader.biHeight) - \
                    StringBitmap.Bitmapinfo.bmiHeader.biWidth) + XPixel - (StringBitmap.Bitmapinfo.bmiHeader.biWidth) * YPixel;
            
                memcpy_s(&FontSheetPixel, sizeof(PIXEL32), (PIXEL32*)GameBitmap->Memory + FontSheetOffset, sizeof(PIXEL32));

                FontSheetPixel.Green = 0x00;

                FontSheetPixel.Red = 0xff;

                FontSheetPixel.Blue = 0x00;

                memcpy_s((PIXEL32*)StringBitmap.Memory + StringBitmapOffset, sizeof(PIXEL32), &FontSheetPixel, sizeof(PIXEL32));
            }
        }
    }


    Blit32BppBitmapToBuffer(&StringBitmap, x, y);

    if (StringBitmap.Memory)
    {
        HeapFree(GetProcessHeap(), 0, StringBitmap.Memory);
    }
}

void RednerFrameGraphics(void)
{
    __m128i QuadPixel = {0x7f, 0x00, 0x00, 0xff, 0x7f, 0x00, 0x00, 0xff, 0x7f, 0x00, 0x00, 0xff, 0x7f, 0x00, 0x00, 0xff };

#ifdef SIMD
    ClearScreen(&QuadPixel);
#else
    PIXEL32 Pixel = { 0x7f, 0x00, 0x00, 0xff };
    ClearScreen(&Pixel);
#endif


    BlitStringToBuffer("GAME OVER", &g6x7Font, 60, 60);
    Blit32BppBitmapToBuffer(&gPlayer.Sprite[gPlayer.CurrentArmor][gPlayer.Direction + gPlayer.SpriteIndex], gPlayer.ScreenPosX, gPlayer.ScreenPosY);


    HDC DeviceContext = GetDC(gGameWindow);

    StretchDIBits(DeviceContext, 0, 0, gPerformanceData.MonitorWidth, gPerformanceData.MonitorHeight, 0, 0, GAME_RES_WIDTH, GAME_RES_HEIGHT, gBackBuffer.Memory, &gBackBuffer.Bitmapinfo, DIB_RGB_COLORS, SRCCOPY);

    if (gPerformanceData.DisplayDegubInfo)
    {
        SelectObject(DeviceContext, (HFONT)GetStockObject(ANSI_FIXED_FONT));

        char DebugTextBuffer[64] = { 0 };

        sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "FPS Raw:       %.1f", gPerformanceData.RawFPSAverage); // _count of returns the number of characters, "places"

        TextOutA(DeviceContext, 0, 0, DebugTextBuffer, (int)strlen(DebugTextBuffer));

        sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "FPS Coocked:    %.1f", gPerformanceData.CoockedFPSAverage); // _count of returns the number of characters, "places"

        TextOutA(DeviceContext, 0, 12, DebugTextBuffer, (int)strlen(DebugTextBuffer));

        sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "Min Timer Resolution: %.1f", gPerformanceData.MinimumTimerResolution / 10000.f); // _count of returns the number of characters, "places"

        TextOutA(DeviceContext, 0, 24, DebugTextBuffer, (int)strlen(DebugTextBuffer));

        sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "Max Timer Resolution:  %.1f", gPerformanceData.MaximumTimerResolution / 10000.f); // _count of returns the number of characters, "places"

        TextOutA(DeviceContext, 0, 36, DebugTextBuffer, (int)strlen(DebugTextBuffer));

        sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "Cur Timer Resolution:  %.1f", gPerformanceData.CurrentTimerResolution / 10000.f); // _count of returns the number of characters, "places"

        TextOutA(DeviceContext, 0, 48, DebugTextBuffer, (int)strlen(DebugTextBuffer));
    
        sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer),"Handles: %lu", gPerformanceData.HandleCount); // _count of returns the number of characters, "places"
   
        TextOutA(DeviceContext, 0, 60, DebugTextBuffer, (int)strlen(DebugTextBuffer));
    
        sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "Memory: %lu KB", gPerformanceData.MemInfo.PrivateUsage / 1024); // _count of returns the number of characters, "places"

        TextOutA(DeviceContext, 0, 72, DebugTextBuffer, (int)strlen(DebugTextBuffer));
    
        sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "CPU: %.3f %%", gPerformanceData.CPUPercent); // _count of returns the number of characters, "places"

        TextOutA(DeviceContext, 0, 84, DebugTextBuffer, (int)strlen(DebugTextBuffer));
    
        sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "Total Frames: %llu", gPerformanceData.TotalFramesRednered); // _count of returns the number of characters, "places"

        TextOutA(DeviceContext, 0, 96, DebugTextBuffer, (int)strlen(DebugTextBuffer));
    
        sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "Screen Pos: (%d, %d)", gPlayer.ScreenPosX, gPlayer.ScreenPosY); // _count of returns the number of characters, "places"

        TextOutA(DeviceContext, 0, 108, DebugTextBuffer, (int)strlen(DebugTextBuffer));
    }

    ReleaseDC(gGameWindow, DeviceContext);
}

#ifdef SIMD
void ClearScreen(_In_ __m128i* Color)
{
    for (int i = 0; i < (GAME_RES_HEIGHT * GAME_RES_WIDTH); i += 4)
    {
        _mm_store_si128((PIXEL32*)gBackBuffer.Memory + i, *Color);
    }
}
#else 
void ClearScreen(_In_ PIXEL32* Pixel)
{
    for (int i = 0; i < (GAME_RES_HEIGHT * GAME_RES_WIDTH); i++)
    {
        memcpy((PIXEL32*)gBackBuffer.Memory + i, Pixel, sizeof(PIXEL32));
    }
}
#endif

