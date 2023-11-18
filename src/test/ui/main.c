#include "..\..\platform.h"
#include "..\..\intrinsics.h"
#include "..\..\asset.h"
#include "memory.h"

#include <stdlib.h>
#include <stdio.h>
#include <conio.h>

#include "renderer.h"
#include "string.h"
#include "ui.h"

#define MAX_NUM_WIDGETS 4096

typedef struct State State;
struct State {
    MemoryArena permanent_arena;
    MemoryArena frame_arena;
    UI *ui;
    Font font;
};

global Font *g_font;
global RenderBuffer *g_buffer;
global Input *g_input;
global UI *ui;
global v2 g_mouse_pos;
global v2 g_prev_mouse_pos;

global State *g_state;
Platform *platform;

#include "renderer.c"
#include "ui.c"

function void
InitializeFont(State *state, Font *font) {
    for (u32 i = 0; i < 128; ++i) {
        char *entry = ascii_table[i];
        if (StringLength(entry)) {
            Bitmap *bitmap = &font->glyphs[i].bitmap;
            s32 height = 8;
            s32 width = 0;
            for (s32 y = 0; y < 8; ++y) {
                for (s32 x = 0; x < 7; ++x) {
                    char ch = entry[y * 7 + x];
                    if (ch == '0') {
                        if ((x + 1) > width) width = (x + 1);
                    }
                }
            }
            Assert(width != 0);
            bitmap->height = height;
            bitmap->width = width;
            bitmap->pixels = PushArray(&state->permanent_arena, width * height, u32);
            for (s32 y = 0; y < 8; ++y) {
                for (s32 x = 0; x < width; ++x) {
                    char ch = entry[y * 7 + x];
                    if (ch == '0') {
                        bitmap->pixels[(7 - y) * width + x] = 0xffffffff;
                    }
                }
            }
        }
    }
}

function void
InitializeUI(State *state) {
    state->ui = PushStruct(&state->permanent_arena, UI);

    state->ui->window_table.length = 4096;
    state->ui->window_table.windows = PushArray(&state->permanent_arena, state->ui->window_table.length, UI_Window *);

    state->ui->widget_table.length = 16384;
    state->ui->widget_table.widgets = PushArray(&state->permanent_arena, state->ui->widget_table.length, UI_Widget *);

    state->ui->active_windows.max_count = 16;
    state->ui->active_windows.windows = PushArray(&state->permanent_arena, state->ui->active_windows.max_count, UI_Window *);
}

GAME_UPDATE_AND_RENDER(GameUpdateAndRender) {
    g_memory = memory;

    BEGIN_TIMED_FUNCTION();

    g_buffer = buffer;
    g_input = input;
    platform = &memory->platform;
    input->mouse_x *= buffer->width;
    input->mouse_y *= buffer->height;
    g_mouse_pos = V2((f32)input->mouse_x, (f32)input->mouse_y);

    State *state = (State *)memory->permanent_storage;
    g_state = state;
    if (!memory->is_initialized) {
        InitializeArena(&state->permanent_arena, memory->permanent_storage_size - sizeof(State), (u8 *)memory->permanent_storage + sizeof(State));
        InitializeArena(&state->frame_arena, MEGABYTES(1024), (u8 *)memory->transient_storage);
        InitializeUI(state);
        InitializeFont(state, &state->font);
        memory->is_initialized = true;
    }
    g_font = &state->font;
    ui = state->ui;

    ClearScreen(buffer, V4(0, 0, 0, 0));

    UI_Begin();

    UI_BeginWindow("My window##1", 0, 0, 500, 500);
    UI_Button("Test2");
    UI_Button("Test");

    UI_BeginWindow("My window##2", 200, 200, 500, 500);

    UI_NextWidth(UI_Pixels(100));
    UI_NextHeight(UI_Pixels(100));
    local_persist b32 show_window = false;
    if (UI_Button("Test").clicked) {
        show_window = !show_window;
    }

    if (show_window) {
        UI_BeginWindow("Hehhehe", 0, 0, 100, 100);
        for(u32 i = 0; i < 10; ++i) {
            UI_Text("Hehe");
        }
        UI_Button("Test##1");
        UI_EndWindow();
    }

    UI_Button("Test##1");

    UI_Button("Test##2");

    UI_EndWindow();

    UI_EndWindow();

    UI_End();

    END_TIMED_FUNCTION();

    for (u32 i = 0; i < 256; ++i) {
        g_counters[i].prev_cycle_count = g_counters[i].cycle_count;
        g_counters[i].cycle_count = 0;
    }

    g_prev_mouse_pos = g_mouse_pos;

    ZeroArena(&state->frame_arena);
}