#include "..\..\platform.h"
#include "..\..\intrinsics.h"
#include "..\..\asset.h"
#include "memory.h"

#include <stdlib.h>
#include <stdio.h>
#include <conio.h>

#include "renderer.h"
#include "ui.h"

global Font g_font;

#include "renderer.c"

#define MAX_NUM_WIDGETS 4096

Platform *platform;

#include "ui.c"

function void
InitializeFont(Font *font) {
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
            bitmap->pixels = PushArray(&ui->permanent_arena, width * height, u32);
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

GAME_UPDATE_AND_RENDER(GameUpdateAndRender) {
    g_memory = memory;
    BEGIN_TIMED_FUNCTION();
    g_buffer = buffer;
    g_input = input;
    platform = &memory->platform;
    input->mouse_x *= buffer->width;
    input->mouse_y *= buffer->height;

    ui = (UI *)memory->permanent_storage;
    if (!ui->frame_arena.base) {
        InitializeArena(&ui->frame_arena, 1024 * 1024 * 1024, memory->transient_storage);

        InitializeArena(&ui->permanent_arena, memory->permanent_storage_size - sizeof(UI),
                        (u8 *)memory->permanent_storage + sizeof(UI));

        InitializeFont(&g_font);

        ui->boxes = PushArray(&ui->permanent_arena, MAX_NUM_WIDGETS, UI_Box *);

        for (u32 i = 0; i < MAX_NUM_WIDGETS; ++i) {
            ui->boxes[i] = PushStruct(&ui->permanent_arena, UI_Box);
        }

        // TODO(hampus): Take this from the frame arena instead,
        ui->boxes_to_render = PushArray(&ui->permanent_arena, MAX_NUM_WIDGETS, UI_Box *);

        for (u32 i = 0; i < MAX_NUM_WIDGETS; ++i) {
            ui->boxes_to_render[i] = PushStruct(&ui->permanent_arena, UI_Box);
        }
    }

    g_clip_rect = (Rect){ 0, 0, (f32)g_buffer->width, (f32)g_buffer->height };

    ui->curr_layout_state = &ui->layout_states[0];

    ClearScreen(buffer, V4(0.1f, 0.1f, 0.1f, 1.0f));

    UI_Begin();

    UI_BeginWindow("My Window 1!", 0, 0, 400, 300, true);

    local_persist b32 test_bool = false;
    local_persist b32 show_counters = false;

    if (UI_Button("Performance").clicked) {
        test_bool = !test_bool;
    }

    if (test_bool) {
        UI_PushIndent(20.0f);

        UI_Text("fps: %d", (s32)(1.0f / input->dt_raw + 0.5f));
        UI_Text("ms: %.02f", input->dt_raw * 1000.0f);
        UI_Text("width: %d", buffer->width);
        UI_Text("height: %d", buffer->height);

        UI_PopIndent();
    }

    if (UI_Button("Test123").clicked) {
    }

    UI_SameLine();
    if (UI_Button("Test1").clicked) {
    }

    UI_SameLine();
    if (UI_Button("Test2").clicked) {
    }

    local_persist v3 pos = { 0 };
    UI_Slider("X", &pos.x, -20.0f, 20.0f);
    UI_SameLine();
    UI_Slider("Y", &pos.y, -20.0f, 20.0f);
    UI_SameLine();
    UI_Slider("Z", &pos.z, -20.0f, 20.0f);

    UI_Divider();

    local_persist v4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
    UI_ColorPicker("My color picker", &color);

    if (UI_Check("Bool", show_border).clicked) {
        show_border = !show_border;
    }
    UI_InputBox(0);

    UI_Text("hehehehhe0");
    UI_SameLine();
    UI_Text("hehehehhe1");

    UI_Text("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    UI_Text("abcdefghijklmnopqrstuvwxyz");
    UI_Text("0123456789");
    UI_Text("!#%%&/()=?{}[]");

    UI_PopWindow();

    UI_BeginWindow("Controls", (f32)buffer->width - 250, 0, 250, 400, true);

    UI_Text("F1  - Toggle border");
    UI_Text("F2  -");
    UI_Text("F3  -");
    UI_Text("F4  -");
    UI_Text("F5  -");
    UI_Text("F6  -");
    UI_Text("F7  -");
    UI_Text("F8  -");
    UI_Text("F9  -");
    UI_Text("F10 -");
    UI_Text("F11 -");
    UI_Text("F12 -");

    UI_PopWindow();

    UI_BeginWindow("Performance", 0, 400, 600, 140, true);

    for (u32 i = 0; i < 256; ++i) {
        if (g_counters[i].prev_cycle_count) {
            UI_Text("%s: %llu (%.02f%%)\n", g_counters[i].function_name, g_counters[i].prev_cycle_count, 100.0f * (f32)g_counters[i].prev_cycle_count / (f32)g_counters[__COUNTER__ - 1].prev_cycle_count);
            //UI_Text("%-30s: %-12llu (%6.02f%%)\n", g_counters[i].function_name, g_counters[i].prev_cycle_count, 100.0f * (f32)g_counters[i].prev_cycle_count / (f32)g_counters[__COUNTER__ - 1].prev_cycle_count);
        }
    }

    UI_PopWindow();

    UI_End();

    DrawRectAlpha(g_buffer, V2(500 + pos.x, 200 + pos.y), V2(600 + pos.x, 300 + pos.y), color);

    ZeroArena(&ui->frame_arena);

    ui->curr_frame_index++;

    END_TIMED_FUNCTION();

    ui->prev_mouse_pos = V2((f32)g_input->mouse_x, (f32)g_input->mouse_y);

    for (u32 i = 0; i < 256; ++i) {
        g_counters[i].prev_cycle_count = g_counters[i].cycle_count;
        g_counters[i].cycle_count = 0;
    }

}