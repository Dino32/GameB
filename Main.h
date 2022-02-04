#pragma once

#pragma warning (disable: 4820)

#pragma warning(disable: 5045)

typedef struct GAMEBITMAP 
{

	BITMAPINFO Bitmapinfo; // includes height, width and others (data structure)
	void* Memory;

} GAMEBITMAP;

typedef struct PIXEL32 
{

	uint8_t Blue; // same as unsigned char, comes from <stdint.h>
	uint8_t Green;
	uint8_t Red;
	uint8_t Alpha;

} PIXEL32;

typedef struct GAMEPERFDATA 
{

	uint64_t TotalFramesRednered;

	uint32_t RawFramesPerSecondAverage;

	uint32_t CoockedFramesPerSecondAverage;

	LARGE_INTEGER Perffrequency;

	LARGE_INTEGER FrameStart;

	LARGE_INTEGER FrameEnd;

	LARGE_INTEGER ElapsedMicrosecondsPerFrame;
	
	MONITORINFO MonitorInfo;

	int32_t MonitorWidth;

	int32_t MonitorHeight;

} GAMEPERFDATA;

INT __stdcall WinMain(HINSTANCE Instance, HINSTANCE PreviousInstance, PSTR CommandLine, INT CommandShow);

LRESULT CALLBACK MainWndowProc(_In_ HWND WindowHandle, _In_ UINT Message, _In_ WPARAM wParam, _In_ LPARAM lParam);

DWORD CreateMainGameWindow(void);

BOOL GameIsAlreadyRunning(void);

void ProcessPlayerInput(void);

void RednerFrameGraphics(void);