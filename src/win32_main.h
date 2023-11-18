#ifndef WIN32_MAIN_H
#define WIN32_MAIN_H

typedef enum Win32FaderState
{
    WIN32_FADE_FADING_IN,
    WIN32_FADE_WAITING_FOR_SHOW,
    WIN32_FADE_INACTIVE,
    WIN32_FADE_FADING_GAME,
    WIN32_FADE_FADING_OUT,
    WIN32_FADE_WAITING_FOR_CLOSE,
} Win32FaderState;

typedef struct Win32Fader
{
    Win32FaderState state;
    HWND window;
    f32 alpha;
} Win32Fader;

typedef struct Win32RenderBuffer
{
    BITMAPINFO info;
    void *memory;
    s32 width;
    s32 height;
    s32 pitch;
    s32 bytes_per_pixel;
} Win32RenderBuffer;

typedef struct Win32WindowDimension
{
    s32 width;
    s32 height;
} Win32WindowDimension;

typedef struct Win32GameCode
{
    HMODULE game_code_dll;

    // IMPORTANT(hampus): Either of these callbacks 
    // can be 0! Check before calling.
    Game_Update_And_Render *update_and_render;
    FILETIME dll_last_write_time;

    b32 valid;
} Win32GameCode;

#define WIN32_STATE_FILE_NAME_COUNT MAX_PATH 
typedef struct Win32ReplayBuffer
{
    HANDLE file_handle;
    HANDLE memory_map;
    char file_name[WIN32_STATE_FILE_NAME_COUNT];
    void *memory_block;
} Win32ReplayBuffer;

typedef struct Win32State
{
    u64 total_size;
    void *game_memory_block;
    Win32ReplayBuffer replay_buffers[4];

    HANDLE recording_handle;
    s32 input_recording_index;

    HANDLE play_back_handle;
    s32 input_playing_index;

    FILETIME last_assets_write_time;

    char exe_file_name[WIN32_STATE_FILE_NAME_COUNT];
    char *one_past_exe_file_name_slash;
} Win32State;

#endif
