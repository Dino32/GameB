#include "Main.h" 

#include "miniz.h"

#include "stb_vorbis.h"

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


    gGamepadID = -1;

    gPassablieTiles[0] = TILE_GRASS_01;

    gPassablieTiles[1] = TILE_BRIDGE_01;

    gPassablieTiles[2] = TILE_TREE_01;

    gCurrentGameState = GAMESTATE_OPENINGSPLASHSCREEN;


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

    if ((Load32BppBitmapFromFile(".\\Assets\\Maps\\Overworld01.bmpx", &gOverWorld01.GameBitmap)) != ERROR_SUCCESS)
    {
        LogMessageA(Error, "[%s] Load32BppBitmapFromFile failed!", __FUNCTION__);

        MessageBoxA(NULL, "Load32BppBitmapFromFile failed!", "Error!", MB_ICONERROR | MB_OK);

        goto Exit;
    }

    if (LoadTilemapFromFile(".\\Assets\\Maps\\Overworld01.tmx", &gOverWorld01.TIleMap) != ERROR_SUCCESS)
    {
        LogMessageA(Error, "[%s] LoadTilemapFromFile failed!", __FUNCTION__);

        MessageBoxA(NULL, "LoadTilemapFromFile failed!", "Error!", MB_ICONERROR | MB_OK);

        goto Exit;
    }

    if (InitializeSoundEngine() != S_OK)
    {
        MessageBoxA(NULL, "InitializeSoundEngine failed!", "Error!", MB_ICONERROR | MB_OK);

        goto Exit;
    }

    if (LoadAssetFromArchive(ASSET_FILE, "MenuSelect.wav", RESOURCETYPE_WAV, &gSoundMenuNavigate) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "LoadAssetFromArchive failed!", "Error!", MB_ICONERROR | MB_OK);

        goto Exit;
    }

    if (LoadAssetFromArchive(ASSET_FILE, "MenuChooser.wav", RESOURCETYPE_WAV, &gSoundMenuChoose) != ERROR_SUCCESS)
    {
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

    if (LoadOggFromFile(".\\Assets\\Overworld01.ogg", &gMusicOverworld01) != ERROR_SUCCESS)
    {
        MessageBoxA(NULL, "LoadOggFromFile failed!", "Error!", MB_ICONERROR | MB_OK);

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

    gPlayer.ScreenPos.x = 192;

    gPlayer.ScreenPos.y = 64;

    gPlayer.WorldPos.x = 192;

    gPlayer.WorldPos.y = 64;

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

void BlitTileMapToBuffer(_In_ GAMEBITMAP* GameBitmap)
{
    int32_t StartingScreenPixel = ((GAME_RES_WIDTH * GAME_RES_HEIGHT) - GAME_RES_WIDTH);

    int32_t StartingBitmapPixel = ((GameBitmap->Bitmapinfo.bmiHeader.biHeight * GameBitmap->Bitmapinfo.bmiHeader.biWidth) - GameBitmap->Bitmapinfo.bmiHeader.biWidth) + gCamera.x - (GameBitmap->Bitmapinfo.bmiHeader.biWidth * gCamera.y);

    int32_t MemoryOffset = 0;

    int32_t BitmapOffset = 0;

#ifdef AVX
    __m256i BitmapOctaPixel;

    for (int16_t YPixel = 0; YPixel < GAME_RES_HEIGHT; YPixel++)
    {
        for (int16_t XPixel = 0; XPixel < GAME_RES_WIDTH; XPixel += 8)
        {
            MemoryOffset = StartingScreenPixel + XPixel - (GAME_RES_WIDTH * YPixel);

            BitmapOffset = StartingBitmapPixel + XPixel - (GameBitmap->Bitmapinfo.bmiHeader.biWidth * YPixel);

            BitmapOctaPixel = _mm256_loadu_si256((PIXEL32*)GameBitmap->Memory + BitmapOffset);

            _mm256_store_si256((PIXEL32*)gBackBuffer.Memory + MemoryOffset, BitmapOctaPixel);//

        }
    }

#elif defined SSE2
    __m128i BitmapQuadPixel;

    for (int16_t YPixel = 0; YPixel < GAME_RES_HEIGHT; YPixel++)
    {
        for (int16_t XPixel = 0; XPixel < GAME_RES_WIDTH; XPixel += 4)
        {
            MemoryOffset = StartingScreenPixel + XPixel - (GAME_RES_WIDTH * YPixel);

            BitmapOffset = StartingBitmapPixel + XPixel - (GameBitmap->Bitmapinfo.bmiHeader.biWidth * YPixel);

            BitmapQuadPixel = _mm_load_si128((PIXEL32*)GameBitmap->Memory + BitmapOffset);

            _mm_store_si128((PIXEL32*)gBackBuffer.Memory + MemoryOffset, BitmapQuadPixel);
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

    LogMessageA(Informational, "[% s] MusicVolume is %.1f.", __FUNCTION__, (float)gRegistryParams.MusicVolume / 100);

    gMusicVolume = (float)gRegistryParams.MusicVolume / 100.f;

    gSFXVolume = (float)gRegistryParams.SFXVolume / 100.f;

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


            break;
        }
        case RESOURCETYPE_TILEMAP:
        {


            break;
        }
        case RESOURCETYPE_BMPX:
        {


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
