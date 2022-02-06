#pragma once

#pragma warning (disable: 4820)

#pragma warning(disable: 5045)

#define SIMD


typedef LONG(NTAPI* _NtQueryTimerResolution) (OUT PULONG MinimumResolution, OUT PULONG MaximumResolution, OUT PULONG CurrentResolution);

_NtQueryTimerResolution NtQueryTimerResolution;

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

	float RawFPSAverage;

	float CoockedFPSAverage;

	int64_t Perffrequency;
	
	MONITORINFO MonitorInfo;

	int32_t MonitorWidth;

	int32_t MonitorHeight;

	BOOL DisplayDegubInfo;

	LONG MinimumTimerResolution;
	
	LONG MaximumTimerResolution;
	
	LONG CurrentTimerResolution;

	DWORD HandleCount;

	PROCESS_MEMORY_COUNTERS_EX MemInfo;

	uint16_t CPUCount;

	SYSTEM_INFO SystemInfo;

	int64_t PreviousSystemTime;

	int64_t CurrentSystemTime;

	double CPUPercent;

} GAMEPERFDATA;

typedef struct PLAYER
{
	int32_t ScreenPosX;
	
	int32_t ScreenPosY;

	char Name[12];

	int32_t HP;

	int32_t Strength;

	int32_t MP; // Magic Power

} PLAYER;

INT __stdcall WinMain(HINSTANCE Instance, HINSTANCE PreviousInstance, PSTR CommandLine, INT CommandShow);

LRESULT CALLBACK MainWndowProc(_In_ HWND WindowHandle, _In_ UINT Message, _In_ WPARAM wParam, _In_ LPARAM lParam);

DWORD CreateMainGameWindow(void);

BOOL GameIsAlreadyRunning(void);

void ProcessPlayerInput(void);

void RednerFrameGraphics(void);

#ifdef SIMD
void ClearScreen(_In_ __m128i* Color);
#else
void ClearScreen(_In_ PIXEL32* Pixel);
#endif