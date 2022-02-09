#pragma once

#pragma warning (disable: 4820)

#pragma warning(disable: 5045)

#ifdef _DEBUG
	#define ASSERT(Expression, Message) if (!(Expression)) { *(int*)0 = 0; }; // crashing the game 
#else
	#define ASSERT(Expression, Message) ((void)0);
#endif
#define GAME_NAME "Game_B"

#define GAME_RES_WIDTH   384

#define GAME_RES_HEIGHT   240

#define GAME_VER "0.9a"

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

#define FONT_SHEET_CHARACTERS_PER_ROW 98

#define NUMBER_OF_SFX_SOURCE_VOICES 4


typedef enum LOGLEVEL
{
	None = 0,

	Error = 1,

	Warning = 2,

	Informational = 3,

	Debug = 4,
} LOGLEVEL;

typedef enum GAMESATE
{
	GAMESTATE_OPENINGSPLASHSCREEN,

	GAMESTATE_TITLESCREEN,

	GAMESTATE_OVERWORLD,

	GAMESTATE_BATTLE,

	GAMESATTE_OPTIONSSCREEN,

	GAMESTATE_EXITYESNOSCREEN

} GAMESTATE;

#define LOG_FILE_NAME "GAMEB.log"


typedef LONG(NTAPI* _NtQueryTimerResolution) (OUT PULONG MinimumResolution, OUT PULONG MaximumResolution, OUT PULONG CurrentResolution);

_NtQueryTimerResolution NtQueryTimerResolution;

typedef struct GAMEBITMAP 
{

	BITMAPINFO Bitmapinfo; // includes height, width and others (data structure)
	void* Memory;

} GAMEBITMAP;

typedef struct GAMESOUND
{
	WAVEFORMATEX WaveFormat;

	XAUDIO2_BUFFER Buffer;

} GAMESOUND;

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

typedef struct GAMEINPUT
{
	int16_t EscapeKeyIsDown;

	int16_t DebugKeyIsDown;

	int16_t LeftKeyIsDown;

	int16_t RightKeyIsDown;

	int16_t UpKeyIsDown;

	int16_t DownKeyIsDown;

	int16_t DebugKeyWasDown;

	int16_t LeftKeyWasDown;

	int16_t RightKeyWasDown;

	int16_t UpKeyWasDown;

	int16_t DownKeyWasDown;

} GAMEINPUT;

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

typedef struct REGISTRYPARAMS
{
	DWORD LogLevel;
} REGISTRYPARAMS;

INT __stdcall WinMain(HINSTANCE Instance, HINSTANCE PreviousInstance, PSTR CommandLine, INT CommandShow);

LRESULT CALLBACK MainWndowProc(_In_ HWND WindowHandle, _In_ UINT Message, _In_ WPARAM wParam, _In_ LPARAM lParam);

DWORD CreateMainGameWindow(void);

BOOL GameIsAlreadyRunning(void);

void ProcessPlayerInput(void);

void RednerFrameGraphics(void);

DWORD Load32BppBitmapFromFile(_In_ char* FileName, _Inout_ GAMEBITMAP* GameBitmap);

DWORD InitializeHero(void);

void Blit32BppBitmapToBuffer(_In_ GAMEBITMAP* GameBitmap, _In_ uint16_t x, _In_ uint16_t y);

void BlitStringToBuffer(_In_ char* String, _In_ GAMEBITMAP* FontSheet, _In_ PIXEL32 Color, _In_ uint16_t x, _In_ uint16_t y);

DWORD LoadRegistryParameters(void);

void LogMessageA(_In_ DWORD LogLevel, _In_ char* Message, _In_ ...);

void DrawDebugInfo(void);

void FindFirstConnctedGamepad(void);

void DrawOpeningSplashScreen(void);

void DrawTitleScreen(void);

void PPI_OpeningSplasheScreen(void);

void PPI_TitleScreen(void);

void PPI_Overworld(void);

HRESULT InitializeSoundEngine(void);

DWORD LoadWaveFromFile(_In_ char* FileName , _Inout_ GAMESOUND* GameSound);

void PlayGameSound(_In_ GAMESOUND* GameSound);

#ifdef SIMD
void ClearScreen(_In_ __m128i* Color);
#else
void ClearScreen(_In_ PIXEL32* Pixel);
#endif