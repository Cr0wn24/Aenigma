#ifndef PLATFORM_H
#define PLATFORM_H

/*
NOTE(hampus):

AENIGMA_INTERNAL:
0 - Build for public release
1 - Build for developer only

AENIGMA_SLOW:
0 - No slow code allowed
1 - Slow code welcome

*/

#define AENIGMA_INTERNAL 1
#define AENIGMA_SLOW     1
#define AENIGMA_RELEASE  0
#define AENIGMA_SIMD     1
#define AENIGMA_DEMO     1  
#define AENIGMA_WIN32    1

#define _CRT_SECURE_NO_WARNINGS

#ifndef COMPILER_MSVC
#define COMPILER_MSVC 0
#endif

#ifndef COMPILER_LLVM
#define COMPILER_LLVM 0
#endif

#if !COMPILER_LLVM && !COMPILER_MSVC

#if _MSC_VER
#undef COMPILER_MSVC
#define COMPILER_MSVC 1
#else
#undef COMPILER_LLVM
#define COMPILER_LLVM 1
#endif

#endif

#define local_persist   static
#define function        static
#define global          static

#define false 0
#define true 1

#if AENIGMA_SLOW
#define Assert(expr) if (!(expr)) { *(s32 *)0 = 0; }
#else
#define Assert(expr)
#endif

#define InvalidCodePath Assert(false)
#define InvalidCase default: { Assert(false); } break;
#define NOT_IMPLEMENTED Assert(false)

#include "types.h"
#include "config.h"
#include "string.c"

#include "debug.h"

typedef struct ThreadContext {
    s32 placeholder;
} ThreadContext;

#define PI32 3.141592f
#define PI64 3.141592653589793

#define KILOBYTES(n) ((n)*1024LL)
#define MEGABYTES(n) (KILOBYTES(n)*1024LL)
#define GIGABYTES(n) (MEGABYTES(n)*1024LL)
#define TERABYTES(n) (GIGABYTES(n)*1024LL)

#define ArrayCount(array) (sizeof(array) / sizeof((array)[0]))

#define Swap(a, b, type) \
{ \
type temp = a; \
a = b; \
b = temp; \
}

typedef struct ReadFileResult {
    u32 content_size;
    void *contents;
} ReadFileResult;

typedef enum CursorType {
    CursorType_Arrow,
    CursorType_Hand,
    CursorType_Slider,
} CursorType;

#define PLATFORM_READ_ENTIRE_FILE(name) ReadFileResult name(ThreadContext *thread, char *file_name)
typedef PLATFORM_READ_ENTIRE_FILE(PlatformReadEntireFile);

#define PLATFORM_FREE_FILE_MEMORY(name) void name(ThreadContext *thread, void *memory)
typedef PLATFORM_FREE_FILE_MEMORY(PlatformFreeMemory);

#define PLATFORM_WRITE_ENTIRE_FILE(name) b32 name(ThreadContext *thread, char *file_name, u32 memory_size, void *memory)
typedef PLATFORM_WRITE_ENTIRE_FILE(PlatformWriteEntireFile);

#define PLATFORM_FIND_FILES_IN_DIRECTORY(name) void name(char dest[32][256], u32 dest_size, char *file_name)
typedef PLATFORM_FIND_FILES_IN_DIRECTORY(PlatformFindFilesInDirectory);

#define PLATFORM_DELETE_FILE(name) void name(char *path)
typedef PLATFORM_DELETE_FILE(PlatformDeleteFile);

#define PLATFORM_TOGGLE_CURSOR(name) void name()
typedef PLATFORM_TOGGLE_CURSOR(PlatformToggleCursor);

#define PLATFORM_RESIZE_RENDER_BUFFER(name) void name(s32 new_width, s32 new_height)
typedef PLATFORM_RESIZE_RENDER_BUFFER(PlatformResizeRenderBuffer);

#define PLATFORM_ALLOC_MEMORY(name) void *name(size_t size)
typedef PLATFORM_ALLOC_MEMORY(PlatformAllocateMemory);

#define PLATFORM_SET_CURSOR_TYPE(name) void name(CursorType type)
typedef PLATFORM_SET_CURSOR_TYPE(PlatformSetCursorType);

typedef struct Platform {
    PlatformFindFilesInDirectory *FindFilesInDirectory;
    PlatformDeleteFile *DeleteFile;
    PlatformToggleCursor *ToggleCursor;
    PlatformSetCursorType *SetCursorType;

    PlatformReadEntireFile *ReadEntireFile;
    PlatformFreeMemory *FreeMemory;
    PlatformWriteEntireFile *WriteEntireFile;

    PlatformResizeRenderBuffer *ResizeRenderBuffer;
    PlatformAllocateMemory *AllocateMemory;
} Platform;

#define KeyDown(input, key) (input->key_states[key].is_down)
#define KeyWasDown(input, key) (input->key_states[key].was_down)
#define KeyPressed(input, key) ((KeyDown(input, key)) && (!KeyWasDown(input, key)))
#define KeyReleased(input, key) ((!KeyDown(input, key)) && (KeyWasDown(input, key)))

enum KeyCode {
    KeyCode_W,
    KeyCode_S,
    KeyCode_A,
    KeyCode_D,

    KeyCode_Q,
    KeyCode_E,
    KeyCode_R,
    KeyCode_O,
    KeyCode_N,
    KeyCode_P,
    KeyCode_L,

    KeyCode_Escape,
    KeyCode_Enter,
    KeyCode_Backspace,
    KeyCode_Space,
    KeyCode_Shift,

    KeyCode_Up,
    KeyCode_Down,
    KeyCode_Left,
    KeyCode_Right,

    KeyCode_MouseLeft,
    KeyCode_MouseMiddle,
    KeyCode_MouseRight,

    KeyCode_0,
    KeyCode_1,
    KeyCode_2,
    KeyCode_3,
    KeyCode_4,
    KeyCode_5,
    KeyCode_6,
    KeyCode_7,
    KeyCode_8,
    KeyCode_9,

    KeyCode_F1,
    KeyCode_F2,
    KeyCode_F3,
    KeyCode_F4,
    KeyCode_F5,
    KeyCode_F6,
    KeyCode_F7,
    KeyCode_F8,
    KeyCode_F9,
    KeyCode_F10,
    KeyCode_F11,
    KeyCode_F12,

    KeyCode_Delete,

    KeyCode_Console,

    KeyCode_COUNT
};

typedef struct RenderBuffer {
    void *memory;
    s32 width;
    s32 height;
    s32 pitch;
    s32 bytes_per_pixel;
} RenderBuffer;

typedef struct KeyState {
    b32 was_down;
    b32 is_down;
} KeyState;

#pragma pack(push, 1)
typedef struct Input {
    union {
        KeyState key_states[KeyCode_COUNT];
        struct {
            KeyState w;
            KeyState s;
            KeyState a;
            KeyState d;
            KeyState q;
            KeyState e;
            KeyState r;
            KeyState o;
            KeyState n;
            KeyState p;
            KeyState l;

            KeyState escape;
            KeyState enter;
            KeyState back;
            KeyState space;
            KeyState shift;

            KeyState up;
            KeyState down;
            KeyState left;
            KeyState right;

            KeyState left_mouse_btn;
            KeyState middle_mouse_btn;
            KeyState right_mouse_btn;

            KeyState num0;
            KeyState num1;
            KeyState num2;
            KeyState num3;
            KeyState num4;
            KeyState num5;
            KeyState num6;
            KeyState num7;
            KeyState num8;
            KeyState num9;

            KeyState f1;
            KeyState f2;
            KeyState f3;
            KeyState f4;
            KeyState f5;
            KeyState f6;
            KeyState f7;
            KeyState f8;
            KeyState f9;
            KeyState f10;
            KeyState f11;
            KeyState f12;

            KeyState delete;

            KeyState console;

            KeyState terminator;
        };
    };

    s32 mouse_wheel;
    f32 mouse_x;
    f32 mouse_y;
    b32 ctrl;

    f32 dt;
    f32 dt_raw;

    b32 alt_down;

    char last_char;
} Input;
#pragma pack(pop)

typedef struct GameMemory {
    u64 permanent_storage_size;
    void *permanent_storage; // @Important: REQUIRED to be cleared to zero at startup

    u64 transient_storage_size;
    void *transient_storage; // @Important: REQUIRED to be cleared to zero at startup

    Platform platform;

    b32 exit_requested;

    b32 assets_folder_changed;

    DebugCycleCounter counter[256];

    b32 is_initialized;
} GameMemory;

#if AENIGMA_INTERNAL
GameMemory *g_memory;
#endif

#define GAME_UPDATE_AND_RENDER(name) void name(ThreadContext *thread, GameMemory *memory, Input *input, RenderBuffer *buffer)

typedef GAME_UPDATE_AND_RENDER(Game_Update_And_Render);

#endif