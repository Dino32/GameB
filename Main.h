#pragma once

#pragma warning(disable: 4668)

#pragma warning(disable: 4820)

#pragma warning(disable: 5045)

#pragma warning(disable: 4244)

#pragma warning(push, 3)

#include <windows.h>

#include <xaudio2.h>  // Audio library

#pragma comment(lib, "XAudio2.lib")

#include <stdio.h>

#include <stdint.h>

#include <xinput.h>    // Xbox input library

#pragma comment(lib, "XInput.lib")

#include <Psapi.h>

#pragma comment(lib, "Winmm.lib")

//#define AVX

#ifdef AVX

#include <immintrin.h>

#elif defined SSE2

#include <emmintrin.h>

#endif

#pragma warning(pop)

#include "Tiles.h"


#ifdef _DEBUG
	#define ASSERT(Expression, Message) if (!(Expression)) { *(int*)0 = 0; }; // crashing the game 
#else
	#define ASSERT(Expression, Message) ((void)0);
#endif
#define GAME_NAME "Game_B"

#define GAME_RES_WIDTH   384

#define GAME_RES_HEIGHT   240

#define GAME_VER "0.9a"

#define GAME_BPP 32

#define GAME_DRAWING_AREA_MEMORY_SIZE (GAME_RES_WIDTH  * GAME_RES_HEIGHT * (GAME_BPP  / 8))

#define CALCULATE_AVG_FPS_EVERY_X_FRAMES 120

#define TARGET_MICROSECONDS_PER_FRAME 16667ULL

//#define SIMD

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

	GAMESTATE_OPTIONSSCREEN,

	GAMESTATE_CHARACTERNAMING,

	GAMESTATE_EXITYESNOSCREEN,

	GAMESTATE_GAMEPADUNPLUGGED,

	GAMESTATE_SAVEGAME,

	GAMESTATE_NEWGAMEPROMPT

} GAMESTATE;

typedef enum RESOURCETYPE
{
	RESOURCETYPE_WAV,

	RESOURCETYPE_OGG,

	RESOURCETYPE_TILEMAP,

	RESOURCETYPE_BMPX

} RESOURCETYPE;

#define LOG_FILE_NAME "GAMEB.log"

#define ASSET_FILE "Assets.dat"



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
	
	uint8_t MaxScaleFactor;

	uint8_t CurrentScaleFactor;

} GAMEPERFDATA;

typedef struct GAMEINPUT
{
	int16_t EscapeKeyIsDown;

	int16_t DebugKeyIsDown;

	int16_t LeftKeyIsDown;

	int16_t RightKeyIsDown;

	int16_t UpKeyIsDown;

	int16_t DownKeyIsDown;

	int16_t ChooseKeyIsDown;

	int16_t PortalIsDown;

	int16_t EscapeKeyWasDown;

	int16_t DebugKeyWasDown;

	int16_t LeftKeyWasDown;

	int16_t RightKeyWasDown;

	int16_t UpKeyWasDown;

	int16_t DownKeyWasDown;

	int16_t ChooseKeyWasDown;

	int16_t PortalWasDown;

} GAMEINPUT;

typedef struct UPOINT
{
	uint16_t x;

	uint16_t y;

} UPOINT;

typedef struct HERO
{
	UPOINT ScreenPos;

	UPOINT WorldPos;

	uint8_t MovementRemaining;

	uint8_t Direction;

	uint8_t CurrentArmor;

	uint8_t SpriteIndex;

	char Name[9];

	GAMEBITMAP Sprite[3][12];

	int32_t HP;

	int32_t Strength;

	int32_t MP; // Magic Power

	BOOL Active;

	uint64_t StepsTaken;

	BOOL HasPlayerMoveSincePortal;

} HERO;

typedef struct REGISTRYPARAMS
{
	DWORD LogLevel;

	DWORD SFXVolume;

	DWORD MusicVolume;

	DWORD ScaleFactor;

	DWORD WorldX;

	DWORD WorldY;

	DWORD ScreenX;

	DWORD ScreenY;

	DWORD CameraX;

	DWORD CameraY;

	DWORD CurrentArmor;

	DWORD MovementsRemaining;

	DWORD Direction;

	DWORD GameSaved;

	char Name[9];

	char* Ime[9];

} REGISTRYPARAMS;

typedef struct MENUITEM
{
	char* Name;

	int16_t x;

	int16_t y;

	BOOL Enabled;

	void(*Action)(void);

} MENUITEM;

typedef struct MENU
{
	char* Name;

	uint8_t SelectedItem;

	uint8_t ItemCount;

	MENUITEM** Items;

} MENU;

typedef struct TILEMAP
{
	uint16_t Width;

	uint16_t Height;
	
	uint8_t** Map;

} TILEMAP;

typedef struct GAMEMAP
{
	TILEMAP TIleMap;

	GAMEBITMAP GameBitmap;

} GAMEMAP;

GAMEPERFDATA gPerformanceData;

GAMEBITMAP gBackBuffer;

GAMESTATE gCurrentGameState;

GAMESTATE gPreviousGameState;

GAMEINPUT gGameInput;

GAMEBITMAP g6x7Font;

GAMESOUND gSoundSplashScreen;

GAMESOUND gSoundFadingScreen;

HERO gPlayer;

int8_t gGamepadID;

GAMESOUND gSoundMenuNavigate;

GAMESOUND gSoundMenuChoose;

GAMESOUND gMusicOverworld01;

GAMESOUND gMusicNoSound;

float gSFXVolume;

float gMusicVolume;

BOOL gGameIsRunning; // gloabal variable, its automatically initialized to zero (false)

HWND gGameWindow; // Wheteher the player has started or loaded a game

BOOL gWindowHasFocus;

BOOL gInputEnabled;

IXAudio2* gXAudio;

IXAudio2MasteringVoice* gXAudioMasteringVoice;

IXAudio2SourceVoice* gXAudioSFXSorceVoice[NUMBER_OF_SFX_SOURCE_VOICES];

IXAudio2SourceVoice* gXAudioMusicSourceVoice;

uint8_t gSFXSourceVoiceSelector;

REGISTRYPARAMS gRegistryParams;

XINPUT_STATE gGamepadState;

GAMEMAP gOverWorld01;

UPOINT gCamera;

HANDLE gAssetLoadingThreadHandle;

uint8_t gPassablieTiles[5];

RECT gCurrentArea;

RECT gOverwrldArea;

RECT gDungeon1Area;

// This event gets signalled after the most essential assets have been loaded 
// "Essential" means the assets required to render the splash screen
HANDLE gEssentialAssetsLoadedEvent;


INT __stdcall WinMain(HINSTANCE Instance, HINSTANCE PreviousInstance, PSTR CommandLine, INT CommandShow);

LRESULT CALLBACK MainWndowProc(_In_ HWND WindowHandle, _In_ UINT Message, _In_ WPARAM wParam, _In_ LPARAM lParam);

DWORD CreateMainGameWindow(void);

BOOL GameIsAlreadyRunning(void);

void ProcessPlayerInput(void);

void RednerFrameGraphics(void);

DWORD Load32BppBitmapFromFile(_In_ char* FileName, _Inout_ GAMEBITMAP* GameBitmap);

DWORD Load32BppBitmapFromMemoy(_In_ void* Buffer, _Inout_ GAMEBITMAP* GameBitmap);

DWORD InitializeHero(void);

void Blit32BppBitmapToBuffer(_In_ GAMEBITMAP* GameBitmap, _In_ int16_t x, _In_ int16_t y, _In_ int16_t BrightnessAdjustment);

void BlitStringToBuffer(_In_ char* String, _In_ GAMEBITMAP* FontSheet, _In_ PIXEL32 Color, _In_ uint16_t x, _In_ uint16_t y);

void BlitTileMapToBuffer(_In_ GAMEBITMAP* GameBitmap, _In_ int16_t BrightnessAdjustment);

DWORD LoadRegistryParameters(void);

DWORD SaveRegistryParametars(void);

void LogMessageA(_In_ DWORD LogLevel, _In_ char* Message, _In_ ...);

void DrawDebugInfo(void);

void FindFirstConnctedGamepad(void);

HRESULT InitializeSoundEngine(void);

DWORD LoadWaveFromFile(_In_ char* FileName , _Inout_ GAMESOUND* GameSound);

DWORD LoadWaveFromMemory(_In_ void* Buffer, _Inout_ GAMESOUND* GameSound);

void PlayGameSound(_In_ GAMESOUND* GameSound);

void PlayGameMusic(_In_ GAMESOUND* GameSound);

DWORD LoadTilemapFromFile(_In_ char* FileName, _Inout_ TILEMAP* TileMap);

DWORD LoadTilemapFromMemory(_In_ void* Buffer, uint32_t BufferSize, _Inout_ TILEMAP* TileMap);

DWORD LoadOggFromFile(_In_ char* FileName, _Inout_ GAMESOUND* GameSound);

DWORD LoadOggFromMemory(_In_ void* Buffer, _In_ uint32_t BufferSize, _Inout_ GAMESOUND* GameSound);

DWORD LoadAssetFromArchive(_In_ char* ArchiveName, _In_ char* AssetFileName, _In_ RESOURCETYPE ResourceType, _Inout_ void* Resource);

DWORD AssetLoadingThreadProc(_In_ LPVOID lpParam);

void InitializeGlobals(void);

#ifdef SIMD
void ClearScreen(_In_ __m128i* Color);
#else
void ClearScreen(_In_ PIXEL32* Pixel);
#endif


