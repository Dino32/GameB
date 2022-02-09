#include <stdio.h>

#pragma warning(push, 3)

#include <windows.h>

#include <Psapi.h>

#pragma warning(pop)

#include <stdint.h>

#include <emmintrin.h>

#include <xinput.h>    // Xbox input library

#include <xaudio2.h>  // Audio library

#pragma comment(lib, "XAudio2.lib")

#pragma comment(lib, "XInput.lib")

#pragma comment(lib, "Winmm.lib")

#include "Main.h"

#include "Menus.h"

#pragma warning (disable: 4668)

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

REGISTRYPARAMS gRegistryParams;

XINPUT_STATE gGamepadState;

int8_t gGamepadID = -1;

GAMESTATE gCurrentGameState = GAMESTATE_TITLESCREEN;

GAMESTATE gPreviousGameState;

GAMESTATE gDesiredGameState;

GAMEINPUT gGameInput;

IXAudio2* gXAudio;

IXAudio2MasteringVoice* gXAudioMasteringVoice;

IXAudio2SourceVoice* gXAudioSFXSorceVoice[NUMBER_OF_SFX_SOURCE_VOICES];

IXAudio2SourceVoice* gXAudioMusicSourceVoice;

uint8_t gSFXSourceVoiceSelector;

float gSFXVolume = 0.5f;

float gMusicVolume = 0.5f;

GAMESOUND gMenuNavigate;

GAMESOUND gMenuChoose;


INT WINAPI WinMain(_In_ HINSTANCE Instance, _In_ HINSTANCE PreviousInstance,
    _In_ PSTR CommandLine, _In_ INT CommandShow) {

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


    if (LoadRegistryParameters() != ERROR_SUCCESS)
    {
        goto Exit;
    }
    
    LogMessageA(Informational, "[%s], Informational", __FUNCTION__);
    
    LogMessageA(Warning, "[%s] Warning", __FUNCTION__);

    LogMessageA(Error, "[%s], Error", __FUNCTION__);

    LogMessageA(Debug, "[%s] Debug", __FUNCTION__);

    if ((NtDllModuleHandle = GetModuleHandleA("ntdll.dll")) == NULL)
    {
        LogMessageA(Error, "[%s] Couldn't load ntdll.dll! Error 0x%08lx!", __FUNCTION__, GetLastError());
        MessageBoxA(NULL, "Couldn't load ntdll.dll", "Error!", MB_ICONERROR | MB_OK);
        goto Exit;
    }

    if ((NtQueryTimerResolution = (_NtQueryTimerResolution)GetProcAddress(NtDllModuleHandle, "NtQueryTimerResolution")) == NULL) 
    {
        LogMessageA(Error, "[%s] Couldn't find the NtQueryTimerResolution function in ntdll.dll! GetProcAddres failed! Error 0x%08lx", __FUNCTION__, GetLastError());
        MessageBoxA(NULL, "Couldn't find the NtQueryTimerResolution function in ntdll.dll", "Error!", MB_ICONERROR | MB_OK);
        goto Exit;
    }

    NtQueryTimerResolution(&gPerformanceData.MinimumTimerResolution, &gPerformanceData.MaximumTimerResolution, &gPerformanceData.CurrentTimerResolution);
    
    GetSystemInfo(&gPerformanceData.SystemInfo);

    GetSystemTimeAsFileTime((FILETIME*)&gPerformanceData.PreviousSystemTime);


    if (GameIsAlreadyRunning()) {
        LogMessageA(Error, "[%s] Another instance of this program is already running!", __FUNCTION__);
        MessageBoxA(NULL, "Another instance of this program is already running!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    if (timeBeginPeriod(1) == TIMERR_NOCANDO) 
    {
        LogMessageA(Error, "[%s] Failed to set global timer reoslution!", __FUNCTION__);

        MessageBoxA(NULL, "Failed to set global timer reoslution!", "Error!", MB_ICONERROR | MB_OK);

        goto Exit;
    }

    if (SetPriorityClass(ProcessHandle, HIGH_PRIORITY_CLASS) == 0) 
    {
        LogMessageA(Error, "[%s] Failed to set process priority! Error 0x%08lx", __FUNCTION__, GetLastError());

        MessageBoxA(NULL, "Failed to set process priority!", "Error!", MB_ICONERROR | MB_OK);

        goto Exit;
    }

    if (SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST) == 0)
    {
        LogMessageA(Error, "[%s] Failed to set thread priority! Error 0x%08lx", __FUNCTION__, GetLastError());

        MessageBoxA(NULL, "Failed to set thread priority!", "Error!", MB_ICONERROR | MB_OK);

        goto Exit;
    }

    if (CreateMainGameWindow() != ERROR_SUCCESS) {
        LogMessageA(Error, "[%s] CreateMainGameWindow Failed!", __FUNCTION__);

        goto Exit;
    }

    if ((Load32BppBitmapFromFile(".\\Assets\\6x7Font.bmpx", &g6x7Font)) != ERROR_SUCCESS)
    {
        LogMessageA(Error, "[%s] Load32BppBitmapFromFile failed!", __FUNCTION__);

        MessageBoxA(NULL, "Load32BppBitmapFromFile failed!", "Error!", MB_ICONERROR | MB_OK);

        goto Exit;
    }

    if (InitializeSoundEngine() != S_OK)
    {
        MessageBoxA(NULL, "InitializeSoundEngine failed!", "Error!", MB_ICONERROR | MB_OK);

        goto Exit;
    }

    if (LoadWaveFromFile(".\\Assets\\MenuSelect.wav", &gMenuNavigate) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "LoadWaveFromFile failed!", "Error!", MB_ICONERROR | MB_OK);

        goto Exit;
    }

    if (LoadWaveFromFile(".\\Assets\\MenuChooser.wav", &gMenuChoose) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "LoadWaveFromFile failed!", "Error!", MB_ICONERROR | MB_OK);

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
        LogMessageA(Error, "[%s] Faild to allocate memory for drawing surface! Error 0x%08lx", __FUNCTION__, GetLastError());

        MessageBoxA(NULL, "Faild to allocate memory for drawing surface!", "Error!", MB_ICONERROR | MB_OK);

        goto Exit;
    }

    if (InitializeHero() != ERROR_SUCCESS)
    {
        LogMessageA(Error, "[%s] Faild to initialize hero!" __FUNCTION__);

        MessageBoxA(NULL, "Faild to initialize hero!", "Error!", MB_ICONERROR | MB_OK);

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

            // Gamepad Input
            FindFirstConnctedGamepad();


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
    WindowClass.hbrBackground = CreateSolidBrush(RGB(0, 0, 0)); // changing background color
    WindowClass.lpszMenuName = NULL;
    WindowClass.lpszClassName = GAME_NAME  "_WINDOWCLASS";

    //SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    if (RegisterClassExA(&WindowClass) == 0)
    {
        Result = GetLastError();
        LogMessageA(Error, "[%s] RegisterClassExA failed! Error 0x%08lx", __FUNCTION__, GetLastError());
        MessageBoxA(NULL, "Window Registration Failed!", "Error!",MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    gGameWindow = CreateWindowExA(0, WindowClass.lpszClassName, "Window Title", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 640,480, NULL, NULL, GetModuleHandleA(NULL), NULL);

    if (gGameWindow == NULL)
    {
        Result = GetLastError();
        LogMessageA(Error, "[%s] CreateWindowEx failed! Error 0x%08lx", __FUNCTION__, GetLastError());
        MessageBoxA(NULL, "Window Creation Failed!", "Error!",
            MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    gPerformanceData.MonitorInfo.cbSize = sizeof(MONITORINFO);

    if (GetMonitorInfoA(MonitorFromWindow(gGameWindow, MONITOR_DEFAULTTOPRIMARY), &gPerformanceData.MonitorInfo) == 0) 
    {
        LogMessageA(Error, "[%s] GetMonitorInfoA(MonitorFromWindow()) failed! Error 0x%08lx", __FUNCTION__, GetLastError());

        Result = ERROR_MONITOR_NO_DESCRIPTOR;

        goto Exit;
    }

    gPerformanceData.MonitorWidth = gPerformanceData.MonitorInfo.rcMonitor.right - gPerformanceData.MonitorInfo.rcMonitor.left;
    gPerformanceData.MonitorHeight = gPerformanceData.MonitorInfo.rcMonitor.bottom - gPerformanceData.MonitorInfo.rcMonitor.top;

    if (SetWindowLongPtrA(gGameWindow, GWL_STYLE, (WS_OVERLAPPEDWINDOW | WS_VISIBLE) & ~WS_OVERLAPPEDWINDOW) == 0)
    {
        LogMessageA(Error, "[%s] SetWindowPtrA() failed! Error 0x%08lx", __FUNCTION__, GetLastError());

        Result = GetLastError();

        goto Exit;
    }// removing the title bar

    if (SetWindowPos(gGameWindow, HWND_TOP, gPerformanceData.MonitorInfo.rcMonitor.left, gPerformanceData.MonitorInfo.rcMonitor.top, gPerformanceData.MonitorWidth, gPerformanceData.MonitorHeight, SWP_NOOWNERZORDER | SWP_FRAMECHANGED) == 0)
    {
        LogMessageA(Error, "[%s] SetWindowPos() failed! Error 0x%08lx", __FUNCTION__, GetLastError());

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

    gGameInput.EscapeKeyIsDown = GetAsyncKeyState(VK_ESCAPE);

    gGameInput.DebugKeyIsDown = GetAsyncKeyState(VK_F1);

    gGameInput.LeftKeyIsDown = GetAsyncKeyState(VK_LEFT) | GetAsyncKeyState('A');

    gGameInput.RightKeyIsDown = GetAsyncKeyState(VK_RIGHT) | GetAsyncKeyState('D');

    gGameInput.UpKeyIsDown = GetAsyncKeyState('W') | GetAsyncKeyState(VK_UP);

    gGameInput.DownKeyIsDown = GetAsyncKeyState(VK_DOWN) | GetAsyncKeyState('S');
    
    gGameInput.ChooseKeyIsDown = GetAsyncKeyState(VK_RETURN);

    if (gGamepadID >= 0)
    {
        if (XInputGetState(gGamepadID, &gGamepadState) == ERROR_SUCCESS)
        {
            gGameInput.EscapeKeyIsDown |= gGamepadState.Gamepad.wButtons & XINPUT_GAMEPAD_BACK;

            gGameInput.LeftKeyIsDown |= gGamepadState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT;

            gGameInput.RightKeyIsDown |= gGamepadState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT;

            gGameInput.DownKeyIsDown |= gGamepadState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN;

            gGameInput.UpKeyIsDown |= gGamepadState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP;
        
            gGameInput.ChooseKeyIsDown |= gGamepadState.Gamepad.wButtons & XINPUT_GAMEPAD_A;
        }
    }

    switch (gCurrentGameState)
    {
        case GAMESTATE_OPENINGSPLASHSCREEN:
        {
            PPI_OpeningSplasheScreen();

            break;
        }
        case GAMESTATE_TITLESCREEN:
        {
            PPI_TitleScreen();

            break;
        }
        case GAMESTATE_OVERWORLD:
        {
            PPI_Overworld();
        
            break;
        }
        case GAMESTATE_EXITYESNOSCREEN:
        {
            
            PPI_ExitYesNo();

            break;
        }
        case GAMESTATE_CHARACTERNAMING:
        {
        
            break;
        }
        case GAMESTATE_BATTLE:
        {
        
            break;
        }
        case GAMESTATE_OPTIONSSCREEN:
        {
            
            break;
        }
        default:
        {
            ASSERT(FALSE, "Unknow game state!");
        }
    }


    gGameInput.DebugKeyWasDown = gGameInput.DebugKeyIsDown;

    gGameInput.LeftKeyWasDown = gGameInput.LeftKeyIsDown;

    gGameInput.RightKeyWasDown = gGameInput.RightKeyIsDown;

    gGameInput.UpKeyWasDown = gGameInput.UpKeyIsDown;

    gGameInput.DownKeyWasDown = gGameInput.DownKeyIsDown;

    gGameInput.ChooseKeyWasDown = gGameInput.ChooseKeyIsDown;
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

    if (Error == ERROR_SUCCESS)
    {
        LogMessageA(Informational, "[%s] Loading successful: %s", __FUNCTION__, FileName);
    }
    else 
    {
        LogMessageA(Error, "[%s] Loading failed: %s! Error 0x%08lx!", __FUNCTION__, FileName, Error);
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

    //PIXEL32 BackgroundPixel = { 0 };

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

void BlitStringToBuffer(_In_ char* String, _In_ GAMEBITMAP* FontSheet, _In_ PIXEL32 Color, _In_ uint16_t x, _In_ uint16_t y)
{
    uint16_t CharWidth = (uint16_t)FontSheet->Bitmapinfo.bmiHeader.biWidth / FONT_SHEET_CHARACTERS_PER_ROW;

    uint16_t CharHeight = (uint16_t)FontSheet->Bitmapinfo.bmiHeader.biHeight;

    uint16_t BytesPerCharacter = CharWidth * CharHeight * (FontSheet->Bitmapinfo.bmiHeader.biBitCount / 8);

    uint16_t StringLength = strlen(String);

    GAMEBITMAP StringBitmap = { 0 };

    StringBitmap.Bitmapinfo.bmiHeader.biBitCount = GAME_BPP;

    StringBitmap.Bitmapinfo.bmiHeader.biHeight = CharHeight;

    StringBitmap.Bitmapinfo.bmiHeader.biWidth = CharWidth * StringLength;

    StringBitmap.Bitmapinfo.bmiHeader.biPlanes = 1;

    StringBitmap.Bitmapinfo.bmiHeader.biCompression = BI_RGB;

    StringBitmap.Memory = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, BytesPerCharacter * StringLength);

    for (int Character = 0; Character < StringLength; Character++)
    {
        int StartingFontSheetPixel = 0;

        int FontSheetOffset = 0;

        int StringBitmapOffset = 0;
    
        PIXEL32 FontSheetPixel = { 0 };
        
        switch (String[Character])
        {
            case 'A':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth;

                break;
            }
            case 'B':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + CharWidth;

                break;
            }
            case 'C':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 2 * CharWidth;

                break;
            }
            case 'D':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 3 * CharWidth;

                break;
            }

            case 'E':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 4 * CharWidth;

                break;
            }

            case 'F':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 5 * CharWidth;

                break;
            }

            case 'G':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 6 * CharWidth;

                break;
            }

            case 'H':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 7 * CharWidth;

                break;
            }

            case 'I':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 8 * CharWidth;

                break;
            }

            case 'J':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 9 * CharWidth;

                break;
            }

            case 'K':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 10 * CharWidth;

                break;
            }

            case 'L':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 11 * CharWidth;

                break;
            }

            case 'M':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 12 * CharWidth;

                break;
            }

            case 'N':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 13 * CharWidth;

                break;
            }

            case 'O':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 14 * CharWidth;

                break;
            }

            case 'P':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 15 * CharWidth;

                break;
            }

            case 'Q':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 16 * CharWidth;

                break;
            }

            case 'R':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 17 * CharWidth;

                break;
            }

            case 'S':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 18 * CharWidth;

                break;
            }

            case 'T':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 19 * CharWidth;

                break;
            }

            case 'U':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 20 * CharWidth;

                break;
            }

            case 'V':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 21 * CharWidth;

                break;
            }

            case 'W':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 22 * CharWidth;

                break;
            }

            case 'X':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 23 * CharWidth;

                break;
            }

            case 'Y':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 24 * CharWidth;

                break;
            }

            case 'Z':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 25 * CharWidth;

                break;
            }

            case 'a':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 26 * CharWidth;

                break;
            }
            case 'b':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 27 * CharWidth;

                break;
            }
            case 'c':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 28 * CharWidth;

                break;
            }
            case 'd':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 29 * CharWidth;

                break;
            }

            case 'e':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 30 * CharWidth;

                break;
            }

            case 'f':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 31 * CharWidth;

                break;
            }

            case 'g':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 32 * CharWidth;

                break;
            }

            case 'h':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 33 * CharWidth;

                break;
            }

            case 'i':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 34 * CharWidth;

                break;
            }

            case 'j':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 35 * CharWidth;

                break;
            }

            case 'k':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 36 * CharWidth;

                break;
            }

            case 'l':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 37 * CharWidth;

                break;
            }

            case 'm':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 38 * CharWidth;

                break;
            }

            case 'n':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 39 * CharWidth;

                break;
            }

            case 'o':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 40 * CharWidth;

                break;
            }

            case 'p':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 41 * CharWidth;

                break;
            }

            case 'q':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 42 * CharWidth;

                break;
            }

            case 'r':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 43 * CharWidth;

                break;
            }

            case 's':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 44 * CharWidth;

                break;
            }

            case 't':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 45 * CharWidth;

                break;
            }

            case 'u':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 46 * CharWidth;

                break;
            }

            case 'v':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 47 * CharWidth;

                break;
            }

            case 'w':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 48 * CharWidth;

                break;
            }

            case 'x':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 49 * CharWidth;

                break;
            }

            case 'y':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 50 * CharWidth;

                break;
            }

            case 'z':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 51 * CharWidth;

                break;
            }

            case '0':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 52 * CharWidth;

                break;
            }

            case '1':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 53 * CharWidth;

                break;
            }

            case '2':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 54 * CharWidth;

                break;
            }

            case '3':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 55 * CharWidth;

                break;
            }

            case '4':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 56 * CharWidth;

                break;
            }

            case '5':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 57 * CharWidth;

                break;
            }

            case '6':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 58 * CharWidth;

                break;
            }

            case '7':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 59 * CharWidth;

                break;
            }

            case '8':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 60 * CharWidth;

                break;
            }

            case '9':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 61 * CharWidth;

                break;
            }

            case '`':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 62 * CharWidth;

                break;
            }
            case '~':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 63 * CharWidth;

                break;
            }
            case '!':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 64 * CharWidth;

                break;
            }
            case '@':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 65 * CharWidth;

                break;
            }
            case '#':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 66 * CharWidth;

                break;
            }
            case '$':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 67 * CharWidth;

                break;
            }
            case '%':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 68 * CharWidth;

                break;
            }
            case '^':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 69 * CharWidth;

                break;
            }
            case '&':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 70 * CharWidth;

                break;
            }
            case '*':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 71 * CharWidth;

                break;
            }
            case '(':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 72 * CharWidth;

                break;
            }
            case ')':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 73 * CharWidth;

                break;
            }
            case '-':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 74 * CharWidth;

                break;
            }
            case '=':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 75 * CharWidth;

                break;
            }
            case '_':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 76 * CharWidth;

                break;
            }
            case '+':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 77 * CharWidth;

                break;
            }
            case '\\':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 78 * CharWidth;

                break;
            }
            case '|':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 79 * CharWidth;

                break;
            }
            case '[':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 80 * CharWidth;

                break;
            }
            case ']':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 81 * CharWidth;

                break;
            }
            case '{':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 82 * CharWidth;

                break;
            }
            case '}':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 83 * CharWidth;

                break;
            }
            case ';':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 84 * CharWidth;

                break;
            }
            case '\xF':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 85 * CharWidth;

                break;
            }
            case ':':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 86 * CharWidth;

                break;
            }
            case '"':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 87 * CharWidth;

                break;
            }
            case ',':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 88 * CharWidth;

                break;
            }
            case '<':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 89 * CharWidth;

                break;
            }
            case '>':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 90 * CharWidth;

                break;
            }
            case '.':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 91 * CharWidth;

                break;
            }
            case '/':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 92 * CharWidth;

                break;
            }
            case '?':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 93 * CharWidth;

                break;
            }
            case ' ':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 94 * CharWidth;

                break;
            }
            case '\xbb':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 95 * CharWidth;

                break;
            }
            case '\xAB':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 96 * CharWidth;

                break;
            }
            case '\xf2':
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 97 * CharWidth;

                break;
            }

            default:
            {
                StartingFontSheetPixel = (FontSheet->Bitmapinfo.bmiHeader.biWidth * FontSheet->Bitmapinfo.bmiHeader.biHeight) - FontSheet->Bitmapinfo.bmiHeader.biWidth + 93 * CharWidth; // question mark
            }
        }

        for (int YPixel = 0; YPixel < CharHeight; YPixel++)
        {
            for (int XPixel = 0; XPixel < CharWidth; XPixel++)
            {
                FontSheetOffset = StartingFontSheetPixel + XPixel - (FontSheet->Bitmapinfo.bmiHeader.biWidth * YPixel);

                StringBitmapOffset = (Character * CharWidth) + ((StringBitmap.Bitmapinfo.bmiHeader.biWidth * StringBitmap.Bitmapinfo.bmiHeader.biHeight) - \
                    StringBitmap.Bitmapinfo.bmiHeader.biWidth) + XPixel - (StringBitmap.Bitmapinfo.bmiHeader.biWidth) * YPixel;
            
                memcpy_s(&FontSheetPixel, sizeof(PIXEL32), (PIXEL32*)FontSheet->Memory + FontSheetOffset, sizeof(PIXEL32));

                FontSheetPixel.Green = Color.Green;

                FontSheetPixel.Red = Color.Red;

                FontSheetPixel.Blue = Color.Blue;

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
    switch (gCurrentGameState)
    {
        case GAMESTATE_OPENINGSPLASHSCREEN:
        {
            DrawOpeningSplashScreen();

            break;
        }

        case GAMESTATE_TITLESCREEN:
        {
            DrawTitleScreen();

            break;
        }
        case GAMESTATE_CHARACTERNAMING:
        {
            
            break;
        }
        case GAMESTATE_OVERWORLD:
        {
            
            break;
        }
        case GAMESTATE_BATTLE:
        {
            
            break;
        }
        case GAMESTATE_EXITYESNOSCREEN:
        {
            DrawExitYesNoExitScreen();
            
            break;
        }
        case GAMESTATE_OPTIONSSCREEN:
        {
            
            break;
        }
        default:
        {
            ASSERT(FALSE, "Game state not implemented");
        }
    }

    __m128i QuadPixel = {0x7f, 0x00, 0x00, 0xff, 0x7f, 0x00, 0x00, 0xff, 0x7f, 0x00, 0x00, 0xff, 0x7f, 0x00, 0x00, 0xff };

//#ifdef SIMD
//    ClearScreen(&QuadPixel);
//#else
//    PIXEL32 Pixel = { 0x7f, 0x00, 0x00, 0xff };
//    ClearScreen(&Pixel);
//#endif
//
//
//    PIXEL32 White = {0xff, 0xff, 0xff, 0xff};
//
//    Blit32BppBitmapToBuffer(&gPlayer.Sprite[gPlayer.CurrentArmor][gPlayer.Direction + gPlayer.SpriteIndex], gPlayer.ScreenPosX, gPlayer.ScreenPosY);

    if (gPerformanceData.DisplayDegubInfo)
    {
        DrawDebugInfo();
    }

    HDC DeviceContext = GetDC(gGameWindow);

    StretchDIBits(DeviceContext, 0, 0, gPerformanceData.MonitorWidth, gPerformanceData.MonitorHeight, 0, 0, GAME_RES_WIDTH, GAME_RES_HEIGHT, gBackBuffer.Memory, &gBackBuffer.Bitmapinfo, DIB_RGB_COLORS, SRCCOPY);

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

DWORD LoadRegistryParameters(void)
{
    DWORD Result = ERROR_SUCCESS;

    HKEY RegKey = NULL;

    DWORD RegDisposition = 0;

    DWORD RegBytesRead = sizeof(DWORD);

    Result = RegCreateKeyExA(HKEY_CURRENT_USER, "SOFTWARE\\" GAME_NAME, 0, NULL, 0, KEY_READ, NULL, &RegKey, &RegDisposition);

    if (Result != ERROR_SUCCESS)
    {
        LogMessageA(Error, "[%s] RegCreateKey failed with error code 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    if (RegDisposition == REG_CREATED_NEW_KEY)
    {
        LogMessageA(Informational, "[%s] Registry key did not exist; created new key HKCU\\SOFTWARE\\%s.", __FUNCTION__, GAME_NAME);
    }
    else
    {
        LogMessageA(Informational, "[%s] Opened existing registry key HCKU\\SOFTWARE\\%s", __FUNCTION__, GAME_NAME);
    }

    Result = RegGetValueA(RegKey, NULL, "LogLevel", RRF_RT_DWORD, NULL, (BYTE*)&gRegistryParams.LogLevel, &RegBytesRead);

    if (Result != ERROR_SUCCESS)
    {
        if (Result == ERROR_FILE_NOT_FOUND)
        {
            Result = ERROR_SUCCESS;

            LogMessageA(Informational, "[%s] Registry value 'LogLevel' not found. Using default of 0. (LOG_LEVEL_NONE)", __FUNCTION__);

            gRegistryParams.LogLevel = None;
        }
        else
        {
            LogMessageA(Error, "[%s] Failed to read the 'LogLevel' registry value! Error 0x%08lx!", __FUNCTION__, Result);

            goto Exit;
        }
    }

    LogMessageA(Informational, "[% s] LogLevel is %d", __FUNCTION__, gRegistryParams.LogLevel);

Exit:

    if (RegKey)
    {
        RegCloseKey(RegKey);
    }

    return Result;
}


void LogMessageA(_In_ DWORD LogLevel, _In_ char* Message, _In_ ...) 
{
    size_t MessageLength = strlen(Message);

    SYSTEMTIME Time = { 0 };

    HANDLE LogFileHandle = INVALID_HANDLE_VALUE;

    DWORD EndOfFile = 0;

    DWORD NumberOfyBytesWritten = 0;

    char DateTimeString[96] = { 0 };

    char SeverityString[8] = { 0 };

    char FormattedString[4096] = { 0 };

    if (gRegistryParams.LogLevel < (DWORD)LogLevel)
    {
        return;
    }
    
    if (MessageLength < 1 || MessageLength > 4096)
    {
        ASSERT(FALSE, "Message was too long or too short!");

        return;
    }

    switch (LogLevel)
    {
    case (None):
        {
            return;
        }
        case(Informational):
        {
            strcpy_s(SeverityString, sizeof(SeverityString), "[INFO]");
        
            break;
        }
        case (Warning):
        {
            strcpy_s(SeverityString, sizeof(SeverityString), "[WARN]");
        
            break;
        }
        case (Error):
        {
            strcpy_s(SeverityString, sizeof(SeverityString), "[ERROR]");

            break;
        }
        case (Debug):
        {
            strcpy_s(SeverityString, sizeof(SeverityString), "[DEBUG]");
            
            break;
        }
        default:
        {
            ASSERT(FALSE, "Unrecognized LogLevel");
        }
    }

    GetLocalTime(&Time);

    va_list ArgPointer = NULL;

    va_start(ArgPointer, Message);

    _vsnprintf_s(FormattedString, sizeof(FormattedString), _TRUNCATE, Message, ArgPointer);

    va_end(ArgPointer);

    _snprintf_s(DateTimeString, sizeof(DateTimeString), _TRUNCATE, "\r\n[%02u/%02u/%u %02u:%02u:%02u.%03u]", Time.wMonth, Time.wDay, Time.wYear, Time.wHour, Time.wMinute, Time.wSecond, Time.wMilliseconds);

    if ((LogFileHandle = CreateFileA(LOG_FILE_NAME, FILE_APPEND_DATA, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
    {
        ASSERT(FALSE, "Failed to acces log file!");

        return;
    }

    EndOfFile = SetFilePointer(LogFileHandle, 0, NULL, FILE_END);

    WriteFile(LogFileHandle, DateTimeString, (DWORD)strlen(DateTimeString), &NumberOfyBytesWritten, NULL);

    WriteFile(LogFileHandle, SeverityString, (DWORD)strlen(SeverityString), &NumberOfyBytesWritten, NULL);

    WriteFile(LogFileHandle, FormattedString, (DWORD)strlen(FormattedString), &NumberOfyBytesWritten, NULL);
    
    if (LogFileHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(LogFileHandle);
    }
}

void DrawDebugInfo()
{
    if (gCurrentGameState != GAMESTATE_OVERWORLD)
    {
        return;
    }

    char DebugTextBuffer[64] = { 0 };

    PIXEL32 White = { 0xFF, 0xFF, 0xFF, 0xFF };

    sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "FPS Raw:     %.1f", gPerformanceData.RawFPSAverage); // _count of returns the number of characters, "places"

    BlitStringToBuffer(DebugTextBuffer, &g6x7Font, White, 0, 0);

    sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "FPS Coocked: %.1f", gPerformanceData.CoockedFPSAverage); // _count of returns the number of characters, "places"

    BlitStringToBuffer(DebugTextBuffer, &g6x7Font, White,  0, 8);

    sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "Min Timer:   %.1f", gPerformanceData.MinimumTimerResolution / 10000.f); // _count of returns the number of characters, "places"

    BlitStringToBuffer(DebugTextBuffer, &g6x7Font, White, 0, 16);

    sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "Max Timer:   %.1f", gPerformanceData.MaximumTimerResolution / 10000.f); // _count of returns the number of characters, "places"

    BlitStringToBuffer(DebugTextBuffer, &g6x7Font, White, 0, 24);

    sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "Cur Timer:   %.1f", gPerformanceData.CurrentTimerResolution / 10000.f); // _count of returns the number of characters, "places"

    BlitStringToBuffer(DebugTextBuffer, &g6x7Font, White, 0, 32);

    sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "Handles:     %lu", gPerformanceData.HandleCount); // _count of returns the number of characters, "places"

    BlitStringToBuffer(DebugTextBuffer, &g6x7Font, White, 0,40);

    sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "Memory:      %lu KB", gPerformanceData.MemInfo.PrivateUsage / 1024); // _count of returns the number of characters, "places"

    BlitStringToBuffer(DebugTextBuffer, &g6x7Font, White, 0, 48);

    sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "CPU:         %.3f %%", gPerformanceData.CPUPercent); // _count of returns the number of characters, "places"

    BlitStringToBuffer(DebugTextBuffer, &g6x7Font, White, 0, 56);

    sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "Total Frames:%llu", gPerformanceData.TotalFramesRednered); // _count of returns the number of characters, "places"

    BlitStringToBuffer(DebugTextBuffer, &g6x7Font, White, 0, 64);

    sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "Screen Pos:  (%d, %d)", gPlayer.ScreenPosX, gPlayer.ScreenPosY); // _count of returns the number of characters, "places"

    BlitStringToBuffer(DebugTextBuffer, &g6x7Font, White, 0, 72);
}

void FindFirstConnctedGamepad(void)
{
    gGamepadID = -1;

    for (int8_t GamepadIndex = 0; GamepadIndex < XUSER_MAX_COUNT && gGamepadID == -1; GamepadIndex++)
    {
        XINPUT_STATE State = { 0 };

        if (XInputGetState(GamepadIndex, &State) == ERROR_SUCCESS)
        {
            gGamepadID = GamepadIndex;
        }
    }
}

void MenuItem_TitleScreen_Resume(void)
{
    
}

void MenuItem_TitleScree_StartNew(void)
{

}

void MenuItem_TitleScree_Options(void)
{
    
}

void MenuItem_TitleScree_Exit(void)
{
    gPreviousGameState = gCurrentGameState;

    gCurrentGameState = GAMESTATE_EXITYESNOSCREEN;
}

void MenuItem_ExitYesNo_Yes(void)
{
    SendMessageA(gGameWindow, WM_CLOSE, 0, 0);
}

void MenuItem_ExitYesNo_No(void)
{
    gPreviousGameState = gCurrentGameState;

    gCurrentGameState = GAMESTATE_TITLESCREEN;
        
}

void DrawExitYesNoExitScreen()
{
    PIXEL32 White = { 0xff, 0xff, 0xff, 0xff};

    static uint64_t LocalFrameCounter;

    static uint64_t LastFrameSeen;

    memset(gBackBuffer.Memory, 0, GAME_DRAWING_AREA_MEMORY_SIZE);

    BlitStringToBuffer(gMenu_ExitYesNo.Name, &g6x7Font, White, (GAME_RES_WIDTH/2)-((uint16_t)strlen(gMenu_ExitYesNo.Name)*6)/2, 60);

    BlitStringToBuffer(gMenu_ExitYesNo.Items[0]->Name, &g6x7Font, White, (GAME_RES_WIDTH / 2) - ((uint16_t)strlen(gMenu_ExitYesNo.Items[0]->Name) * 6) / 2, 100);

    BlitStringToBuffer(gMenu_ExitYesNo.Items[1]->Name, &g6x7Font, White, (GAME_RES_WIDTH / 2) - ((uint16_t)strlen(gMenu_ExitYesNo.Items[1]->Name) * 6) / 2, 120);

    BlitStringToBuffer("\xbb", &g6x7Font, White, gMenu_ExitYesNo.Items[gMenu_ExitYesNo.SelectedItem]->x - 6, gMenu_ExitYesNo.Items[gMenu_ExitYesNo.SelectedItem]->y);
}

void DrawOpeningSplashScreen(void)
{

}

void DrawTitleScreen(void)
{
    PIXEL32 White = {0xff, 0xff, 0xff, 0xff};

    static uint64_t LocalFrameCounter;

    static uint64_t LastFrameSeen;

    memset(gBackBuffer.Memory, 0, GAME_DRAWING_AREA_MEMORY_SIZE);

    BlitStringToBuffer(GAME_NAME, &g6x7Font, White, (GAME_RES_WIDTH/2) - ((strlen(GAME_NAME) * 6)/2),  60);

    for (uint8_t MenuItem = 0; MenuItem < gMenu_TitleScreen.ItemCount; MenuItem++)
    {
        BlitStringToBuffer(gMenu_TitleScreen.Items[MenuItem]->Name, &g6x7Font, White, gMenu_TitleScreen.Items[MenuItem]->x, gMenu_TitleScreen.Items[MenuItem]->y);
    }

    BlitStringToBuffer("\xbb", &g6x7Font, White, gMenu_TitleScreen.Items[gMenu_TitleScreen.SelectedItem]->x - 6, gMenu_TitleScreen.Items[gMenu_TitleScreen.SelectedItem]->y);

}

void PPI_OpeningSplasheScreen(void)
{

}

void PPI_TitleScreen(void)
{
    //if (gGameInput.EscapeKeyIsDown)
    //{
    //  SendMessageA(gGameWindow, WM_CLOSE, 0, 0);
    //}

    if (gGameInput.DebugKeyIsDown && !gGameInput.DebugKeyWasDown)
    {
        gPerformanceData.DisplayDegubInfo = !gPerformanceData.DisplayDegubInfo;
    }

    if (gGameInput.DownKeyIsDown && !gGameInput.DownKeyWasDown)
    {
        if (gMenu_TitleScreen.SelectedItem < gMenu_TitleScreen.ItemCount -1)
        {
            gMenu_TitleScreen.SelectedItem++;

            PlayGameSound(&gMenuNavigate);
        }
    }

    if (gGameInput.UpKeyIsDown && !gGameInput.UpKeyWasDown)
    {
        if (gMenu_TitleScreen.SelectedItem > 0)
        {
            gMenu_TitleScreen.SelectedItem--;

            PlayGameSound(&gMenuNavigate);
        }
    }

    if (gGameInput.ChooseKeyIsDown && !gGameInput.ChooseKeyWasDown)
    {
        gMenu_TitleScreen.Items[gMenu_TitleScreen.SelectedItem]->Action();
    
        PlayGameSound(&gMenuChoose);
    }
}

void PPI_Overworld(void)
{
    if (gGameInput.EscapeKeyIsDown)
    {
        SendMessageA(gGameWindow, WM_CLOSE, 0, 0);
    }

    if (gGameInput.DebugKeyIsDown && !gGameInput.DebugKeyWasDown)
    {
        gPerformanceData.DisplayDegubInfo = !gPerformanceData.DisplayDegubInfo;
    }

    if (!gPlayer.MovementRemaining)
    {
        if (gGameInput.DownKeyIsDown)
        {
            if (gPlayer.ScreenPosY < GAME_RES_HEIGHT - 16)
            {
                gPlayer.Direction = DIRECTION_DOWN;

                gPlayer.MovementRemaining = 16;
            }
        }
        else if (gGameInput.LeftKeyIsDown)
        {
            if (gPlayer.ScreenPosX > 0)
            {
                gPlayer.Direction = DIRECTION_LEFT;

                gPlayer.MovementRemaining = 16;
            }
        }
        else if (gGameInput.RightKeyIsDown)
        {
            if (gPlayer.ScreenPosX < GAME_RES_WIDTH - 16)
            {
                gPlayer.Direction = DIRECTION_RIGTH;

                gPlayer.MovementRemaining = 16;
            }
        }
        else if (gGameInput.UpKeyIsDown)
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
}

void PPI_ExitYesNo(void)
{
    if (gGameInput.DebugKeyIsDown && !gGameInput.DebugKeyWasDown)
    {
        gPerformanceData.DisplayDegubInfo = !gPerformanceData.DisplayDegubInfo;
    }

    if (gGameInput.DownKeyIsDown && !gGameInput.DownKeyWasDown)
    {
        if (gMenu_ExitYesNo.SelectedItem < gMenu_ExitYesNo.ItemCount - 1)
        {
            gMenu_ExitYesNo.SelectedItem++;

            PlayGameSound(&gMenuNavigate);
        }
    }

    if (gGameInput.UpKeyIsDown && !gGameInput.UpKeyWasDown)
    {
        if (gMenu_ExitYesNo.SelectedItem > 0)
        {
            gMenu_ExitYesNo.SelectedItem--;

            PlayGameSound(&gMenuNavigate);
        }
    }

    if (gGameInput.ChooseKeyIsDown && !gGameInput.ChooseKeyWasDown)
    {
        gMenu_ExitYesNo.Items[gMenu_ExitYesNo.SelectedItem]->Action();

        PlayGameSound(&gMenuChoose);
    }
}

HRESULT InitializeSoundEngine(void)
{
    HRESULT Result = S_OK;

    WAVEFORMATEX SfxWaveFormat = { 0 };

    WAVEFORMATEX MusicWaveFormat = { 0 };

    Result = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    if (Result != S_OK)
    {
        LogMessageA(Error, "[%s] CoInitializeEx failed! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    Result = XAudio2Create(&gXAudio, 0, XAUDIO2_ANY_PROCESSOR);

    if (FAILED(Result))
    {
        LogMessageA(Error, "[%s] CoInitializeEx failed! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    Result = gXAudio->lpVtbl->CreateMasteringVoice(gXAudio, &gXAudioMasteringVoice, XAUDIO2_DEFAULT_CHANNELS, XAUDIO2_DEFAULT_SAMPLERATE, 0, 0, NULL, 0);

    if (FAILED(Result))
    {
        LogMessageA(Error, "[%s] CoInitializeEx failed! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    SfxWaveFormat.wFormatTag = WAVE_FORMAT_PCM;

    SfxWaveFormat.nChannels = 1;

    SfxWaveFormat.nSamplesPerSec = 44100;

    SfxWaveFormat.nAvgBytesPerSec = SfxWaveFormat.nSamplesPerSec * SfxWaveFormat.nChannels * 2;

    SfxWaveFormat.nBlockAlign = SfxWaveFormat.nChannels * 2;

    SfxWaveFormat.wBitsPerSample = 16;

    SfxWaveFormat.cbSize = 0x6164;

    for (uint8_t Counter = 0; Counter < NUMBER_OF_SFX_SOURCE_VOICES; Counter++)
    {
        Result = gXAudio->lpVtbl->CreateSourceVoice(gXAudio, &gXAudioSFXSorceVoice[Counter], &SfxWaveFormat, 0, XAUDIO2_DEFAULT_FREQ_RATIO, NULL, NULL, NULL);
    
        if (Result != S_OK)
        {
            LogMessageA(Error, "[%s] CoInitializeEx failed! Error 0x%08lx!", __FUNCTION__, Result);

            goto Exit;
        }

        gXAudioSFXSorceVoice[Counter]->lpVtbl->SetVolume(gXAudioSFXSorceVoice[Counter], gSFXVolume, XAUDIO2_COMMIT_NOW);
    }

    MusicWaveFormat.wFormatTag = WAVE_FORMAT_PCM;

    MusicWaveFormat.nChannels = 2;

    MusicWaveFormat.nSamplesPerSec = 44100;

    MusicWaveFormat.nAvgBytesPerSec = MusicWaveFormat.nSamplesPerSec * MusicWaveFormat.nChannels * 2;

    MusicWaveFormat.nBlockAlign = MusicWaveFormat.nChannels * 2;

    MusicWaveFormat.wBitsPerSample = 16;

    MusicWaveFormat.cbSize = 0;

    Result = gXAudio->lpVtbl->CreateSourceVoice(gXAudio, &gXAudioMusicSourceVoice, &MusicWaveFormat, 0, XAUDIO2_DEFAULT_FREQ_RATIO, NULL, NULL, NULL);
    
    if (Result != S_OK)
    {
        LogMessageA(Error, "[%s] CoInitializeEx failed! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    gXAudioMusicSourceVoice->lpVtbl->SetVolume(gXAudioMusicSourceVoice, gMusicVolume, XAUDIO2_COMMIT_NOW);

Exit:
    return Result;
}

DWORD LoadWaveFromFile(_In_ char* FileName, _Inout_ GAMESOUND* GameSound)
{
    DWORD Result = ERROR_SUCCESS;

    DWORD RIFF = 0;

    DWORD NumberOfBytesRead = 0;

    uint16_t DataChunkOffset = 0;

    DWORD DataChunkSearcher = 0;

    BOOL DataChunkFound = FALSE;
    
    DWORD DataChunkSize = 0;

    HANDLE FileHandle = INVALID_HANDLE_VALUE;

    void* AudioData = NULL;

    if ((FileHandle = CreateFileA(FileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
    {
        Result = GetLastError();

        LogMessageA(Error, "[%s] CreateFileA failed! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    if (ReadFile(FileHandle, &RIFF, sizeof(DWORD), &NumberOfBytesRead, NULL) == 0)
    {
        Result = GetLastError();

        LogMessageA(Error, "[%s] ReadFile failed! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    if (RIFF != 0x46464952) // "RIFF" backwards
    {
        Result = ERROR_FILE_INVALID;

        LogMessageA(Error, "[%s] First four bytes of this file are not RIFF! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    if (SetFilePointer(FileHandle, 20, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
    {
        Result = GetLastError();

        LogMessageA(Error, "[%s] SetFilePointer failed! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    if (ReadFile(FileHandle, &GameSound->WaveFormat, sizeof(WAVEFORMATEX), &NumberOfBytesRead, NULL) == 0)
    {
        Result = GetLastError();

        LogMessageA(Error, "[%s] CreateFileA failed! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    if (GameSound->WaveFormat.nBlockAlign != ((GameSound->WaveFormat.nChannels * GameSound->WaveFormat.wBitsPerSample) / 8) ||
        (GameSound->WaveFormat.wFormatTag != WAVE_FORMAT_PCM) || (GameSound->WaveFormat.wBitsPerSample != 16))
    {
        Result = ERROR_DATATYPE_MISMATCH;

        LogMessageA(Error, "[%s] This wave file did not meet the format requirements! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    while (DataChunkFound == FALSE)
    {
        if (SetFilePointer(FileHandle, DataChunkOffset, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
        {
            Result = GetLastError();

            LogMessageA(Error, "[%s] SetFilePointer failed! Error 0x%08lx!", __FUNCTION__, Result);

            goto Exit;
        }

        if (ReadFile(FileHandle, &DataChunkSearcher, sizeof(DWORD), &NumberOfBytesRead, NULL) == 0)
        {
            Result = GetLastError();

            LogMessageA(Error, "[%s] CreateFileA failed! Error 0x%08lx!", __FUNCTION__, Result);

            goto Exit;
        }

        if (DataChunkSearcher == 0x61746164) // 'data' backwards
        {
            DataChunkFound == TRUE;

            break;
        }
        else
        {
            DataChunkOffset += 4;
        }

        if (DataChunkOffset > 256)
        {
            Result = ERROR_DATATYPE_MISMATCH;

            LogMessageA(Error, "[%s] DataChunk not found! Error 0x%08lx!", __FUNCTION__, Result);

            goto Exit;
        }
    }

    if (SetFilePointer(FileHandle, DataChunkOffset + 4, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
    {
        Result = GetLastError();

        LogMessageA(Error, "[%s] SetFilePointer failed! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    if (ReadFile(FileHandle, &DataChunkSize, sizeof(DWORD), &NumberOfBytesRead, NULL) == 0)
    {
        Result = GetLastError();

        LogMessageA(Error, "[%s] CreateFileA failed! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    AudioData = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, DataChunkSize);
    
    if (AudioData == NULL)
    {
        Result = ERROR_NOT_ENOUGH_MEMORY;

        LogMessageA(Error, "[%s] HeapAlloc failed! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    GameSound->Buffer.Flags = XAUDIO2_END_OF_STREAM;

    GameSound->Buffer.AudioBytes = DataChunkSize;

    if (SetFilePointer(FileHandle, DataChunkOffset + 8, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
    {
        Result = GetLastError();

        LogMessageA(Error, "[%s] SetFilePointer failed! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    if (ReadFile(FileHandle, AudioData, DataChunkSize, &NumberOfBytesRead, NULL) == 0)
    {
        Result = GetLastError();

        LogMessageA(Error, "[%s] CreateFileA failed! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    GameSound->Buffer.pAudioData = AudioData;

Exit:
    if (Result == ERROR_SUCCESS)
    {
        LogMessageA(Informational, "[%s] Successfully loaded %s!", __FUNCTION__, FileName);
    }
    else
    {
        LogMessageA(Error, "[%s] Failed to load %s! Error 0x%08lx!", __FUNCTION__, FileName,Result);
    }

    if (FileHandle != NULL && FileHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(FileHandle);
    }

    return Result;
}

void PlayGameSound(_In_ GAMESOUND* GameSound)
{
    gXAudioSFXSorceVoice[gSFXSourceVoiceSelector]->lpVtbl->SubmitSourceBuffer(gXAudioSFXSorceVoice[gSFXSourceVoiceSelector], &GameSound->Buffer, NULL);

    gXAudioSFXSorceVoice[gSFXSourceVoiceSelector]->lpVtbl->Start(gXAudioSFXSorceVoice[gSFXSourceVoiceSelector], 0, XAUDIO2_COMMIT_NOW);

    gSFXSourceVoiceSelector++;

    if (gSFXSourceVoiceSelector > (NUMBER_OF_SFX_SOURCE_VOICES - 1))
    {
        gSFXSourceVoiceSelector = 0;
    }
}

