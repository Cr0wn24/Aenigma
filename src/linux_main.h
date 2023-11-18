#ifndef LINUX_MAIN_H
#define LINUX_MAIN_H

typedef struct LinuxRenderBuffer
{
    XImage *ximage;
    void *memory;
    s32 width;
    s32 height;
    s32 pitch;
    s32 bytes_per_pixel;

} LinuxRenderBuffer;

typedef struct LinuxGameCode
{
    void *game_code_dll;
    Game_Update_And_Render *update_and_render;

    b32 valid;
} LinuxGameCode;

typedef struct LinuxState
{
    s64 total_size;
    void *game_memory_block;
} LinuxState;

#endif