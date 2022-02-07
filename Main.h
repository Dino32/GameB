#pragma once

#pragma warning (disable: 4820)

#pragma warning(disable: 5045)

#define SIMD

#define SUIT_0 0

#define SUIT_1 1

#define SUIT_2 2

#define FACING_DOWN_0 0

#define FACING_DOWN_1 1

#define FACING_DOWN_2 2

#define FACING_LEFT_0 3

#define FACING_LEFT_1 4

#define FACING_LEFT_2 5

#define FACING_RIGHT_0 6

#define FACING_RIGHT_1 7

#define FACING_RIGHT_2 8

#define FACING_UPWARD_0 9

#define FACING_UPWARD_1 10

#define FACING_UPWARD_2 11



#define DIRECTION_DOWN 0

#define DIRECTION_LEFT 3

#define DIRECTION_RIGTH 6

#define DIRECTION_UP 9


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

typedef struct HERO
{
	int32_t ScreenPosX;
	
	int32_t ScreenPosY;

	uint8_t MovementRemaining;

	uint8_t Direction;

	uint8_t CurrentArmor;

	uint8_t SpriteIndex;

	char Name[12];

	GAMEBITMAP Sprite[3][12];

	int32_t HP;

	int32_t Strength;

	int32_t MP; // Magic Power

} HERO;

INT __stdcall WinMain(HINSTANCE Instance, HINSTANCE PreviousInstance, PSTR CommandLine, INT CommandShow);

LRESULT CALLBACK MainWndowProc(_In_ HWND WindowHandle, _In_ UINT Message, _In_ WPARAM wParam, _In_ LPARAM lParam);

DWORD CreateMainGameWindow(void);

BOOL GameIsAlreadyRunning(void);

void ProcessPlayerInput(void);

void RednerFrameGraphics(void);

DWORD Load32BppBitmapFromFile(_In_ char* FileName, _Inout_ GAMEBITMAP* GameBitmap);

DWORD InitializeHero(void);

void Blit32BppBitmapToBuffer(_In_ GAMEBITMAP* GameBitmap, _In_ uint16_t x, _In_ uint16_t y);

#ifdef SIMD
void ClearScreen(_In_ __m128i* Color);
#else
void ClearScreen(_In_ PIXEL32* Pixel);
#endif