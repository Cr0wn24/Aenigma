// 
// ~~~~~~~~~~~~~~~~~~~~ NOTE(hampus): Keying ~~~~~~~~~~~~~~~~~~~~ 
//
global u32 g_crc32_lookup_table[256] =
{
    0x00000000,0x77073096,0xEE0E612C,0x990951BA,0x076DC419,0x706AF48F,0xE963A535,0x9E6495A3,0x0EDB8832,0x79DCB8A4,0xE0D5E91E,0x97D2D988,0x09B64C2B,0x7EB17CBD,0xE7B82D07,0x90BF1D91,
    0x1DB71064,0x6AB020F2,0xF3B97148,0x84BE41DE,0x1ADAD47D,0x6DDDE4EB,0xF4D4B551,0x83D385C7,0x136C9856,0x646BA8C0,0xFD62F97A,0x8A65C9EC,0x14015C4F,0x63066CD9,0xFA0F3D63,0x8D080DF5,
    0x3B6E20C8,0x4C69105E,0xD56041E4,0xA2677172,0x3C03E4D1,0x4B04D447,0xD20D85FD,0xA50AB56B,0x35B5A8FA,0x42B2986C,0xDBBBC9D6,0xACBCF940,0x32D86CE3,0x45DF5C75,0xDCD60DCF,0xABD13D59,
    0x26D930AC,0x51DE003A,0xC8D75180,0xBFD06116,0x21B4F4B5,0x56B3C423,0xCFBA9599,0xB8BDA50F,0x2802B89E,0x5F058808,0xC60CD9B2,0xB10BE924,0x2F6F7C87,0x58684C11,0xC1611DAB,0xB6662D3D,
    0x76DC4190,0x01DB7106,0x98D220BC,0xEFD5102A,0x71B18589,0x06B6B51F,0x9FBFE4A5,0xE8B8D433,0x7807C9A2,0x0F00F934,0x9609A88E,0xE10E9818,0x7F6A0DBB,0x086D3D2D,0x91646C97,0xE6635C01,
    0x6B6B51F4,0x1C6C6162,0x856530D8,0xF262004E,0x6C0695ED,0x1B01A57B,0x8208F4C1,0xF50FC457,0x65B0D9C6,0x12B7E950,0x8BBEB8EA,0xFCB9887C,0x62DD1DDF,0x15DA2D49,0x8CD37CF3,0xFBD44C65,
    0x4DB26158,0x3AB551CE,0xA3BC0074,0xD4BB30E2,0x4ADFA541,0x3DD895D7,0xA4D1C46D,0xD3D6F4FB,0x4369E96A,0x346ED9FC,0xAD678846,0xDA60B8D0,0x44042D73,0x33031DE5,0xAA0A4C5F,0xDD0D7CC9,
    0x5005713C,0x270241AA,0xBE0B1010,0xC90C2086,0x5768B525,0x206F85B3,0xB966D409,0xCE61E49F,0x5EDEF90E,0x29D9C998,0xB0D09822,0xC7D7A8B4,0x59B33D17,0x2EB40D81,0xB7BD5C3B,0xC0BA6CAD,
    0xEDB88320,0x9ABFB3B6,0x03B6E20C,0x74B1D29A,0xEAD54739,0x9DD277AF,0x04DB2615,0x73DC1683,0xE3630B12,0x94643B84,0x0D6D6A3E,0x7A6A5AA8,0xE40ECF0B,0x9309FF9D,0x0A00AE27,0x7D079EB1,
    0xF00F9344,0x8708A3D2,0x1E01F268,0x6906C2FE,0xF762575D,0x806567CB,0x196C3671,0x6E6B06E7,0xFED41B76,0x89D32BE0,0x10DA7A5A,0x67DD4ACC,0xF9B9DF6F,0x8EBEEFF9,0x17B7BE43,0x60B08ED5,
    0xD6D6A3E8,0xA1D1937E,0x38D8C2C4,0x4FDFF252,0xD1BB67F1,0xA6BC5767,0x3FB506DD,0x48B2364B,0xD80D2BDA,0xAF0A1B4C,0x36034AF6,0x41047A60,0xDF60EFC3,0xA867DF55,0x316E8EEF,0x4669BE79,
    0xCB61B38C,0xBC66831A,0x256FD2A0,0x5268E236,0xCC0C7795,0xBB0B4703,0x220216B9,0x5505262F,0xC5BA3BBE,0xB2BD0B28,0x2BB45A92,0x5CB36A04,0xC2D7FFA7,0xB5D0CF31,0x2CD99E8B,0x5BDEAE1D,
    0x9B64C2B0,0xEC63F226,0x756AA39C,0x026D930A,0x9C0906A9,0xEB0E363F,0x72076785,0x05005713,0x95BF4A82,0xE2B87A14,0x7BB12BAE,0x0CB61B38,0x92D28E9B,0xE5D5BE0D,0x7CDCEFB7,0x0BDBDF21,
    0x86D3D2D4,0xF1D4E242,0x68DDB3F8,0x1FDA836E,0x81BE16CD,0xF6B9265B,0x6FB077E1,0x18B74777,0x88085AE6,0xFF0F6A70,0x66063BCA,0x11010B5C,0x8F659EFF,0xF862AE69,0x616BFFD3,0x166CCF45,
    0xA00AE278,0xD70DD2EE,0x4E048354,0x3903B3C2,0xA7672661,0xD06016F7,0x4969474D,0x3E6E77DB,0xAED16A4A,0xD9D65ADC,0x40DF0B66,0x37D83BF0,0xA9BCAE53,0xDEBB9EC5,0x47B2CF7F,0x30B5FFE9,
    0xBDBDF21C,0xCABAC28A,0x53B39330,0x24B4A3A6,0xBAD03605,0xCDD70693,0x54DE5729,0x23D967BF,0xB3667A2E,0xC4614AB8,0x5D681B02,0x2A6F2B94,0xB40BBE37,0xC30C8EA1,0x5A05DF1B,0x2D02EF8D,
};

function UI_Key
UI__HashStr(const char *data_p, size_t data_size, u32 seed) {
    seed = ~seed;
    u32 crc = seed;
    const unsigned char *data = (const unsigned char *)data_p;
    const u32 *crc32_lut = g_crc32_lookup_table;
    if (data_size != 0) {
        while (data_size-- != 0) {
            unsigned char c = *data++;
            if (c == '#' && data_size >= 2 && data[0] == '#' && data[1] == '#') {
                crc = seed;
            }
            crc = (crc >> 8) ^ crc32_lut[(crc & 0xFF) ^ c];
        }
    } else {
        unsigned char c;
        while (c = *data++) {
            if (c == '#' && data[0] == '#' && data[1] == '#') {
                crc = seed;
            }
            crc = (crc >> 8) ^ crc32_lut[(crc & 0xFF) ^ c];
        }
    }
    UI_Key key = { ~crc };
    return key;
}

function void
UI_PushKey(char *key) {
    StringCopy(ui->key_stack.keys[ui->key_stack.current_pos++], 128, key);
}

function void
UI_PopKey() {
    ui->key_stack.current_pos--;
    ZeroArray(ui->key_stack.keys[ui->key_stack.current_pos], 128, u8);
}

function b32
UI_KeySame(UI_Key a, UI_Key b) {
    return a.key == b.key;
}

function b32
UI_KeyIsNull(UI_Key key) {
    return key.key == 0;
}

function b32
UI__PointInsideRect(v2 point, Rect rect) {
    return (point.x >= rect.min.x && point.y >= rect.min.y &&
            point.x < rect.max.x && point.y < rect.max.y);
}

function void
UI_Begin() {
    // Prepare the array of widgets to be rendered
    // Find out which window is currently being hovered
    ui->active_widgets.count = 0;
    ui->active_widgets.max_count = 4096;
    ui->active_widgets.widgets = PushArray(&g_state->frame_arena, ui->active_widgets.max_count, UI_Widget *);

    for (s32 i = 0; i < ui->active_windows.count - 1; ++i) {
        for (s32 j = 0; j < ui->active_windows.count - i - 1; ++j) {
            if (ui->active_windows.windows[j]->z < ui->active_windows.windows[j + 1]->z) {
                Swap(ui->active_windows.windows[j], ui->active_windows.windows[j + 1], UI_Window *);
            }
        }
    }

    ui->active_window.key = 0;

    for (s32 i = 0; i < ui->active_windows.count; ++i) {
        UI_Window *window = ui->active_windows.windows[i];
        if (UI__PointInsideRect(g_mouse_pos, window->rect)) {
            ui->active_window = window->key;
            break;
        }
    }

    ui->active_windows.count = 0;
    ui->hot_key.key = 0;
    ui->active_key.key = 0;
    ZeroArray(ui->active_windows.windows, ui->active_windows.count, sizeof(UI_Window *));

    ui->layout = &ui->layout_stack.layouts[0];
}

function void
UI_End() {
    // Sort all the widgets according to their windows z
    // Render them according to their flags
    UI_Widget **widgets = ui->active_widgets.widgets;
    for (s32 i = 0; i < ui->active_widgets.count - 1; ++i) {
        for (s32 j = 0; j < ui->active_widgets.count - i - 1; ++j) {
            if (widgets[j]->window->z > widgets[j + 1]->window->z) {
                Swap(widgets[j], widgets[j + 1], UI_Widget *);
            }
        }
    }

    for (s32 i = 0; i < ui->active_widgets.count; ++i) {
        UI_Widget *widget = widgets[i];
        UI_Window *window = widget->window;
        g_clip_rect = window->rect;

        v4 color = widget->background;
        f32 a = color.a;
        if (!UI_KeyIsNull(widget->key)) {
            if (UI_KeySame(widget->key, ui->active_key)) {
                color = V4MulF32(color, 0.5f);
            } else if (UI_KeySame(widget->key, ui->hot_key)) {
                color = V4MulF32(color, 0.9f);
            }
        }
        color.a = a;

        if (widget->flags & UI_WidgetFlag_DrawBackground)
            DrawRectAlphaClipped(g_buffer, widget->rect.min, widget->rect.max, color);

        v2 text_pos = GetCenterAlignedPosOfText(widget->text, 1.0f, widget->rect);

        DrawTextClipped(g_buffer, widget->text, 1.0f, text_pos, V4(1.0f, 1.0f, 1.0f, 1.0f));
    }

#if 0
    for (s32 i = 0; i < ui->active_windows.count; ++i) {
        UI_Window *window = ui->active_windows.windows[i];
        DrawBorderAroundRectInPixels(g_buffer, window->content_rect.min, window->content_rect.max, V4(1.0f, 0.0f, 0.0f, 1.0f), 1);
        if (UI_KeySame(ui->active_window, window->key)) {
            DrawBorderAroundRectInPixels(g_buffer, window->rect.min, window->rect.max, V4(0.0f, 1.0f, 0.0f, 1.0f), 1);
        } else {
            DrawBorderAroundRectInPixels(g_buffer, window->rect.min, window->rect.max, V4(0.0f, 0.0f, 1.0f, 1.0f), 1);
        }
    }
#endif
    ui->prev_active_key = ui->active_key;
    ui->prev_hot_key = ui->hot_key;
}

// 
// ~~~~~~~~~~~~~~~~~~~~ NOTE(hampus): Layouting ~~~~~~~~~~~~~~~~~~~~ 
//

function UI_Layout *
UI__GetCurrLayout() {
    return ui->layout;
}

function UI_Layout *
UI__PushLayout() {
    NOT_IMPLEMENTED;
}

function UI_Layout *
UI__PopLayout() {
    NOT_IMPLEMENTED;
}

function UI_Size
UI_Pixels(f32 value) {
    UI_Size result = { UI_SizeKind_Pixels, value, 0 };
    return result;
}

function void
UI_NextWidth(UI_Size size) {
    UI_Layout *layout = UI__GetCurrLayout();
    layout->next_width_set = true;
    layout->next_width = size;
}

function void
UI_NextHeight(UI_Size size) {
    UI_Layout *layout = UI__GetCurrLayout();
    layout->next_height_set = true;
    layout->next_height = size;
}

function void
UI_SkipPos() {
    UI_Layout *layout = UI__GetCurrLayout();
    layout->skip_pos = true;
}

function void
UI_NextPos(v2 pos) {
    UI_Layout *layout = UI__GetCurrLayout();
    layout->next_pos_set = true;
    layout->next_pos = pos;
}

function void
UI_NextBackground(v4 color) {
    UI_Layout *layout = UI__GetCurrLayout();
    layout->next_background = color;
    layout->next_background_set = true;
}

// 
// ~~~~~~~~~~~~~~~~~~~~ NOTE(hampus): Widget ~~~~~~~~~~~~~~~~~~~~ 
//

function UI_Comm
UI_CommFromWidget(UI_Widget *widget) {
    UI_Comm comm = { 0 };
    if (widget) {
        comm.widget = widget;

        if (UI_KeySame(ui->active_window, widget->window->key)) {
            comm.mouse = V2((f32)g_input->mouse_x, (f32)g_input->mouse_y);

            if (UI__PointInsideRect(comm.mouse, widget->rect)) {
                if (widget->flags & UI_WidgetFlag_HotAnimation)
                    ui->hot_key = widget->key;

                if (KeyPressed(g_input, KeyCode_MouseLeft)) {
                    if (widget->flags & UI_WidgetFlag_ActiveAnimation)
                        ui->active_key = widget->key;
                    if (widget->flags & UI_WidgetFlag_Clickable)
                        comm.clicked = true;
                }
            }
        }

        if (UI_KeySame(ui->prev_active_key, widget->key)) {
            // TODO(hampus): UI_WidgetFlag_Draggable?
            if (KeyDown(g_input, KeyCode_MouseLeft)) {
                comm.dragging = true;
                comm.drag_delta = V2SubV2(g_mouse_pos, g_prev_mouse_pos);
                if (widget->flags & UI_WidgetFlag_ActiveAnimation)
                    ui->active_key = widget->key;
            }
        }
    }

    return comm;
}

function void
UI__PushWidgetToGroup(UI_Widget *widget) {
    Assert((ui->active_widgets.count + 1) < ui->active_widgets.max_count);
    ui->active_widgets.widgets[ui->active_widgets.count++] = widget;
}

function b32
UI__WidgetRequiresHashing(UI_WidgetFlags flags) {
    b32 result = false;

    result = ((flags & UI_WidgetFlag_HotAnimation) || (flags & UI_WidgetFlag_ActiveAnimation));

    return result;
}

function void
UI__PushWidgetInTree(UI_Widget *widget) {
}

function void
UI__CalcSize(UI_Widget *widget) {
    switch (widget->semantic_size[Axis2_X].kind) {
        case UI_SizeKind_Null: {
            NOT_IMPLEMENTED;
        } break;

        case UI_SizeKind_Pixels: {
            widget->computed_size[Axis2_X] = widget->semantic_size[Axis2_X].value;
        } break;

        case UI_SizeKind_PercentOfParent: {
            NOT_IMPLEMENTED;
        } break;

        case UI_SizeKind_TextContents: {
            NOT_IMPLEMENTED;
        } break;

        case UI_SizeKind_ChildrenSum: {
            NOT_IMPLEMENTED;
        } break;
    }

    switch (widget->semantic_size[Axis2_Y].kind) {
        case UI_SizeKind_Null: {
            NOT_IMPLEMENTED;
        } break;

        case UI_SizeKind_Pixels: {
            widget->computed_size[Axis2_Y] = widget->semantic_size[Axis2_Y].value;
        } break;

        case UI_SizeKind_PercentOfParent: {
            NOT_IMPLEMENTED;
        } break;

        case UI_SizeKind_TextContents: {
            NOT_IMPLEMENTED;
        } break;

        case UI_SizeKind_ChildrenSum: {
            NOT_IMPLEMENTED;
        } break;
    }
}

function void
UI__CalcRelPosition(UI_Widget *widget) {
    UI_Layout *layout = UI__GetCurrLayout();
    if (layout->next_pos_set) {
        widget->computed_rel_position[Axis2_X] = layout->next_pos.x - ui->window->content_rect.min.x;
        widget->computed_rel_position[Axis2_Y] = layout->next_pos.y - ui->window->content_rect.min.y;
        layout->next_pos_set = false;
    } else {
        widget->computed_rel_position[Axis2_X] = ui->window->cursor_pos.x;
        widget->computed_rel_position[Axis2_Y] = ui->window->cursor_pos.y;
    }
}

function void
UI__CalcRect(UI_Widget *widget) {
    f32 x0 = widget->window->content_rect.min.x + widget->computed_rel_position[Axis2_X];
    f32 y0 = widget->window->content_rect.min.y + widget->computed_rel_position[Axis2_Y];

    f32 x1 = x0 + widget->computed_size[Axis2_X];
    f32 y1 = y0 + widget->computed_size[Axis2_Y];

    widget->rect.min = V2(x0, y0);
    widget->rect.max = V2(x1, y1);
}

function UI_Widget *
UI_WidgetMakeFromKey(char *string, UI_WidgetFlags flags) {
    UI_Widget *widget = 0;
    if (!UI__WidgetRequiresHashing(flags)) {
        widget = PushStruct(&g_state->frame_arena, UI_Widget);
    } else {
        char hash[512] = { 0 };
        for (s32 i = 0; i < ui->key_stack.current_pos; ++i) {
            StringAppend(hash, 512, ui->key_stack.keys[i]);
        }
        StringAppend(hash, 512, string);
        UI_Key key = UI__HashStr(hash, 0, 1);
        u32 slot_index = key.key % ui->widget_table.length;
        if (!ui->widget_table.widgets[slot_index]) {
            ui->widget_table.widgets[slot_index] = PushStruct(&g_state->permanent_arena, UI_Widget);
        } else {
            if (UI_KeySame(ui->widget_table.widgets[slot_index]->key, key)) {

            } else {
                // TODO(hampus): Hash if the key is the same
                NOT_IMPLEMENTED;
            }
        }
        widget = ui->widget_table.widgets[slot_index];

        widget->key = key;
    }

    return widget;
}

function UI_Widget *
UI_WidgetMake(char *string, UI_WidgetFlags flags) {
    UI_Widget *widget = 0;
    if (ui->window->hide_content && !(flags & UI_WidgetFlag_NoHide)) {

    } else {
        widget = UI_WidgetMakeFromKey(string, flags);
        s32 string_length = StringLength(string);
        for (s32 i = 0; i < string_length; ++i) {
            if (i != string_length - 1) {
                if (string[i] == '#' && string[i + 1] == '#') {
                    break;
                }
            }
            widget->text[i] = string[i];
        }

        widget->flags = flags;
        widget->window = ui->window;

        // NOTE(hampus): Push the widget in the hierarchy
        // TODO(hampus): Do we need this?
        UI__PushWidgetInTree(widget);

        UI_Layout *layout = UI__GetCurrLayout();
        if (layout->next_width_set) {
            widget->semantic_size[0] = layout->next_width;
            layout->next_width_set = false;
        } else {
            widget->semantic_size[0].kind = UI_SizeKind_Pixels;
            widget->semantic_size[0].value = (f32)TextLength(widget->text);

            widget->semantic_size[0].value += 5;
        }

        if (layout->next_height_set) {
            widget->semantic_size[1] = layout->next_height;
            layout->next_height_set = false;
        } else {
            widget->semantic_size[1].kind = UI_SizeKind_Pixels;
            widget->semantic_size[1].value = 0;

            widget->semantic_size[1].value += 12;
        }

        // NOTE(hampus): Calculate the size
        UI__CalcSize(widget);

        // NOTE(hampus): Calculate its position
        UI__CalcRelPosition(widget);

        // NOTE(hampus): Calculate its rectangle
        UI__CalcRect(widget);

        // TODO(hampus): Only add the position if it is greater than
        // the current position of the cursor
        if (layout->skip_pos) {
            layout->skip_pos = false;
        } else {
            ui->window->cursor_pos.y += widget->computed_size[Axis2_Y] + 3;
        }

        if (layout->next_background_set) {
            widget->background = layout->next_background;
            layout->next_background_set = false;
        } else {
            widget->background = V4(0.3f, 0.3f, 0.3f, 1.0f);
        }

        UI__PushWidgetToGroup(widget);
    }

    return widget;
}

// 
// ~~~~~~~~~~~~~~~~~~~~ NOTE(hampus): Window ~~~~~~~~~~~~~~~~~~~~ 
//
function UI_Window *
UI__WindowMake(char *string, s32 x0, s32 y0, s32 x1, s32 y1) {
    UI_Window *window = 0;

    UI_Key key = UI__HashStr(string, 0, 1);
    u32 slot_index = key.key % ui->window_table.length;
    if (!ui->window_table.windows[slot_index]) {
        ui->window_table.windows[slot_index] = PushStruct(&g_state->permanent_arena, UI_Window);
        window = ui->window_table.windows[slot_index];

        window->rect = (Rect){ (f32)x0, (f32)y0, (f32)x1, (f32)y1 };
        window->content_rect = window->rect;
        window->content_rect.min.y += 16;
    } else {
        if (UI_KeySame(ui->window_table.windows[slot_index]->key, key)) {

        } else {
            // TODO(hampus): Hashing
            NOT_IMPLEMENTED;
        }
        Assert(UI_KeySame(ui->window_table.windows[slot_index]->key, key));
        window = ui->window_table.windows[slot_index];
    }

    window->key = key;

    return window;
}

#include "ui_widgets.c"

function UI_Window *
UI_BeginWindow(char *string, s32 x0, s32 y0, s32 x1, s32 y1) {
    UI_Window *window = 0;
    window = UI__WindowMake(string, x0, y0, x1, y1);
    Assert(window);
    window->parent = ui->window;
    ui->window = window;

    window->z = ui->active_windows.count;
    ui->active_windows.windows[ui->active_windows.count++] = window;

    s32 string_length = StringLength(string);
    for (s32 i = 0; i < string_length; ++i) {
        if (i != string_length - 1) {
            if (string[i] == '#' && string[i + 1] == '#') {
                break;
            }
        }
        window->title[i] = string[i];
    }

    window->cursor_pos = V2(3, 3);
    f32 title_bar_height = 16;
    UI_PushKey(string);

    if (window->dragging) {
        v2 delta = V2SubV2(g_mouse_pos, g_prev_mouse_pos);
        window->rect.min.x += delta.x;
        window->rect.min.y += delta.y;
        window->rect.max.x += delta.x;
        window->rect.max.y += delta.y;
        window->content_rect = window->rect;
        window->content_rect.min.y += title_bar_height;
    }

    UI_NextWidth(UI_Pixels((f32)x1 - x0));
    UI_NextHeight(UI_Pixels(title_bar_height));
    UI_SkipPos();
    UI_NextPos(V2(window->rect.min.x, window->rect.min.y));
    UI_Widget *title_widget = UI_WidgetMake(window->title,
                                            UI_WidgetFlag_DrawBackground |
                                            UI_WidgetFlag_DrawText |
                                            UI_WidgetFlag_HotAnimation |
                                            UI_WidgetFlag_Clickable |
                                            UI_WidgetFlag_ActiveAnimation |
                                            UI_WidgetFlag_NoHide);

    UI_Comm title_comm = UI_CommFromWidget(title_widget);

    if (title_comm.clicked) {
        //window->hide_content = !window->hide_content;
    }

    window->dragging = title_comm.dragging;

    UI_NextWidth(UI_Pixels((f32)x1 - x0));
    UI_NextHeight(UI_Pixels((f32)y1 - y0 - title_bar_height));
    UI_SkipPos();
    UI_NextPos(window->content_rect.min);
    UI_NextBackground(V4(0.1f, 0.1f, 0.1f, 0.9f));
    UI_Widget *background = UI_WidgetMake(0,
                                          UI_WidgetFlag_DrawBackground);
#if 1
    UI_NextWidth(UI_Pixels(10));
    if (window->hide_content) {
        UI_NextHeight(UI_Pixels(5));
    } else {
        UI_NextHeight(UI_Pixels(10));
    }
    UI_SkipPos();
    UI_NextPos(V2AddV2(window->rect.min, V2(3, 3)));
    UI_NextBackground(V4(1.0f, 1.0f, 1.0f, 1.0f));
    UI_PushKey("Hide_Button");
    UI_Comm comm = UI_CommFromWidget(UI_WidgetMake(0,
                                                   UI_WidgetFlag_Clickable |
                                                   UI_WidgetFlag_DrawBorder |
                                                   UI_WidgetFlag_DrawText |
                                                   UI_WidgetFlag_DrawBackground |
                                                   UI_WidgetFlag_HotAnimation |
                                                   UI_WidgetFlag_ActiveAnimation |
                                                   UI_WidgetFlag_NoHide));
    UI_PopKey();
    if (comm.clicked) {
        window->hide_content = !window->hide_content;
    }
#endif

    return ui->window;
}

function UI_Window *
UI_EndWindow() {
    UI_PopKey();
    ui->window = ui->window->parent;
    return ui->window;
}
