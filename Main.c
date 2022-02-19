// Dino Babic
// TODO:
// Draw message dialogs that will lead user on what to do
// There still might be some bugs when choosing "Start New Game" or "Continue Game"

#include "Main.h" 

#include "miniz.h"

#include "stb_vorbis.h"

#include "Overworld.h"

#include "GameSaved.h"

#include "NewGamePrompt.h"


// This critical section is used to synchronize logging between multiple threads
CRITICAL_SECTION gLogCritSec;

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

    InitializeGlobals();

    //This critical section is used to synchronize access to the log file vis a vis
    // LogMessageA when used by mutiple threads simultaneously
    InitializeCriticalSectionAndSpinCount(&gLogCritSec, 0x400);

    // This event gets signalled after the most essential assets have been loaded 
    // "Essential" means the assets required to render the splash screen
    gEssentialAssetsLoadedEvent = CreateEventA(NULL, TRUE, FALSE, "gEssentialAssetsLoadedEvent");


    if (LoadRegistryParameters() != ERROR_SUCCESS)
    {
        goto Exit;
    }

    LogMessageA(Informational, "[%s] gPlayer.WorldPos.x is %d", __FUNCTION__, gPlayer.WorldPos.x);

    LogMessageA(Informational, "[%s] gPlayer.WorldPos.y is %d", __FUNCTION__, gPlayer.WorldPos.y);

    LogMessageA(Informational, "[%s] gPlayer.ScreenPos.x is %d", __FUNCTION__, gPlayer.ScreenPos.x);

    LogMessageA(Informational, "[%s] gPlayer.ScreenPos.x is %d", __FUNCTION__, gPlayer.ScreenPos.y);

    LogMessageA(Informational, "[%s] gCamera.x is %d", __FUNCTION__,gCamera.x);

    LogMessageA(Informational, "[%s] gCamera.x is %d", __FUNCTION__, gCamera.y);


    
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

    if ((gAssetLoadingThreadHandle = CreateThread(NULL, 0, AssetLoadingThreadProc, NULL, 0, NULL)) == NULL)
    {
        MessageBoxA(NULL, "CreateThread failed!", "Error!", MB_ICONERROR | MB_OK);

        goto Exit;
    }

    if (InitializeSoundEngine() != S_OK)
    {
        MessageBoxA(NULL, "InitializeSoundEngine failed!", "Error!", MB_ICONERROR | MB_OK);

        goto Exit;
    }

    QueryPerformanceFrequency((LARGE_INTEGER*) & gPerformanceData.Perffrequency);

    gPerformanceData.DisplayDegubInfo = FALSE;

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

    //gPerformanceData.MonitorSize.x = gPerformanceData.MonitorInfo.rcMonitor.right - gPerformanceData.MonitorInfo.rcMonitor.left;
    //gPerformanceData.MonitorHeight = gPerformanceData.MonitorInfo.rcMonitor.bottom - gPerformanceData.MonitorInfo.rcMonitor.top;

    /*if (gPerformanceData.WindowWidth == 0)
    {
        gPerformanceData.WindowWidth = gPerformanceData.MonitorWidth;
    }

    if (gPerformanceData.WindowHeight == 0)
    {
        gPerformanceData.WindowHeight = gPerformanceData.MonitorHeight;
    }*/

    /*if (gPerformanceData.WindowHeight > gPerformanceData.MonitorHeight ||
        gPerformanceData.WindowWidth > gPerformanceData.MonitorWidth)
    {
        LogMessageA(Error, "[%s] The WinodwWidth or WindowHeight retrieved from the registry eas larger from the current monitor size. Resetting", __FUNCTION__);
    
        gPerformanceData.WindowWidth = gPerformanceData.MonitorWidth;

        gPerformanceData.WindowHeight = gPerformanceData.MonitorHeight;
    }*/

    for (uint8_t Counter = 1; Counter < 12; Counter++)
    {
        if (GAME_RES_WIDTH * Counter > gPerformanceData.MonitorInfo.rcMonitor.right - gPerformanceData.MonitorInfo.rcMonitor.left ||
            GAME_RES_HEIGHT * Counter > (gPerformanceData.MonitorInfo.rcMonitor.bottom - gPerformanceData.MonitorInfo.rcMonitor.top))
        {
            gPerformanceData.MaxScaleFactor = Counter - 1;

            break;
        }
    }

    if (gRegistryParams.ScaleFactor == 0)
    {
        gPerformanceData.CurrentScaleFactor = gPerformanceData.MaxScaleFactor;
    }
    else 
    {
        gPerformanceData.CurrentScaleFactor = gRegistryParams.ScaleFactor;
    }
    if (SetWindowLongPtrA(gGameWindow, GWL_STYLE, (WS_OVERLAPPEDWINDOW | WS_VISIBLE) & ~WS_OVERLAPPEDWINDOW) == 0)
    {
        LogMessageA(Error, "[%s] SetWindowPtrA() failed! Error 0x%08lx", __FUNCTION__, GetLastError());

        Result = GetLastError();

        goto Exit;
    }// removing the title bar

   

   if (SetWindowPos(gGameWindow, HWND_TOP, gPerformanceData.MonitorInfo.rcMonitor.left, gPerformanceData.MonitorInfo.rcMonitor.top, gPerformanceData.MonitorInfo.rcMonitor.right - gPerformanceData.MonitorInfo.rcMonitor.left , gPerformanceData.MonitorInfo.rcMonitor.bottom - gPerformanceData.MonitorInfo.rcMonitor.top, SWP_NOOWNERZORDER | SWP_FRAMECHANGED) == 0)
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
    if (gInputEnabled == FALSE || gWindowHasFocus == FALSE)
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

    gGameInput.PortalIsDown = GetAsyncKeyState('P');

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
        else 
        {
            // Gamepad Unplugged

            gGamepadID = -1;

            gPreviousGameState = gCurrentGameState;
        
            gCurrentGameState = GAMESTATE_GAMEPADUNPLUGGED;

            gGameInput.EscapeKeyWasDown = gGameInput.EscapeKeyIsDown;
        }
    }

    if (gGameInput.DebugKeyIsDown && !gGameInput.DebugKeyWasDown)
    {
        gPerformanceData.DisplayDegubInfo = !gPerformanceData.DisplayDegubInfo;
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
            PPI_CharacterNaming();
        
            break;
        }
        case GAMESTATE_BATTLE:
        {
        
            break;
        }
        case GAMESTATE_OPTIONSSCREEN:
        {
            PPI_OptionsScreen();
            
            break;
        }
        case GAMESTATE_GAMEPADUNPLUGGED:
        {
            PPI_GamepadUnplugged();
            
            break;
        }
        case GAMESTATE_SAVEGAME:
        {
            PPI_GameSavedScreen();

            break;
        }
        case GAMESTATE_NEWGAMEPROMPT:
        {
            PPI_NewGame();

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

    gGameInput.EscapeKeyWasDown = gGameInput.EscapeKeyIsDown;

    gGameInput.PortalWasDown = gGameInput.PortalIsDown;
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

    /*gPlayer.ScreenPos.x = 64;

    gPlayer.ScreenPos.y = 80;

    gPlayer.WorldPos.x = 64;

    gPlayer.WorldPos.y = 80;

    gPlayer.CurrentArmor = SUIT_0;

    gPlayer.Direction = DIRECTION_DOWN;

    gPlayer.MovementRemaining = 0;

    gCamera.x = 0;

    gCamera.y = 0;*/

    //gPlayer.WorldPos.x = 4032;


Exit:
    return Error;
}

void Blit32BppBitmapToBuffer(_In_ GAMEBITMAP* GameBitmap, _In_ int16_t x, _In_ int16_t y, _In_ int16_t BrightnessAdjustment)
{
    int32_t StartingScreenPixel = ((GAME_RES_WIDTH * GAME_RES_HEIGHT) - GAME_RES_WIDTH) - (GAME_RES_WIDTH * y) + x;

    int32_t StartingBitmapPixel = ((GameBitmap->Bitmapinfo.bmiHeader.biHeight * GameBitmap->Bitmapinfo.bmiHeader.biWidth) - GameBitmap->Bitmapinfo.bmiHeader.biWidth);

    int32_t MemoryOffset = 0;

    int32_t BitmapOffset = 0;

    PIXEL32 BitmapPixel = { 0 };

#ifdef AVX

    __m256i BitmapOctoPixel;

    for (int16_t YPixel = 0; YPixel < GameBitmap->Bitmapinfo.bmiHeader.biHeight; YPixel++)
    {
        int16_t PixelsRemainingOnThisRow = GameBitmap->Bitmapinfo.bmiHeader.biWidth;

        int16_t XPixel = 0;

        while (PixelsRemainingOnThisRow >= 8)
        {
            MemoryOffset = StartingScreenPixel + XPixel - (GAME_RES_WIDTH * YPixel);

            BitmapOffset = StartingBitmapPixel + XPixel - (GameBitmap->Bitmapinfo.bmiHeader.biWidth * YPixel);

            BitmapOctoPixel = _mm256_load_si256((const __m256i*)((PIXEL32*)GameBitmap->Memory + BitmapOffset));
            
            __m256i Half1 = _mm256_cvtepu8_epi16(_mm256_extracti128_si256(BitmapOctoPixel, 0));

            Half1 = _mm256_add_epi16(Half1, _mm256_set_epi16(
                0, BrightnessAdjustment, BrightnessAdjustment, BrightnessAdjustment,
                0, BrightnessAdjustment, BrightnessAdjustment, BrightnessAdjustment,
                0, BrightnessAdjustment, BrightnessAdjustment, BrightnessAdjustment,
                0, BrightnessAdjustment, BrightnessAdjustment, BrightnessAdjustment));

            __m256i Half2 = _mm256_cvtepu8_epi16(_mm256_extracti128_si256(BitmapOctoPixel, 1));

            Half2 = _mm256_add_epi16(Half2, _mm256_set_epi16(
                0, BrightnessAdjustment, BrightnessAdjustment, BrightnessAdjustment,
                0, BrightnessAdjustment, BrightnessAdjustment, BrightnessAdjustment,
                0, BrightnessAdjustment, BrightnessAdjustment, BrightnessAdjustment,
                0, BrightnessAdjustment, BrightnessAdjustment, BrightnessAdjustment));

            __m256i Recombined = _mm256_packus_epi16(Half1, Half2);

            BitmapOctoPixel = _mm256_permute4x64_epi64(Recombined, _MM_SHUFFLE(3, 1, 2, 0));

            __m256i Mask = _mm256_cmpeq_epi8(BitmapOctoPixel, _mm256_set1_epi32(0xFF000000));

            _mm256_maskstore_epi32((__m256i*)((PIXEL32*)gBackBuffer.Memory + MemoryOffset), Mask, BitmapOctoPixel);

            // Store the result to the global back buffer.
            //_mm256_store_si256((__m256i*)((PIXEL32*)gBackBuffer.Memory + MemoryOffset), BitmapOctoPixel);

            PixelsRemainingOnThisRow -= 8;

            XPixel += 8;
        }

        while (PixelsRemainingOnThisRow > 0)
        {
            MemoryOffset = StartingScreenPixel + XPixel - (GAME_RES_WIDTH * YPixel);

            BitmapOffset = StartingBitmapPixel + XPixel - (GameBitmap->Bitmapinfo.bmiHeader.biWidth * YPixel);

            memcpy_s(&BitmapPixel, sizeof(PIXEL32), (PIXEL32*)GameBitmap->Memory + BitmapOffset, sizeof(PIXEL32));

            if (BitmapPixel.Alpha == 255)
            {
                BitmapPixel.Red = min(255, max((BitmapPixel.Red + BrightnessAdjustment), 0));

                BitmapPixel.Blue = min(255, max((BitmapPixel.Blue + BrightnessAdjustment), 0));

                BitmapPixel.Green = min(255, max((BitmapPixel.Green + BrightnessAdjustment), 0));

                memcpy_s((PIXEL32*)gBackBuffer.Memory + MemoryOffset, sizeof(PIXEL32), &BitmapPixel, sizeof(PIXEL32));
            }

            PixelsRemainingOnThisRow--;

            XPixel++;
        }
    }

#else

    for (int16_t YPixel = 0; YPixel < (GameBitmap->Bitmapinfo.bmiHeader.biHeight); YPixel++)
    {
        for (int16_t XPixel = 0; XPixel < (GameBitmap->Bitmapinfo.bmiHeader.biWidth); XPixel++)
        { 
            MemoryOffset = StartingScreenPixel + XPixel - (GAME_RES_WIDTH * YPixel);

            BitmapOffset = StartingBitmapPixel + XPixel - (GameBitmap->Bitmapinfo.bmiHeader.biWidth * YPixel);

            memcpy_s(&BitmapPixel, sizeof(PIXEL32), (PIXEL32*)GameBitmap->Memory + BitmapOffset, sizeof(PIXEL32));

            if (BitmapPixel.Alpha == 255)
            {
                BitmapPixel.Red = min(255, max((BitmapPixel.Red + BrightnessAdjustment), 0));

                BitmapPixel.Blue = min(255, max((BitmapPixel.Blue + BrightnessAdjustment), 0));

                BitmapPixel.Green = min(255, max((BitmapPixel.Green + BrightnessAdjustment), 0));

                memcpy_s((PIXEL32*)gBackBuffer.Memory + MemoryOffset, sizeof(PIXEL32), &BitmapPixel, sizeof(PIXEL32));
            }
        }
    }

#endif
}

void BlitTileMapToBuffer(_In_ GAMEBITMAP* GameBitmap, _In_ int16_t BrightnessAdjustment)
{
    int32_t StartingScreenPixel = ((GAME_RES_WIDTH * GAME_RES_HEIGHT) - GAME_RES_WIDTH);

    int32_t StartingBitmapPixel = ((GameBitmap->Bitmapinfo.bmiHeader.biHeight * GameBitmap->Bitmapinfo.bmiHeader.biWidth) - GameBitmap->Bitmapinfo.bmiHeader.biWidth) + gCamera.x - (GameBitmap->Bitmapinfo.bmiHeader.biWidth * gCamera.y);

    int32_t MemoryOffset = 0;

    int32_t BitmapOffset = 0;

#ifdef AVX
    __m256i BitmapOctoPixel;

    for (int16_t YPixel = 0; YPixel < GAME_RES_HEIGHT; YPixel++)
    {
        for (int16_t XPixel = 0; XPixel < GAME_RES_WIDTH; XPixel += 8)
        {
            MemoryOffset = StartingScreenPixel + XPixel - (GAME_RES_WIDTH * YPixel);

            BitmapOffset = StartingBitmapPixel + XPixel - (GameBitmap->Bitmapinfo.bmiHeader.biWidth * YPixel);

            // Load 256 bits (8 pixels) from memory into register YMMx
            BitmapOctoPixel = _mm256_load_si256((const __m256i*)((PIXEL32*)GameBitmap->Memory + BitmapOffset));
            //        AARRGGBBAARRGGBB-AARRGGBBAARRGGBB-AARRGGBBAARRGGBB-AARRGGBBAARRGGBB
            // YMM0 = FF5B6EE1FF5B6EE1-FF5B6EE1FF5B6EE1-FF5B6EE1FF5B6EE1-FF5B6EE1FF5B6EE1

            // Blow the 256-bit vector apart into two separate 256-bit vectors Half1 and Half2, 
            // each containing 4 pixels, where each pixel is now 16 bits instead of 8.            

            __m256i Half1 = _mm256_cvtepu8_epi16(_mm256_extracti128_si256(BitmapOctoPixel, 0));
            //        AAAARRRRGGGGBBBB-AAAARRRRGGGGBBBB-AAAARRRRGGGGBBBB-AAAARRRRGGGGBBBB
            // YMM0 = 00FF005B006E00E1-00FF005B006E00E1-00FF005B006E00E1-00FF005B006E00E1

            // Add the brightness adjustment to each 16-bit element
            Half1 = _mm256_add_epi16(Half1, _mm256_set1_epi16(BrightnessAdjustment));
            // YMM0 = 0000FF5CFF6FFFE2-0000FF5CFF6FFFE2-0000FF5CFF6FFFE2-0000FF5CFF6FFFE2

            // Do the same for Half2 that we just did for Half1.
            __m256i Half2 = _mm256_cvtepu8_epi16(_mm256_extracti128_si256(BitmapOctoPixel, 1));

            Half2 = _mm256_add_epi16(Half2, _mm256_set1_epi16(BrightnessAdjustment));

            // Now we need to reassemble the two halves back into a single 256-bit group of 8 pixels.
            // _mm256_packus_epi16(a,b) takes the 16-bit signed integers in the 256-bit vectors a and b
            // and converts them to a 256-bit vector of 8-bit unsigned integers. The result contains the
            // first 8 integers from a, followed by the first 8 integers from b, followed by the last 8
            // integers from a, followed by the last 8 integers from b.
            // Values that are out of range are set to 0 or 255.
            __m256i Recombined = _mm256_packus_epi16(Half1, Half2);

            BitmapOctoPixel = _mm256_permute4x64_epi64(Recombined, _MM_SHUFFLE(3, 1, 2, 0));

            // Store the result to the global back buffer.
            _mm256_store_si256((__m256i*)((PIXEL32*)gBackBuffer.Memory + MemoryOffset), BitmapOctoPixel);
        }
    }
#else
    PIXEL32 BitmapPixel = { 0 };

    for (int16_t YPixel = 0; YPixel < GAME_RES_HEIGHT; YPixel++)
    {
        for (int16_t XPixel = 0; XPixel < GAME_RES_WIDTH; XPixel++)
        {
            MemoryOffset = StartingScreenPixel + XPixel - (GAME_RES_WIDTH * YPixel);

            BitmapOffset = StartingBitmapPixel + XPixel - (GameBitmap->Bitmapinfo.bmiHeader.biWidth * YPixel);

            memcpy_s(&BitmapPixel, sizeof(PIXEL32), (PIXEL32*)GameBitmap->Memory + BitmapOffset, sizeof(PIXEL32));

            BitmapPixel.Red = min(255, max((BitmapPixel.Red + BrightnessAdjustment), 0));

            BitmapPixel.Blue = min(255, max((BitmapPixel.Blue + BrightnessAdjustment), 0));

            BitmapPixel.Green = min(255, max((BitmapPixel.Green + BrightnessAdjustment), 0));

            memcpy_s((PIXEL32*)gBackBuffer.Memory + MemoryOffset, sizeof(PIXEL32), &BitmapPixel, sizeof(PIXEL32));

        }
    }

#endif
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


    Blit32BppBitmapToBuffer(&StringBitmap, x, y, 0);

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
        case GAMESTATE_GAMEPADUNPLUGGED:
        {
            DrawGamepadUnplugged();

            break;
        }
        case GAMESTATE_TITLESCREEN:
        {
            DrawTitleScreen();

            break;
        }
        case GAMESTATE_CHARACTERNAMING:
        {
            DrawCharacterNaming();
            
            break;
        }
        case GAMESTATE_OVERWORLD:
        {
            DrawOverworld();

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
            DrawOptionsScreen();
            
            break;
        }
        case GAMESTATE_SAVEGAME:
        {
            DrawGameSavedScreen();

            break;
        }
        case GAMESTATE_NEWGAMEPROMPT:
        {
            DrawNewGamePrompt();

            break;
        }
        default:
        {
            ASSERT(FALSE, "Game state not implemented");
        }
    }

    //__m128i QuadPixel = {0x7f, 0x00, 0x00, 0xff, 0x7f, 0x00, 0x00, 0xff, 0x7f, 0x00, 0x00, 0xff, 0x7f, 0x00, 0x00, 0xff };

//#ifdef SIMD
//    ClearScreen(&QuadPixel);
//#else
//    PIXEL32 Pixel = { 0x7f, 0x00, 0x00, 0xff };
//    ClearScreen(&Pixel);
//#endif
//
//

   
    if (gPerformanceData.DisplayDegubInfo)
    {
        DrawDebugInfo();
    }

    HDC DeviceContext = GetDC(gGameWindow);

    StretchDIBits(DeviceContext, ((gPerformanceData.MonitorInfo.rcMonitor.right - gPerformanceData.MonitorInfo.rcMonitor.left) / 2) - (GAME_RES_WIDTH * gPerformanceData.CurrentScaleFactor) /2, ((gPerformanceData.MonitorInfo.rcMonitor.bottom - gPerformanceData.MonitorInfo.rcMonitor.top) / 2) - ((GAME_RES_HEIGHT * gPerformanceData.CurrentScaleFactor) / 2), GAME_RES_WIDTH * gPerformanceData.CurrentScaleFactor, GAME_RES_HEIGHT * gPerformanceData.CurrentScaleFactor, 0, 0, GAME_RES_WIDTH, GAME_RES_HEIGHT, gBackBuffer.Memory, &gBackBuffer.Bitmapinfo, DIB_RGB_COLORS, SRCCOPY);

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

    Result = RegCreateKeyExA(HKEY_CURRENT_USER, "SOFTWARE\\GAMEB", 0, NULL, 0, KEY_ALL_ACCESS, NULL, &RegKey, &RegDisposition);

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

            LogMessageA(Informational, "[%s] Registry value 'LogLevel' not found. Using default of 0. (LOG_LEVEL)", __FUNCTION__);

            gRegistryParams.LogLevel = 0;
        }
        else
        {
            LogMessageA(Error, "[%s] Failed to read the 'LogLevel' registry value! Error 0x%08lx!", __FUNCTION__, Result);

            goto Exit;
        }
    }

    LogMessageA(Informational, "[% s] LogLevel is %d", __FUNCTION__, gRegistryParams.LogLevel);


    Result = RegGetValueA(RegKey, NULL, "ScaleFactor", RRF_RT_DWORD, NULL, (BYTE*)&gRegistryParams.ScaleFactor, &RegBytesRead);

    if (Result != ERROR_SUCCESS)
    {
        if (Result == ERROR_FILE_NOT_FOUND)
        {
            Result = ERROR_SUCCESS;

            LogMessageA(Informational, "[%s] Registry value 'ScaleFactor' not found. Using default of 0.", __FUNCTION__);

            gRegistryParams.ScaleFactor = 0;
        }
        else
        {
            LogMessageA(Error, "[%s] Failed to read the 'ScaleFactor' registry value! Error 0x%08lx!", __FUNCTION__, Result);

            goto Exit;
        }
    }

    LogMessageA(Informational, "[% s] ScaleFactor is %d", __FUNCTION__, gRegistryParams.ScaleFactor);

    Result = RegGetValueA(RegKey, NULL, "SFXVolume", RRF_RT_DWORD, NULL, (BYTE*)&gRegistryParams.SFXVolume, &RegBytesRead);

    if (Result != ERROR_SUCCESS)
    {
        if (Result == ERROR_FILE_NOT_FOUND)
        {
            Result = ERROR_SUCCESS;

            LogMessageA(Informational, "[%s] Registry value 'SFXVolume' not found. Using default of 0.5", __FUNCTION__);

            gRegistryParams.SFXVolume = 50;
        }
        else
        {
            LogMessageA(Error, "[%s] Failed to read the 'SFXVolume' registry value! Error 0x%08lx!", __FUNCTION__, Result);

            goto Exit;
        }
    }

    LogMessageA(Informational, "[% s] SFXVolume is %.1f.", __FUNCTION__, (float)gRegistryParams.SFXVolume / 100);

    Result = RegGetValueA(RegKey, NULL, "MusicVolume", RRF_RT_DWORD, NULL, (BYTE*)&gRegistryParams.MusicVolume, &RegBytesRead);

    if (Result != ERROR_SUCCESS)
    {
        if (Result == ERROR_FILE_NOT_FOUND)
        {
            Result = ERROR_SUCCESS;

            LogMessageA(Informational, "[%s] Registry value 'MusicVolume' not found. Using default of 0.5", __FUNCTION__);

            gRegistryParams.MusicVolume = 50;
        }
        else
        {
            LogMessageA(Error, "[%s] Failed to read the 'MusicVolume' registry value! Error 0x%08lx!", __FUNCTION__, Result);

            goto Exit;
        }
    }

    LogMessageA(Informational, "[%s] MusicVolume is %.1f.", __FUNCTION__, (float)gRegistryParams.MusicVolume / 100);

    gMusicVolume = (float)gRegistryParams.MusicVolume / 100.f;

    gSFXVolume = (float)gRegistryParams.SFXVolume / 100.f;

    Result = RegGetValueA(RegKey, NULL, "gPlayer.WorldPos.x", RRF_RT_DWORD, NULL, (BYTE*)&gRegistryParams.WorldX, &RegBytesRead);

    if (Result != ERROR_SUCCESS)
    {
        if (Result == ERROR_FILE_NOT_FOUND)
        {
            Result = ERROR_SUCCESS;

            LogMessageA(Informational, "[%s] Registry value 'gPlayer.WorldPos.x' not found. Using default of 64", __FUNCTION__);

            gRegistryParams.WorldX = 64;
        }
        else
        {
            LogMessageA(Error, "[%s] Failed to read the 'gPlayer.WorldPos.x' registry value! Error 0x%08lx!", __FUNCTION__, Result);

            goto Exit;
        }
    }

    gPlayer.WorldPos.x = gRegistryParams.WorldX;

    LogMessageA(Informational, "[%s] gPlayer.WorldPos.x is %d", __FUNCTION__, gPlayer.WorldPos.x);

    Result = RegGetValueA(RegKey, NULL, "gPlayer.ScreenPos.y", RRF_RT_DWORD, NULL, (BYTE*)&gRegistryParams.ScreenY, &RegBytesRead);

    if (Result != ERROR_SUCCESS)
    {
        if (Result == ERROR_FILE_NOT_FOUND)
        {
            Result = ERROR_SUCCESS;

            LogMessageA(Informational, "[%s] Registry value 'gPlayer.ScreenPos.y' not found. Using default of 80", __FUNCTION__);

            gRegistryParams.ScreenY = 80;
        }
        else
        {
            LogMessageA(Error, "[%s] Failed to read the 'gPlayer.ScreenPos.y' registry value! Error 0x%08lx!", __FUNCTION__, Result);

            goto Exit;
        }
    }

    gPlayer.ScreenPos.y = gRegistryParams.ScreenY;

    LogMessageA(Informational, "[%s] gPlayer.ScreenPos.y is %d", __FUNCTION__, gPlayer.ScreenPos.y);

    Result = RegGetValueA(RegKey, NULL, "gPlayer.WorldPos.y", RRF_RT_DWORD, NULL, (BYTE*)&gRegistryParams.WorldY, &RegBytesRead);

    if (Result != ERROR_SUCCESS)
    {
        if (Result == ERROR_FILE_NOT_FOUND)
        {
            Result = ERROR_SUCCESS;

            LogMessageA(Informational, "[%s] Registry value 'gPlayer.WorldPos.y' not found. Using default of 80", __FUNCTION__);

            gRegistryParams.WorldY = 80;
        }
        else
        {
            LogMessageA(Error, "[%s] Failed to read the 'gPlayer.WorldPos.y' registry value! Error 0x%08lx!", __FUNCTION__, Result);

            goto Exit;
        }
    }
    gPlayer.WorldPos.y = gRegistryParams.WorldY;

    LogMessageA(Informational, "[%s] gPlayer.WorldPos.y is %d", __FUNCTION__, gPlayer.WorldPos.y);

    Result = RegGetValueA(RegKey, NULL, "gPlayer.ScreenPos.x", RRF_RT_DWORD, NULL, (BYTE*)&gRegistryParams.ScreenX, &RegBytesRead);

    if (Result != ERROR_SUCCESS)
    {
        if (Result == ERROR_FILE_NOT_FOUND)
        {
            Result = ERROR_SUCCESS;

            LogMessageA(Informational, "[%s] Registry value 'gPlayer.ScreenPos.x' not found. Using default of 64", __FUNCTION__);

            gRegistryParams.ScreenX = 64;
        }
        else
        {
            LogMessageA(Error, "[%s] Failed to read the 'gPlayer.ScreenPos.x' registry value! Error 0x%08lx!", __FUNCTION__, Result);

            goto Exit;
        }
    }

    gPlayer.ScreenPos.x = gRegistryParams.ScreenX;

    LogMessageA(Informational, "[%s] gPlayer.ScreenPos.x is %d", __FUNCTION__, gPlayer.ScreenPos.x);

    Result = RegGetValueA(RegKey, NULL, "gCamera.x", RRF_RT_DWORD, NULL, (BYTE*)&gRegistryParams.CameraX, &RegBytesRead);

    if (Result != ERROR_SUCCESS)
    {
        if (Result == ERROR_FILE_NOT_FOUND)
        {
            Result = ERROR_SUCCESS;

            LogMessageA(Informational, "[%s] Registry value 'gCamera.x' not found. Using default of 0", __FUNCTION__);

            gRegistryParams.CameraX = 0;
        }
        else
        {
            LogMessageA(Error, "[%s] Failed to read the 'gCamera.x' registry value! Error 0x%08lx!", __FUNCTION__, Result);

            goto Exit;
        }
    }

    gCamera.x = gRegistryParams.CameraX;

    LogMessageA(Informational, "[%s] gCamera.x is %d", __FUNCTION__, gCamera.x);

    if (gCamera.x < gOverwrldArea.right - 64)
    {
        gCurrentArea = gOverwrldArea;
    }
    else if (gCamera.x >= gDungeon1Area.left)
    {
        gCurrentArea = gDungeon1Area;
    }

    Result = RegGetValueA(RegKey, NULL, "gCamera.y", RRF_RT_DWORD, NULL, (BYTE*)&gRegistryParams.CameraY, &RegBytesRead);

    if (Result != ERROR_SUCCESS)
    {
        if (Result == ERROR_FILE_NOT_FOUND)
        {
            Result = ERROR_SUCCESS;

            LogMessageA(Informational, "[%s] Registry value 'gCamera.y' not found. Using default of 0", __FUNCTION__);

            gRegistryParams.CameraY = 0;
        }
        else
        {
            LogMessageA(Error, "[%s] Failed to read the 'gCamera.y' registry value! Error 0x%08lx!", __FUNCTION__, Result);

            goto Exit;
        }
    }

    gCamera.y = gRegistryParams.CameraY;

    LogMessageA(Informational, "[%s] gCamera.y is %d", __FUNCTION__, gCamera.y);

    Result = RegGetValueA(RegKey, NULL, "gPlayer.Direction", RRF_RT_DWORD, NULL, (BYTE*)&gRegistryParams.Direction, &RegBytesRead);

    if (Result != ERROR_SUCCESS)
    {
        if (Result == ERROR_FILE_NOT_FOUND)
        {
            Result = ERROR_SUCCESS;

            LogMessageA(Informational, "[%s] Registry value 'gPlayer.Direction' not found. Using default of DIRECTION_DOWN", __FUNCTION__);

            gRegistryParams.Direction = DIRECTION_DOWN;
        }
        else
        {
            LogMessageA(Error, "[%s] Failed to read the 'gPlayer.Direction' registry value! Error 0x%08lx!", __FUNCTION__, Result);

            goto Exit;
        }
    }

    gPlayer.Direction = gRegistryParams.Direction;

    LogMessageA(Informational, "[%s] gPlayer.Direction is %d", __FUNCTION__, gPlayer.Direction);

    Result = RegGetValueA(RegKey, NULL, "gPlayer.CurrentArmor", RRF_RT_DWORD, NULL, (BYTE*)&gRegistryParams.CurrentArmor, &RegBytesRead);

    if (Result != ERROR_SUCCESS)
    {
        if (Result == ERROR_FILE_NOT_FOUND)
        {
            Result = ERROR_SUCCESS;

            LogMessageA(Informational, "[%s] Registry value 'gPlayer.CurrentArmor' not found. Using default of SUIT_0", __FUNCTION__);

            gRegistryParams.CurrentArmor = SUIT_0;
        }
        else
        {
            LogMessageA(Error, "[%s] Failed to read the 'gPlayer.CurrentArmor' registry value! Error 0x%08lx!", __FUNCTION__, Result);

            goto Exit;
        }
    }

    gPlayer.CurrentArmor = gRegistryParams.CurrentArmor;

    LogMessageA(Informational, "[%s] gPlayer.CurrentArmor is %d", __FUNCTION__, gPlayer.CurrentArmor);

    Result = RegGetValueA(RegKey, NULL, "gPlayer.MovementRemaining", RRF_RT_DWORD, NULL, (BYTE*)&gRegistryParams.MovementsRemaining, &RegBytesRead);

    if (Result != ERROR_SUCCESS)
    {
        if (Result == ERROR_FILE_NOT_FOUND)
        {
            Result = ERROR_SUCCESS;

            LogMessageA(Informational, "[%s] Registry value 'gPlayer.MovementRemaining' not found. Using default of 0", __FUNCTION__);

            gRegistryParams.MovementsRemaining = 0;
        }
        else
        {
            LogMessageA(Error, "[%s] Failed to read the 'gPlayer.MovementRemaining' registry value! Error 0x%08lx!", __FUNCTION__, Result);

            goto Exit;
        }
    }

    gPlayer.MovementRemaining = gRegistryParams.MovementsRemaining;

    LogMessageA(Informational, "[%s] gPlayer.MovementRemaining is %d", __FUNCTION__, gPlayer.MovementRemaining);

    Result = RegGetValueA(RegKey, NULL, "gRegistryParams.GameSaved", RRF_RT_DWORD, NULL, (BYTE*)&gRegistryParams.GameSaved, &RegBytesRead);

    if (Result != ERROR_SUCCESS)
    {
        if (Result == ERROR_FILE_NOT_FOUND)
        {
            Result = ERROR_SUCCESS;

            LogMessageA(Informational, "[%s] Registry value 'gRegistryParams.GameSaved' not found. Using default of 0", __FUNCTION__);

            gRegistryParams.GameSaved = 0;
        }
        else
        {
            LogMessageA(Error, "[%s] Failed to read the 'gRegistryParams.GameSaved' registry value! Error 0x%08lx!", __FUNCTION__, Result);

            goto Exit;
        }
    }

    gPlayer.MovementRemaining = gRegistryParams.MovementsRemaining;

    LogMessageA(Informational, "[%s] gRegistryParams.GameSaved is %d", __FUNCTION__, gRegistryParams.GameSaved);

    Result = RegGetValueA(RegKey, NULL, "gRegistryParams.Name[0]", RRF_RT_REG_SZ, NULL, (void*)&gRegistryParams.Name[0], &RegBytesRead);

    if (Result != ERROR_SUCCESS)
    {
        if (Result == ERROR_FILE_NOT_FOUND)
        {
            Result = ERROR_SUCCESS;

            LogMessageA(Informational, "[%s] Registry value 'gRegistryParams.Name[0]' not found. Using default of 0", __FUNCTION__);

            gRegistryParams.Name[0] = '\0';
        }
        else
        {
            LogMessageA(Error, "[%s] Failed to read the 'gRegistryParams.Name[0]' registry value! Error 0x%08lx!", __FUNCTION__, Result);

            goto Exit;
        }
    }

    gPlayer.Name[0] = gRegistryParams.Name[0];

    LogMessageA(Informational, "[%s] gRegistryParams.Name[0] is %c", __FUNCTION__, gPlayer.Name[0]);

    Result = RegGetValueA(RegKey, NULL, "gRegistryParams.Name[1]", RRF_RT_REG_SZ, NULL, (void*)&gRegistryParams.Name[1], &RegBytesRead);

    if (Result != ERROR_SUCCESS)
    {
        if (Result == ERROR_FILE_NOT_FOUND)
        {
            Result = ERROR_SUCCESS;

            LogMessageA(Informational, "[%s] Registry value 'gRegistryParams.Name[1]' not found. Using default of 0", __FUNCTION__);

            gRegistryParams.Name[1] = '\0';
        }
        else
        {
            LogMessageA(Error, "[%s] Failed to read the 'gRegistryParams.Name[1]' registry value! Error 0x%08lx!", __FUNCTION__, Result);

            goto Exit;
        }
    }

    gPlayer.Name[1] = gRegistryParams.Name[1];

    LogMessageA(Informational, "[%s] gRegistryParams.Name[1] is %c", __FUNCTION__, gPlayer.Name[1]);

    Result = RegGetValueA(RegKey, NULL, "gRegistryParams.Name[2]", RRF_RT_REG_SZ, NULL, (void*)&gRegistryParams.Name[2], &RegBytesRead);

    if (Result != ERROR_SUCCESS)
    {
        if (Result == ERROR_FILE_NOT_FOUND)
        {
            Result = ERROR_SUCCESS;

            LogMessageA(Informational, "[%s] Registry value 'gRegistryParams.Name[2]' not found. Using default of 0", __FUNCTION__);

            gRegistryParams.Name[2] = '\0';
        }
        else
        {
            LogMessageA(Error, "[%s] Failed to read the 'gRegistryParams.Name[2]' registry value! Error 0x%08lx!", __FUNCTION__, Result);

            goto Exit;
        }
    }

    gPlayer.Name[2] = gRegistryParams.Name[2];

    LogMessageA(Informational, "[%s] gRegistryParams.Name[2] is %c", __FUNCTION__, gPlayer.Name[2]);

    Result = RegGetValueA(RegKey, NULL, "gRegistryParams.Name[3]", RRF_RT_REG_SZ, NULL, (void*)&gRegistryParams.Name[3], &RegBytesRead);

    if (Result != ERROR_SUCCESS)
    {
        if (Result == ERROR_FILE_NOT_FOUND)
        {
            Result = ERROR_SUCCESS;

            LogMessageA(Informational, "[%s] Registry value 'gRegistryParams.Name[3]' not found. Using default of 0", __FUNCTION__);

            gRegistryParams.Name[3] = '\0';
        }
        else
        {
            LogMessageA(Error, "[%s] Failed to read the 'gRegistryParams.Name[3]' registry value! Error 0x%08lx!", __FUNCTION__, Result);

            goto Exit;
        }
    }

    gPlayer.Name[3] = gRegistryParams.Name[3];

    LogMessageA(Informational, "[%s] gRegistryParams.Name[3] is %c", __FUNCTION__, gPlayer.Name[3]);

    Result = RegGetValueA(RegKey, NULL, "gRegistryParams.Name[4]", RRF_RT_REG_SZ, NULL, (void*)&gRegistryParams.Name[4], &RegBytesRead);

    if (Result != ERROR_SUCCESS)
    {
        if (Result == ERROR_FILE_NOT_FOUND)
        {
            Result = ERROR_SUCCESS;

            LogMessageA(Informational, "[%s] Registry value 'gRegistryParams.Name[4]' not found. Using default of 0", __FUNCTION__);

            gRegistryParams.Name[4] = '\0';
        }
        else
        {
            LogMessageA(Error, "[%s] Failed to read the 'gRegistryParams.Name[4]' registry value! Error 0x%08lx!", __FUNCTION__, Result);

            goto Exit;
        }
    }

    gPlayer.Name[4] = gRegistryParams.Name[4];

    LogMessageA(Informational, "[%s] gRegistryParams.Name[4] is %c", __FUNCTION__, gPlayer.Name[4]);

    Result = RegGetValueA(RegKey, NULL, "gRegistryParams.Name[5]", RRF_RT_REG_SZ, NULL, (void*)&gRegistryParams.Name[5], &RegBytesRead);

    if (Result != ERROR_SUCCESS)
    {
        if (Result == ERROR_FILE_NOT_FOUND)
        {
            Result = ERROR_SUCCESS;

            LogMessageA(Informational, "[%s] Registry value 'gRegistryParams.Name[5]' not found. Using default of 0", __FUNCTION__);

            gRegistryParams.Name[5] = '\0';
        }
        else
        {
            LogMessageA(Error, "[%s] Failed to read the 'gRegistryParams.Name[5]' registry value! Error 0x%08lx!", __FUNCTION__, Result);

            goto Exit;
        }
    }

    gPlayer.Name[5] = gRegistryParams.Name[5];

    LogMessageA(Informational, "[%s] gRegistryParams.Name[5] is %c", __FUNCTION__, gPlayer.Name[5]);

    Result = RegGetValueA(RegKey, NULL, "gRegistryParams.Name[6]", RRF_RT_REG_SZ, NULL, (void*)&gRegistryParams.Name[6], &RegBytesRead);

    if (Result != ERROR_SUCCESS)
    {
        if (Result == ERROR_FILE_NOT_FOUND)
        {
            Result = ERROR_SUCCESS;

            LogMessageA(Informational, "[%s] Registry value 'gRegistryParams.Name[6]' not found. Using default of 0", __FUNCTION__);

            gRegistryParams.Name[6] = '\0';
        }
        else
        {
            LogMessageA(Error, "[%s] Failed to read the 'gRegistryParams.Name[6]' registry value! Error 0x%08lx!", __FUNCTION__, Result);

            goto Exit;
        }
    }

    gPlayer.Name[6] = gRegistryParams.Name[6];

    LogMessageA(Informational, "[%s] gRegistryParams.Name[6] is %c", __FUNCTION__, gPlayer.Name[6]);

    Result = RegGetValueA(RegKey, NULL, "gRegistryParams.Name[7]", RRF_RT_REG_SZ, NULL, (void*)&gRegistryParams.Name[7], &RegBytesRead);

    if (Result != ERROR_SUCCESS)
    {
        if (Result == ERROR_FILE_NOT_FOUND)
        {
            Result = ERROR_SUCCESS;

            LogMessageA(Informational, "[%s] Registry value 'gRegistryParams.Name[7]' not found. Using default of 0", __FUNCTION__);

            gRegistryParams.Name[7] = '\0';
        }
        else
        {
            LogMessageA(Error, "[%s] Failed to read the 'gRegistryParams.Name[7]' registry value! Error 0x%08lx!", __FUNCTION__, Result);

            goto Exit;
        }
    }

    gPlayer.Name[7] = gRegistryParams.Name[7];

    LogMessageA(Informational, "[%s] gRegistryParams.Name[7] is %c", __FUNCTION__, gPlayer.Name[7]);

    
Exit:

    if (RegKey)
    {
        RegCloseKey(RegKey);
    }

    return Result;
}

DWORD SaveRegistryParametars(void)
{
    DWORD Result = ERROR_SUCCESS;

    HKEY RegKey = NULL;

    DWORD RegDisposition = 0;

    Result = RegCreateKeyExA(HKEY_CURRENT_USER, "SOFTWARE\\GAMEB", 0, NULL, 0, KEY_ALL_ACCESS, NULL, &RegKey, &RegDisposition);

    if (Result != ERROR_SUCCESS)
    {
        LogMessageA(Error, "[%s] RegCreateKey failed with error code 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    DWORD SFXVolume = (DWORD)(gSFXVolume * 100);

    DWORD MusicVolume = (DWORD)(gMusicVolume * 100);

    DWORD WorldPosX = gPlayer.WorldPos.x;

    DWORD WorldPosY = gPlayer.WorldPos.y;

    DWORD ScreenPosX = gPlayer.ScreenPos.x;

    DWORD ScreenPosY = gPlayer.ScreenPos.y;

    DWORD CameraX = gCamera.x;

    DWORD CameraY = gCamera.y;

    DWORD CurrentArmor = gPlayer.CurrentArmor;

    DWORD Direction = gPlayer.Direction;

    DWORD MovementRemaining = gPlayer.MovementRemaining;

    LogMessageA(Informational, "[%s] RegKey open for save", __FUNCTION__);

    Result =  RegSetValueExA(RegKey, "SFXVolume", 0, REG_DWORD, &SFXVolume, sizeof(DWORD));

    if (Result != ERROR_SUCCESS)
    {
        LogMessageA(Error, "[%s] Failed to set 'SFXVolume' in the registry! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    LogMessageA(Informational, "[%s] SFXVolume saved: %d", __FUNCTION__, SFXVolume);

    Result = RegSetValueExA(RegKey, "MusicVolume", 0, REG_DWORD, &MusicVolume, sizeof(DWORD));

    if (Result != ERROR_SUCCESS)
    {
        LogMessageA(Error, "[%s] Failed to set 'MusicVolume' in the registry! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    LogMessageA(Informational, "[%s] MusicVolume saved: %d", __FUNCTION__, MusicVolume);

    Result = RegSetValueExA(RegKey, "ScaleFactor", 0, REG_DWORD, &gPerformanceData.CurrentScaleFactor, sizeof(DWORD));

    if (Result != ERROR_SUCCESS)
    {
        LogMessageA(Error, "[%s] Failed to set 'CurrentScaleFacor' in the registry! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    LogMessageA(Informational, "[%s] ScaleFactor saved: %d", __FUNCTION__, gPerformanceData.CurrentScaleFactor);

    Result = RegSetValueExA(RegKey, "gPlayer.WorldPos.x", 0, REG_DWORD, &WorldPosX, sizeof(DWORD));

    if (Result != ERROR_SUCCESS)
    {
        LogMessageA(Error, "[%s] Failed to set 'gPlayer.WorldPos.x' in the registry! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    LogMessageA(Informational, "[%s] gPlayer.WorldPos.x saved: %d", __FUNCTION__, gPlayer.WorldPos.x);

    Result = RegSetValueExA(RegKey, "gPlayer.WorldPos.y", 0, REG_DWORD, &WorldPosY, sizeof(DWORD));

    if (Result != ERROR_SUCCESS)
    {
        LogMessageA(Error, "[%s] Failed to set 'gPlayer.WorldPos.y' in the registry! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    LogMessageA(Informational, "[%s] gPlayer.WorldPos.y saved: %d", __FUNCTION__, WorldPosY);

    Result = RegSetValueExA(RegKey, "gPlayer.ScreenPos.x", 0, REG_DWORD, &ScreenPosX, sizeof(DWORD));

    if (Result != ERROR_SUCCESS)
    {
        LogMessageA(Error, "[%s] Failed to set 'gPlayer.ScreenPos.x' in the registry! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    LogMessageA(Informational, "[%s] gPlayer.ScreenPos.x saved: %d", __FUNCTION__, gPlayer.ScreenPos.x);

    Result = RegSetValueExA(RegKey, "gPlayer.ScreenPos.y", 0, REG_DWORD, &ScreenPosY, sizeof(DWORD));

    if (Result != ERROR_SUCCESS)
    {
        LogMessageA(Error, "[%s] Failed to set 'gPlayer.ScreenPos.y' in the registry! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    LogMessageA(Informational, "[%s] gPlayer.ScreenPos.y saved: %d", __FUNCTION__, gPlayer.ScreenPos.y);

    Result = RegSetValueExA(RegKey, "gCamera.x", 0, REG_DWORD, &CameraX, sizeof(DWORD));

    if (Result != ERROR_SUCCESS)
    {
        LogMessageA(Error, "[%s] Failed to set 'gCamera.x' in the registry! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    LogMessageA(Informational, "[%s] gCamera.x saved: %d", __FUNCTION__, gCamera.x);

    Result = RegSetValueExA(RegKey, "gCamera.y", 0, REG_DWORD, &CameraY, sizeof(DWORD));

    if (Result != ERROR_SUCCESS)
    {
        LogMessageA(Error, "[%s] Failed to set 'gCamera.y' in the registry! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    LogMessageA(Informational, "[%s] gCamera.y saved: %d", __FUNCTION__, gCamera.y);

    Result = RegSetValueExA(RegKey, "gPlayer.Direction", 0, REG_DWORD, &Direction, sizeof(DWORD));

    if (Result != ERROR_SUCCESS)
    {
        LogMessageA(Error, "[%s] Failed to set 'gPlayer.Direction' in the registry! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    LogMessageA(Informational, "[%s] gPlayer.Direction saved: %d", __FUNCTION__, gPlayer.Direction);

    Result = RegSetValueExA(RegKey, "gPlayer.MovementRemaining", 0, REG_DWORD, &MovementRemaining, sizeof(DWORD));

    if (Result != ERROR_SUCCESS)
    {
        LogMessageA(Error, "[%s] Failed to set 'gPlayer.MovementRemaining' in the registry! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    LogMessageA(Informational, "[%s] gPlayer.MovementRemaining saved: %d", __FUNCTION__, gPlayer.MovementRemaining);

    Result = RegSetValueExA(RegKey, "gPlayer.CurrentArmor", 0, REG_DWORD, &CurrentArmor, sizeof(DWORD));

    if (Result != ERROR_SUCCESS)
    {
        LogMessageA(Error, "[%s] Failed to set 'gPlayer.CurrentAromro' in the registry! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    LogMessageA(Informational, "[%s] gPlayer.CurrentArmor saved: %d", __FUNCTION__, gPlayer.CurrentArmor);

    Result = RegSetValueExA(RegKey, "gRegistryParams.GameSaved", 0, REG_DWORD, &gRegistryParams.GameSaved, sizeof(DWORD));

    if (Result != ERROR_SUCCESS)
    {
        LogMessageA(Error, "[%s] Failed to set 'gRegistryParams.GameSaved' in the registry! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    LogMessageA(Informational, "[%s] gRegistryParams.GameSaved saved: %d", __FUNCTION__, gRegistryParams.GameSaved);
    
    Result = RegSetValueExA(RegKey, "gRegistryParams.Name[0]", 0, REG_SZ, &gPlayer.Name[0], sizeof(char));

    if (Result != ERROR_SUCCESS)
    {
        LogMessageA(Error, "[%s] Failed to set 'gRegistryParams.Name[0]' in the registry! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    LogMessageA(Informational, "[%s] gRegistryParams.Name[0] saved: %c", __FUNCTION__, gPlayer.Name[0]);

    Result = RegSetValueExA(RegKey, "gRegistryParams.Name[1]", 0, REG_SZ, &gPlayer.Name[1], sizeof(char));

    if (Result != ERROR_SUCCESS)
    {
        LogMessageA(Error, "[%s] Failed to set 'gRegistryParams.Name[1]' in the registry! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    LogMessageA(Informational, "[%s] gRegistryParams.Name[1] saved: %c", __FUNCTION__, gPlayer.Name[1]);

    Result = RegSetValueExA(RegKey, "gRegistryParams.Name[2]", 0, REG_SZ, &gPlayer.Name[2], sizeof(char));

    if (Result != ERROR_SUCCESS)
    {
        LogMessageA(Error, "[%s] Failed to set 'gRegistryParams.Name[2]' in the registry! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    LogMessageA(Informational, "[%s] gRegistryParams.Name[2] saved: %c", __FUNCTION__, gPlayer.Name[2]);

    Result = RegSetValueExA(RegKey, "gRegistryParams.Name[3]", 0, REG_SZ, &gPlayer.Name[3], sizeof(char));

    if (Result != ERROR_SUCCESS)
    {
        LogMessageA(Error, "[%s] Failed to set 'gRegistryParams.Name[3]' in the registry! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    LogMessageA(Informational, "[%s] gRegistryParams.Name[3] saved: %c", __FUNCTION__, gPlayer.Name[3]);

    Result = RegSetValueExA(RegKey, "gRegistryParams.Name[4]", 0, REG_SZ, &gPlayer.Name[4], sizeof(char));

    if (Result != ERROR_SUCCESS)
    {
        LogMessageA(Error, "[%s] Failed to set 'gRegistryParams.Name[4]' in the registry! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    LogMessageA(Informational, "[%s] gRegistryParams.Name[4] saved: %c", __FUNCTION__, gPlayer.Name[4]);

    Result = RegSetValueExA(RegKey, "gRegistryParams.Name[5]", 0, REG_SZ, &gPlayer.Name[5], sizeof(char));

    if (Result != ERROR_SUCCESS)
    {
        LogMessageA(Error, "[%s] Failed to set 'gRegistryParams.Name[5]' in the registry! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    LogMessageA(Informational, "[%s] gRegistryParams.Name[5] saved: %c", __FUNCTION__, gPlayer.Name[5]);

    Result = RegSetValueExA(RegKey, "gRegistryParams.Name[6]", 0, REG_SZ, &gPlayer.Name[6], sizeof(char));

    if (Result != ERROR_SUCCESS)
    {
        LogMessageA(Error, "[%s] Failed to set 'gRegistryParams.Name[6]' in the registry! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    LogMessageA(Informational, "[%s] gRegistryParams.Name[6] saved: %c", __FUNCTION__, gPlayer.Name[6]);

    Result = RegSetValueExA(RegKey, "gRegistryParams.Name[7]", 0, REG_SZ, &gPlayer.Name[7], sizeof(char));

    if (Result != ERROR_SUCCESS)
    {
        LogMessageA(Error, "[%s] Failed to set 'gRegistryParams.Name[7]' in the registry! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    LogMessageA(Informational, "[%s] gRegistryParams.Name[7] saved: %c", __FUNCTION__, gPlayer.Name[7]);


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

    EnterCriticalSection(&gLogCritSec);
    

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

    LeaveCriticalSection(&gLogCritSec);
}

void DrawDebugInfo()
{
    

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

    sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "ScreenXY: %d, %d", gPlayer.ScreenPos.x, gPlayer.ScreenPos.y); // _count of returns the number of characters, "places"

    BlitStringToBuffer(DebugTextBuffer, &g6x7Font, White, 0, 72);

    snprintf(DebugTextBuffer, sizeof(DebugTextBuffer), "WorldXY:   %d, %d", gPlayer.WorldPos.x, gPlayer.WorldPos.y);

    BlitStringToBuffer(DebugTextBuffer, &g6x7Font, White, 0, 80);

    snprintf(DebugTextBuffer, sizeof(DebugTextBuffer), "CameraXY:  %d, %d", gCamera.x, gCamera.y);

    BlitStringToBuffer(DebugTextBuffer, &g6x7Font, White, 0, 88);

    snprintf(DebugTextBuffer, sizeof(DebugTextBuffer), "Movment:  %d", gPlayer.MovementRemaining);

    BlitStringToBuffer(DebugTextBuffer, &g6x7Font, White, 0, 96);
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

DWORD LoadWaveFromMemory(_In_ void* Buffer, _Inout_ GAMESOUND* GameSound)
{
    DWORD Result = ERROR_SUCCESS;

    DWORD RIFF = 0;

    //DWORD NumberOfBytesRead = 0;

    uint16_t DataChunkOffset = 0;

    DWORD DataChunkSearcher = 0;

    BOOL DataChunkFound = FALSE;

    DWORD DataChunkSize = 0;

    //void* AudioData = NULL;

    //HANDLE FileHandle = INVALID_HANDLE_VALUE;

    /*if ((FileHandle = CreateFileA(FileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
    {
        Result = GetLastError();

        LogMessageA(Error, "[%s] CreateFileA failed! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }*/

    memcpy(&RIFF, Buffer, sizeof(DWORD));

    /*if (ReadFile(FileHandle, &RIFF, sizeof(DWORD), &NumberOfBytesRead, NULL) == 0)
    {
        Result = GetLastError();

        LogMessageA(Error, "[%s] ReadFile failed! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }*/

    if (RIFF != 0x46464952) // "RIFF" backwards
    {
        Result = ERROR_FILE_INVALID;

        LogMessageA(Error, "[%s] First four bytes of memory buffer are not RIFF! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    /*if (SetFilePointer(FileHandle, 20, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
    {
        Result = GetLastError();

        LogMessageA(Error, "[%s] SetFilePointer failed! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }*/

    memcpy(&GameSound->WaveFormat, (BYTE*)Buffer + 20, sizeof(WAVEFORMATEX));

    /*if (ReadFile(FileHandle, &GameSound->WaveFormat, sizeof(WAVEFORMATEX), &NumberOfBytesRead, NULL) == 0)
    {
        Result = GetLastError();

        LogMessageA(Error, "[%s] CreateFileA failed! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }*/

    if (GameSound->WaveFormat.nBlockAlign != ((GameSound->WaveFormat.nChannels * GameSound->WaveFormat.wBitsPerSample) / 8) ||
        (GameSound->WaveFormat.wFormatTag != WAVE_FORMAT_PCM) || (GameSound->WaveFormat.wBitsPerSample != 16))
    {
        Result = ERROR_DATATYPE_MISMATCH;

        LogMessageA(Error, "[%s] This wave data in the memory buffer did not meet the format requirements! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    while (DataChunkFound == FALSE)
    {
        /*if (SetFilePointer(FileHandle, DataChunkOffset, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
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
        }*/

        memcpy(&DataChunkSearcher, (BYTE*)Buffer + DataChunkOffset, sizeof(DWORD));

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

    /*if (SetFilePointer(FileHandle, DataChunkOffset + 4, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
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
    }*/

    memcpy(&DataChunkSize, (BYTE*)Buffer + DataChunkOffset + 4, sizeof(DWORD));

    /*AudioData = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, DataChunkSize);

    if (AudioData == NULL)
    {
        Result = ERROR_NOT_ENOUGH_MEMORY;

        LogMessageA(Error, "[%s] HeapAlloc failed! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }*/

    GameSound->Buffer.Flags = XAUDIO2_END_OF_STREAM;

    GameSound->Buffer.AudioBytes = DataChunkSize;

    /*if (SetFilePointer(FileHandle, DataChunkOffset + 8, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
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
    }*/

    GameSound->Buffer.pAudioData = (BYTE*)Buffer + DataChunkOffset + 8;

Exit:
    if (Result == ERROR_SUCCESS)
    {
        LogMessageA(Informational, "[%s] Successfully loaded wav from memory %s!", __FUNCTION__);
    }
    else
    {
        LogMessageA(Error, "[%s] Failed to load wav from memory! Error 0x%08lx!", __FUNCTION__, Result);
    }

    /*if (FileHandle != NULL && FileHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(FileHandle);
    }*/

    return Result;
}

DWORD LoadOggFromMemory(_In_ void* Buffer, _In_ uint32_t BufferSize, _Inout_ GAMESOUND* GameSound)
{
    DWORD Result = ERROR_SUCCESS;

    int SamplesDecoded = 0;

    int Channels = 0;

    int SampleRate = 0;

    short* DecodedAudio = NULL;

    SamplesDecoded = stb_vorbis_decode_memory(Buffer, (int)BufferSize, &Channels, &SampleRate, &DecodedAudio);

    if (SamplesDecoded < 1)
    {
        Result = ERROR_BAD_COMPRESSION_BUFFER;

        LogMessageA(Error, "[%s] stb_vorbis_decode_memory failed! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    GameSound->WaveFormat.wFormatTag = WAVE_FORMAT_PCM;

    GameSound->WaveFormat.nChannels = (WORD)Channels;

    GameSound->WaveFormat.nSamplesPerSec = SampleRate;

    GameSound->WaveFormat.nAvgBytesPerSec = GameSound->WaveFormat.nSamplesPerSec * GameSound->WaveFormat.nChannels * 2;

    GameSound->WaveFormat.nBlockAlign = GameSound->WaveFormat.nChannels * 2;

    GameSound->WaveFormat.wBitsPerSample = 16;

    GameSound->Buffer.Flags = XAUDIO2_END_OF_STREAM;

    GameSound->Buffer.AudioBytes = SamplesDecoded * GameSound->WaveFormat.nChannels * 2;

    GameSound->Buffer.pAudioData = (const BYTE*)DecodedAudio;

Exit:
    return Result;
}

DWORD LoadTilemapFromMemory(_In_ void* Buffer, uint32_t BufferSize, _Inout_ TILEMAP* TileMap)
{
    DWORD Result = ERROR_SUCCESS;

    DWORD NumberOfBytesRead = 0;

    char* Cursor = NULL;

    char TempBuffer[16] = { 0 };

    uint16_t Rows = 0;

    uint16_t Columns = 0;

    if (BufferSize < 300)
    {
        Result = GetLastError();

        LogMessageA(Error, "[%s] Buffer is too small to be a valid tilemap! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    if ((Cursor = strstr(Buffer, "width=")) == NULL)
    {
        Result = ERROR_INVALID_DATA;

        LogMessageA(Error, "[%s] Could not locate the width attribute! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    NumberOfBytesRead = 0;

    for (;;)
    {
        if (NumberOfBytesRead > 8)
        {
            // We shoudl have found opening quotation mark

            Result = ERROR_INVALID_DATA;

            LogMessageA(Error, "[%s] Could not locate the opening quotation mark befote the width attribute! Error 0x%08lx!", __FUNCTION__, Result);

            goto Exit;
        }

        if (*Cursor == '\"')
        {
            Cursor++;

            break;
        }
        else
        {
            Cursor++;
        }

        NumberOfBytesRead++;
    }

    NumberOfBytesRead = 0;

    for (uint8_t Counter = 0; Counter < 6; Counter++)
    {
        if (*Cursor == '\"')
        {
            Cursor++;

            break;
        }
        else
        {
            TempBuffer[Counter] = *Cursor;

            Cursor++;
        }
    }

    TileMap->Width = atoi(TempBuffer);

    if (TileMap->Width == 0)
    {
        Result = ERROR_INVALID_DATA;

        LogMessageA(Error, "[%s] Width attribute is 0! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }


    memset(TempBuffer, 0, sizeof(TempBuffer));

    if ((Cursor = strstr(Buffer, "height=")) == NULL)
    {
        Result = ERROR_INVALID_DATA;

        LogMessageA(Error, "[%s] Could not locate the height attribute! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    NumberOfBytesRead = 0;

    for (;;)
    {
        if (NumberOfBytesRead > 8)
        {
            // We shoudl have found opening quotation mark

            Result = ERROR_INVALID_DATA;

            LogMessageA(Error, "[%s] Could not locate the opening quotation mark befote the height attribute! Error 0x%08lx!", __FUNCTION__, Result);

            goto Exit;
        }

        if (*Cursor == '\"')
        {
            Cursor++;

            break;
        }
        else
        {
            Cursor++;
        }

        NumberOfBytesRead++;
    }

    NumberOfBytesRead = 0;

    for (uint8_t Counter = 0; Counter < 6; Counter++)
    {
        if (*Cursor == '\"')
        {
            Cursor++;

            break;
        }
        else
        {
            TempBuffer[Counter] = *Cursor;

            Cursor++;
        }
    }

    TileMap->Height = atoi(TempBuffer);

    if (TileMap->Height == 0)
    {
        Result = ERROR_INVALID_DATA;

        LogMessageA(Error, "[%s] Height attribute is 0! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    Rows = TileMap->Height;

    Columns = TileMap->Width;

    TileMap->Map = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, Rows * sizeof(void*));

    if (TileMap->Map == NULL)
    {
        Result = ERROR_OUTOFMEMORY;

        LogMessageA(Error, "[%s] HeapAlloc failed! 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    for (uint16_t Counter = 0; Counter < TileMap->Height; Counter++)
    {
        TileMap->Map[Counter] = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, Columns * sizeof(void*));

        if (TileMap->Map[Counter] == NULL)
        {
            Result = ERROR_OUTOFMEMORY;

            LogMessageA(Error, "[%s] HeapAlloc failed! 0x%08lx!", __FUNCTION__, Result);

            goto Exit;
        }
    }

    NumberOfBytesRead = 0;

    memset(TempBuffer, 0, sizeof(TempBuffer));

    if ((Cursor = strstr(Buffer, ",")) == NULL)
    {
        Result = ERROR_INVALID_DATA;

        LogMessageA(Error, "[%s] Could not find a comma character! 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    while (*Cursor != '\r' && *Cursor != '\n')
    {
        if (NumberOfBytesRead > 4)
        {
            Result = ERROR_INVALID_DATA;

            LogMessageA(Error, "[%s] Could not find a new line character at the beginning of the tile map data! 0x%08lx!", __FUNCTION__, Result);

            goto Exit;
        }

        NumberOfBytesRead++;

        Cursor--;
    }

    Cursor++;

    for (uint16_t Row = 0; Row < Rows; Row++)
    {
        for (uint16_t Column = 0; Column < Columns; Column++)
        {
            memset(TempBuffer, 0, sizeof(TempBuffer));

        Skip:

            if (*Cursor == '\r' || *Cursor == '\n')
            {
                Cursor++;

                goto Skip;
            }

            for (uint8_t Counter = 0; Counter < 8; Counter++)
            {
                if (*Cursor == ',' || *Cursor == '<')
                {
                    if (((TileMap->Map[Row][Column]) = (uint8_t)atoi(TempBuffer)) == 0)
                    {
                        Result = ERROR_INVALID_DATA;

                        LogMessageA(Error, "[%s] atoi failed while converting tile map data! (The tilemap cannot contain any tiles with the value 0, because atoi cannot differentiate between 0 and failure.) 0x%08lx!", __FUNCTION__, Result);

                        goto Exit;
                    }

                    Cursor++;

                    break;
                }

                TempBuffer[Counter] = *Cursor;

                Cursor++;
            }
        }
    }


Exit:
    

    return Result;
}

DWORD Load32BppBitmapFromMemoy(_In_ void* Buffer, _Inout_ GAMEBITMAP* GameBitmap)
{
    DWORD Error = ERROR_SUCCESS;

    WORD BitmapHeader = 0;

    DWORD PixelDataOffset = 0;

    //DWORD NumberOfBytesRead = 2;

    memcpy(&BitmapHeader, Buffer, 2);

    if (BitmapHeader != 0x4d42) // "BM" backwards
    {
        Error = ERROR_INVALID_DATA;

        goto Exit;
    }

    memcpy(&PixelDataOffset, (BYTE*)Buffer + 0xA, sizeof(DWORD));

    memcpy(&GameBitmap->Bitmapinfo.bmiHeader, (BYTE*)Buffer + 0xE, sizeof(BITMAPINFOHEADER));

    GameBitmap->Memory = (BYTE*)Buffer + PixelDataOffset;


Exit:

    if (Error == ERROR_SUCCESS)
    {
        LogMessageA(Informational, "[%s] Loading successful", __FUNCTION__);
    }
    else
    {
        LogMessageA(Error, "[%s] Loading failed! Error 0x%08lx!", __FUNCTION__, Error);
    }

    return Error;
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

void PlayGameMusic(_In_ GAMESOUND *GameSound)
{
    gXAudioMusicSourceVoice->lpVtbl->Stop(gXAudioMusicSourceVoice, 0, 0);

    gXAudioMusicSourceVoice->lpVtbl->FlushSourceBuffers(gXAudioMusicSourceVoice);

    GameSound->Buffer.LoopCount = XAUDIO2_LOOP_INFINITE;

    gXAudioMusicSourceVoice->lpVtbl->SubmitSourceBuffer(gXAudioMusicSourceVoice, &GameSound->Buffer, NULL);

    gXAudioMusicSourceVoice->lpVtbl->Start(gXAudioMusicSourceVoice, 0, XAUDIO2_COMMIT_NOW);

    
}

BOOL MusicIsPlaying(void)
{
    XAUDIO2_VOICE_STATE State = { 0 };

    gXAudioMusicSourceVoice->lpVtbl->GetState(gXAudioMusicSourceVoice, &State, 0);

    if (State.BuffersQueued > 0)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

DWORD LoadTilemapFromFile(_In_ char* FileName, _Inout_ TILEMAP* TileMap)
{
    DWORD Result = ERROR_SUCCESS;

    HANDLE FileHandle = INVALID_HANDLE_VALUE;

    LARGE_INTEGER FileSize = { 0 };

    void* FileBuffer = NULL;

    DWORD NumberOfBytesRead = 0;

    char* Cursor = NULL;

    char TempBuffer[16] = { 0 };

    uint16_t Rows = 0;

    uint16_t Columns = 0;

    if ((FileHandle = CreateFileA(FileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
    {
        Result = GetLastError();

        LogMessageA(Error, "[%s] CreateFileA failed! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    if (GetFileSizeEx(FileHandle, &FileSize) == 0)
    {
        Result = GetLastError;

        LogMessageA(Error, "[%s] GetFileSizeEx failed! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    LogMessageA(Informational, "[%s] Size of file %s: %lu", __FUNCTION__, FileName, FileSize);

    FileBuffer = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, FileSize.QuadPart);

    if (FileBuffer == NULL)
    {
        Result = ERROR_OUTOFMEMORY;

        LogMessageA(Error, "[%s] HeapAlloc failed! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    if (ReadFile(FileHandle, FileBuffer, (DWORD)FileSize.QuadPart, &NumberOfBytesRead, NULL) == 0)
    {
        Result = GetLastError();

        LogMessageA(Error, "[%s] ReadFile failed! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    if ((Cursor = strstr(FileBuffer, "width=")) == NULL)
    {
        Result = ERROR_INVALID_DATA;

        LogMessageA(Error, "[%s] Could not locate the width attribute! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    NumberOfBytesRead = 0;

    for (;;)
    {
        if (NumberOfBytesRead > 8)
        {
            // We shoudl have found opening quotation mark

            Result = ERROR_INVALID_DATA;

            LogMessageA(Error, "[%s] Could not locate the opening quotation mark befote the width attribute! Error 0x%08lx!", __FUNCTION__, Result);

            goto Exit;
        }

        if (*Cursor == '\"')
        {
            Cursor++;

            break;
        }
        else
        {
            Cursor++;
        }

        NumberOfBytesRead++;
    }
    
    NumberOfBytesRead = 0;

    for (uint8_t Counter = 0; Counter < 6; Counter++)
    {
        if (*Cursor == '\"')
        {
            Cursor++;

            break;
        }
        else
        {
            TempBuffer[Counter] = *Cursor;

            Cursor++;
        }
    }

    TileMap->Width = atoi(TempBuffer);

    if (TileMap->Width == 0)
    {
        Result = ERROR_INVALID_DATA;

        LogMessageA(Error, "[%s] Width attribute is 0! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    memset(TempBuffer, 0, sizeof(TempBuffer));

    if ((Cursor = strstr(FileBuffer, "height=")) == NULL)
    {
        Result = ERROR_INVALID_DATA;

        LogMessageA(Error, "[%s] Could not locate the height attribute! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    NumberOfBytesRead = 0;

    for (;;)
    {
        if (NumberOfBytesRead > 8)
        {
            // We shoudl have found opening quotation mark

            Result = ERROR_INVALID_DATA;

            LogMessageA(Error, "[%s] Could not locate the opening quotation mark befote the height attribute! Error 0x%08lx!", __FUNCTION__, Result);

            goto Exit;
        }

        if (*Cursor == '\"')
        {
            Cursor++;

            break;
        }
        else
        {
            Cursor++;
        }

        NumberOfBytesRead++;
    }

    NumberOfBytesRead = 0;

    for (uint8_t Counter = 0; Counter < 6; Counter++)
    {
        if (*Cursor == '\"')
        {
            Cursor++;

            break;
        }
        else
        {
            TempBuffer[Counter] = *Cursor;

            Cursor++;
        }
    }

    TileMap->Height = atoi(TempBuffer);

    if (TileMap->Height == 0)
    {
        Result = ERROR_INVALID_DATA;

        LogMessageA(Error, "[%s] Height attribute is 0! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    Rows = TileMap->Height;

    Columns = TileMap->Width;

    TileMap->Map = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, Rows * sizeof(void*));

    if (TileMap->Map == NULL)
    {
        Result = ERROR_OUTOFMEMORY;

        LogMessageA(Error, "[%s] HeapAlloc failed! 0x%08lx!", __FUNCTION__, Error);

        goto Exit;
    }

    for (uint16_t Counter = 0; Counter < TileMap->Height; Counter++)
    {
        TileMap->Map[Counter] = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, Columns * sizeof(void*));

        if (TileMap->Map[Counter] == NULL)
        {
            Result = ERROR_OUTOFMEMORY;

            LogMessageA(Error, "[%s] HeapAlloc failed! 0x%08lx!", __FUNCTION__, Error);

            goto Exit;
        }
    }

    NumberOfBytesRead = 0;

    memset(TempBuffer, 0, sizeof(TempBuffer));

    if ((Cursor = strstr(FileBuffer, ",")) == NULL)
    {
        Result = ERROR_INVALID_DATA;

        LogMessageA(Error, "[%s] Could not find a comma character! 0x%08lx!", __FUNCTION__, Error);

        goto Exit;
    }

    while (*Cursor != '\r' && *Cursor != '\n')
    {
        if (NumberOfBytesRead > 4)
        {
            Result = ERROR_INVALID_DATA;

            LogMessageA(Error, "[%s] Could not find a new line character at the beginning of the tile map data! 0x%08lx!", __FUNCTION__, Error);

            goto Exit;
        }

        NumberOfBytesRead++;

        Cursor--;
    }

    Cursor++;

    for (uint16_t Row = 0; Row < Rows; Row++)
    {
        for (uint16_t Column = 0; Column < Columns; Column++)
        {
            memset(TempBuffer, 0, sizeof(TempBuffer));

        Skip:

            if (*Cursor == '\r' || *Cursor == '\n')
            {
                Cursor++;

                goto Skip;
            }

            for (uint8_t Counter = 0; Counter < 8; Counter++)
            {
                if (*Cursor == ',' || *Cursor == '<')
                {
                    if (((TileMap->Map[Row][Column]) = (uint8_t)atoi(TempBuffer)) == 0)
                    {
                        Result = ERROR_INVALID_DATA;

                        LogMessageA(Error, "[%s] atoi failed while converting tile map data! (The tilemap cannot contain any tiles with the value 0, because atoi cannot differentiate between 0 and failure.) 0x%08lx!", __FUNCTION__, Error);

                        goto Exit;
                    }

                    Cursor++;

                    break;
                }

                TempBuffer[Counter] = *Cursor;

                Cursor++;
            }
        }
    }


Exit:
    if (FileHandle && FileHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(FileHandle);
    }

    if (FileBuffer)
    {
        HeapFree(GetProcessHeap(), 0, FileBuffer);
    }

    return Result;
}

DWORD LoadOggFromFile(_In_ char* FileName, _Inout_ GAMESOUND* GameSound)
{
    DWORD Result = ERROR_SUCCESS;

    HANDLE FileHandle = INVALID_HANDLE_VALUE;

    LARGE_INTEGER FileSize = { 0 };

    void* FileBuffer = NULL;

    DWORD NumberOfBytesRead = 0;

    int SamplesDecoded = 0;

    int Channels = 0;

    int SampleRate = 0;

    short* DecodedAudio = NULL;

    if ((FileHandle = CreateFileA(FileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
    {
        Result = GetLastError();

        LogMessageA(Error, "[%s] CreateFileA failed! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    if (GetFileSizeEx(FileHandle, &FileSize) == 0)
    {
        Result = GetLastError;

        LogMessageA(Error, "[%s] GetFileSizeEx failed! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    LogMessageA(Informational, "[%s] Size of file %s: %lu", __FUNCTION__, FileName, FileSize);

    FileBuffer = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, FileSize.QuadPart);

    if (FileBuffer == NULL)
    {
        Result = ERROR_OUTOFMEMORY;

        LogMessageA(Error, "[%s] HeapAlloc failed! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    if (ReadFile(FileHandle, FileBuffer, (DWORD)FileSize.QuadPart, &NumberOfBytesRead, NULL) == 0)
    {
        Result = GetLastError();

        LogMessageA(Error, "[%s] ReadFile failed! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    SamplesDecoded = stb_vorbis_decode_memory(FileBuffer, FileSize.QuadPart, &Channels, &SampleRate, &DecodedAudio);

    if (SamplesDecoded < 1)
    {
        Result = ERROR_BAD_COMPRESSION_BUFFER;

        LogMessageA(Error, "[%s] stb_vorbis_decode_memory failed! Error 0x%08lx on %s!", __FUNCTION__, Result, FileName);

        goto Exit;
    }

    GameSound->WaveFormat.wFormatTag = WAVE_FORMAT_PCM;

    GameSound->WaveFormat.nChannels = Channels;

    GameSound->WaveFormat.nSamplesPerSec = SampleRate;

    GameSound->WaveFormat.nAvgBytesPerSec = GameSound->WaveFormat.nSamplesPerSec * GameSound->WaveFormat.nChannels * 2;

    GameSound->WaveFormat.nBlockAlign = GameSound->WaveFormat.nChannels * 2;

    GameSound->WaveFormat.wBitsPerSample = 16;

    GameSound->Buffer.Flags = XAUDIO2_END_OF_STREAM;

    GameSound->Buffer.AudioBytes = SamplesDecoded * GameSound->WaveFormat.nChannels * 2;

    GameSound->Buffer.pAudioData = DecodedAudio;

Exit:
    if (FileBuffer)
    {
        HeapFree(GetProcessHeap(), 0, FileBuffer);
    }
    
    return Result;
}

DWORD LoadAssetFromArchive(_In_ char* ArchiveName, _In_ char* AssetFileName, _In_ RESOURCETYPE ResourceType, _Inout_ void* Resource)
{
    DWORD Result = ERROR_SUCCESS;

    mz_zip_archive Archive = { 0 };

    BYTE* DecompressedBuffer = NULL;

    size_t DecompressedSize = 0;

    BOOL FileFoundInArchive = FALSE;

    if ((mz_zip_reader_init_file(&Archive, ArchiveName, 0)) == FALSE)
    {
        Result = mz_zip_get_last_error(&Archive);

        LogMessageA(Error, "[%s] mz_zip_reader_init_file failed! Error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    LogMessageA(Informational, "[%s] Archive %s opened.", __FUNCTION__, ArchiveName);

    // Iterate through each file in the archive until we find file we are looking for

    for (int FileIndex = 0; FileIndex < (int)mz_zip_reader_get_num_files(&Archive); FileIndex++)
    {
        mz_zip_archive_file_stat CompressedFileStatistics = { 0 };

        if (mz_zip_reader_file_stat(&Archive, FileIndex, &CompressedFileStatistics) == MZ_FALSE)
        {
            Result = mz_zip_get_last_error(&Archive);

            LogMessageA(Error, "[%s] mz_zip_reader_file_stat failed! Error 0x%08lx! Archive: %s, File: %s", __FUNCTION__, Result, ArchiveName, AssetFileName);

            goto Exit;
        }

        if (_stricmp(CompressedFileStatistics.m_filename, AssetFileName) == 0)
        {

            LogMessageA(Informational, "[%s] File %s found in asset file %s and extracted to heap.", __FUNCTION__, AssetFileName, ArchiveName);

            FileFoundInArchive = TRUE;

            if ((DecompressedBuffer = mz_zip_reader_extract_to_heap(&Archive, FileIndex, &DecompressedSize, 0)) == NULL)
            {
                Result = mz_zip_get_last_error(&Archive);

                LogMessageA(Error, "[%s] mz_zip_reader_extract_file_to_heap failed! Error 0x%08lx! Archive: %s, File: %s", __FUNCTION__, Result, ArchiveName, AssetFileName);

                goto Exit;
            }

            break;
        }
    }

    if (FileFoundInArchive == FALSE)
    {
        Result = ERROR_FILE_NOT_FOUND;

        LogMessageA(Error, "[%s] File %s was not found in archive %s!", __FUNCTION__, AssetFileName, ArchiveName);

        goto Exit;
    }

    switch (ResourceType)
    {
        case RESOURCETYPE_WAV:
        {
            Result = LoadWaveFromMemory(DecompressedBuffer, Resource);

            break;
        }
        case RESOURCETYPE_OGG:
        {
            Result = LoadOggFromMemory(DecompressedBuffer, DecompressedSize, Resource);

            break;
        }
        case RESOURCETYPE_TILEMAP:
        {
            Result = LoadTilemapFromMemory(DecompressedBuffer, DecompressedSize, Resource);

            break;
        }
        case RESOURCETYPE_BMPX:
        {
            Result = Load32BppBitmapFromMemoy(DecompressedBuffer, Resource);

            break;
        }
        default:
        {
            ASSERT(FALSE, "Unknown resource type.")
        }
    }


Exit:

    mz_zip_reader_end(&Archive);

    return Result;
}

DWORD AssetLoadingThreadProc(_In_ LPVOID lpParam)
{
    DWORD Result = ERROR_SUCCESS;

    if (LoadAssetFromArchive(ASSET_FILE, "6x7Font.bmpx", RESOURCETYPE_BMPX, &g6x7Font) != ERROR_SUCCESS)
    {
        LogMessageA(Error, "[%s] LoadAssetFromArchive failed!", __FUNCTION__);

        MessageBoxA(NULL, "LoadAssetFromArchive failed!", "Error!", MB_ICONERROR | MB_OK);

        goto Exit;
    }

    if (LoadAssetFromArchive(ASSET_FILE, "Splash.wav", RESOURCETYPE_WAV, &gSoundSplashScreen) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "LoadWaveFromFile failed!", "Error!", MB_ICONERROR | MB_OK);

        goto Exit;
    }

    if (LoadAssetFromArchive(ASSET_FILE, "Fading.wav", RESOURCETYPE_WAV, &gSoundFadingScreen) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "LoadAssetFromArchive failed!", "Error!", MB_ICONERROR | MB_OK);

        goto Exit;
    }

    SetEvent(gEssentialAssetsLoadedEvent);

    if ((Result = LoadAssetFromArchive(ASSET_FILE, "Overworld01.bmpx", RESOURCETYPE_BMPX, &gOverWorld01.GameBitmap)) != ERROR_SUCCESS)
    {
        LogMessageA(Error, "[%s] Loading Overworld01.bmpx failed with error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    if ((Result = LoadAssetFromArchive(ASSET_FILE, "Overworld01.tmx", RESOURCETYPE_TILEMAP, &gOverWorld01.TIleMap)) != ERROR_SUCCESS)
    {
        LogMessageA(Error, "[%s] Loading Overworld01.tmx failed with error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    if ((Result = LoadAssetFromArchive(ASSET_FILE, "MenuSelect.wav", RESOURCETYPE_WAV, &gSoundMenuNavigate)) != ERROR_SUCCESS)
    {
        LogMessageA(Error, "[%s] Loading MenuSelect.wav failed with error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    if ((Result = LoadAssetFromArchive(ASSET_FILE, "MenuChooser.wav", RESOURCETYPE_WAV, &gSoundMenuChoose)) != ERROR_SUCCESS)
    {
        LogMessageA(Error, "[%s] Loading MenuChooser.wav failed with error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    if ((Result = LoadAssetFromArchive(ASSET_FILE, "Overworld01.ogg", RESOURCETYPE_OGG, &gMusicOverworld01)) != ERROR_SUCCESS)
    {
        LogMessageA(Error, "[%s] Loading Overworld01.ogg failed with error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    if ((Result = LoadAssetFromArchive(ASSET_FILE, "Hero_Suit0_Down_Standing.bmpx", RESOURCETYPE_BMPX, &gPlayer.Sprite[SUIT_0][FACING_DOWN_0])) != ERROR_SUCCESS)
    {
        LogMessageA(Error, "[%s] Loading Hero_Suit0_Down_Standing.bmpx failed with error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    if ((Result = LoadAssetFromArchive(ASSET_FILE, "Hero_Suit0_Down_Walk1.bmpx", RESOURCETYPE_BMPX, &gPlayer.Sprite[SUIT_0][FACING_DOWN_1])) != ERROR_SUCCESS)
    {
        LogMessageA(Error, "[%s] Loading Hero_Suit0_Down_Walk1.bmpx failed with error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    if ((Result = LoadAssetFromArchive(ASSET_FILE, "Hero_Suit0_Down_Walk2.bmpx", RESOURCETYPE_BMPX, &gPlayer.Sprite[SUIT_0][FACING_DOWN_2])) != ERROR_SUCCESS)
    {
        LogMessageA(Error, "[%s] Loading Hero_Suit0_Down_Walk2.bmpx failed with error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    if ((Result = LoadAssetFromArchive(ASSET_FILE, "Hero_Suit0_Left_Standing.bmpx", RESOURCETYPE_BMPX, &gPlayer.Sprite[SUIT_0][FACING_LEFT_0])) != ERROR_SUCCESS)
    {
        LogMessageA(Error, "[%s] Loading Hero_Suit0_Left_Standing.bmpx failed with error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    if ((Result = LoadAssetFromArchive(ASSET_FILE, "Hero_Suit0_Left_Walk1.bmpx", RESOURCETYPE_BMPX, &gPlayer.Sprite[SUIT_0][FACING_LEFT_1])) != ERROR_SUCCESS)
    {
        LogMessageA(Error, "[%s] Loading Hero_Suit0_Left_Walk1.bmpx failed with error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    if ((Result = LoadAssetFromArchive(ASSET_FILE, "Hero_Suit0_Left_Walk2.bmpx", RESOURCETYPE_BMPX, &gPlayer.Sprite[SUIT_0][FACING_LEFT_2])) != ERROR_SUCCESS)
    {
        LogMessageA(Error, "[%s] Loading Hero_Suit0_Left_Walk2.bmpx failed with error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    if ((Result = LoadAssetFromArchive(ASSET_FILE, "Hero_Suit0_Right_Standing.bmpx", RESOURCETYPE_BMPX, &gPlayer.Sprite[SUIT_0][FACING_RIGHT_0])) != ERROR_SUCCESS)
    {
        LogMessageA(Error, "[%s] Loading Hero_Suit0_Right_Standing.bmpx failed with error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    if ((Result = LoadAssetFromArchive(ASSET_FILE, "Hero_Suit0_Right_Walk1.bmpx", RESOURCETYPE_BMPX, &gPlayer.Sprite[SUIT_0][FACING_RIGHT_1])) != ERROR_SUCCESS)
    {
        LogMessageA(Error, "[%s] Loading Hero_Suit0_Right_Walk1.bmpx failed with error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    if ((Result = LoadAssetFromArchive(ASSET_FILE, "Hero_Suit0_Right_Walk2.bmpx", RESOURCETYPE_BMPX, &gPlayer.Sprite[SUIT_0][FACING_RIGHT_2])) != ERROR_SUCCESS)
    {
        LogMessageA(Error, "[%s] Loading Hero_Suit0_Right_Walk2.bmpx failed with error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    if ((Result = LoadAssetFromArchive(ASSET_FILE, "Hero_Suit0_Up_Standing.bmpx", RESOURCETYPE_BMPX, &gPlayer.Sprite[SUIT_0][FACING_UPWARD_0])) != ERROR_SUCCESS)
    {
        LogMessageA(Error, "[%s] Loading Hero_Suit0_Up_Standing.bmpx failed with error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    if ((Result = LoadAssetFromArchive(ASSET_FILE, "Hero_Suit0_Up_Walk1.bmpx", RESOURCETYPE_BMPX, &gPlayer.Sprite[SUIT_0][FACING_UPWARD_1])) != ERROR_SUCCESS)
    {
        LogMessageA(Error, "[%s] Loading Hero_Suit0_Up_Walk1.bmpx failed with error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

    if ((Result = LoadAssetFromArchive(ASSET_FILE, "Hero_Suit0_Up_Walk2.bmpx", RESOURCETYPE_BMPX, &gPlayer.Sprite[SUIT_0][FACING_UPWARD_2])) != ERROR_SUCCESS)
    {
        LogMessageA(Error, "[%s] Loading Hero_Suit0_Up_Walk2.bmpx failed with error 0x%08lx!", __FUNCTION__, Result);

        goto Exit;
    }

Exit:
    return Result;
}

void InitializeGlobals(void)
{
    gGamepadID = -1;

    gPassablieTiles[0] = TILE_GRASS_01;

    gPassablieTiles[1] = TILE_BRIDGE_01;

    gPassablieTiles[2] = TILE_TREE_01;

    gPassablieTiles[3] = TILE_PORTAL_01;

    gPassablieTiles[4] = TILE_BRICK_01;

    gPassablieTiles[5] = TILE_GRAYBRICK_01;

    for (int Counter = 0; Counter < 9; Counter++)
    {
        gPlayer.Name[Counter] = '\0';
    }

    gCurrentGameState = GAMESTATE_OPENINGSPLASHSCREEN;

    gOverwrldArea.left = 0;

    gOverwrldArea.top = 0;

    gOverwrldArea.right = 3840;

    gOverwrldArea.bottom = 2400;

    gDungeon1Area.left = 3856;

    gDungeon1Area.top = 0;

    gDungeon1Area.right = 4240;

    gDungeon1Area.bottom = 240;

    //gCurrentArea = gOverwrldArea;

    gPortal001.ID = 1;

    gPortal001.AreaName = "Dungeon Area";

    gPortal001.WorldPos.x = 336;

    gPortal001.WorldPos.y = 176;

    gPortal001.WorldDest.x = 3952;

    gPortal001.WorldDest.y = 80;

    gPortal001.Area = gDungeon1Area; // the area where the portal takes you

    gPortal001.CameraPos.x = 3856;

    gPortal001.CameraPos.y = 0;

    gPortal001.ScreenPos.x = 96;

    gPortal001.ScreenPos.y = 80;

    gPortal002.AreaName = "Home Town";

    gPortal002.ID = 2;

    gPortal002.WorldPos.x = 3952;

    gPortal002.WorldPos.y = 80;

    gPortal002.WorldDest.x = 336;

    gPortal002.WorldDest.y = 176;

    gPortal002.Area = gOverwrldArea; // the area where the portal takes you

    gPortal002.CameraPos.x = 16;

    gPortal002.CameraPos.y = 0;

    gPortal002.ScreenPos.x = 320;

    gPortal002.ScreenPos.y = 176;

    gPortals[0] = gPortal001;

    gPortals[1] = gPortal002;
}

