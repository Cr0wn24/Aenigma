RenderBuffer *g_buffer;
Input *g_input;

UI *ui = 0;

struct {
    v4 background_color;
    v4 hot_color;
    v4 active_color;
    v4 content_color;
} ui_config =
{
    .background_color = {0.2f, 0.2f, 0.2f, 1.0f},
    .hot_color = {0.3f, 0.3f, 0.3f, 1.0f},
    .active_color = {0.4f, 0.4f, 0.4f, 1.0f},
    .content_color = {1.0f, 1.0f, 1.0f, 1.0f},
};

function b32
UI_SameKey(UI_Key a, UI_Key b) {
    return(a.key == b.key);
}

function void
UI_ZeroKey(UI_Key *a) {
    a->key = 0;
}

function b32
UI_KeyIsNull(UI_Key a) {
    return(a.key == 0);
}

// TODO(hampus): Should we just remove the concept of parents?
// Rethink this sometime! What should we do instead?
// What pros/cons?
function UI_Box *
UI_PushParent(UI_Box *box) {
    ui->parent_index++;
    ui->parents[ui->parent_index] = box;
    ui->curr_parent = box;

    return ui->curr_parent;
}

function UI_Box *
UI_PopParent(void) {
    ui->parent_index--;
    ui->curr_parent = ui->parents[ui->parent_index];

    return ui->curr_parent;
}

function void
UI_PushKey(char *key) {
    BEGIN_TIMED_FUNCTION();
    UI_Window *window = ui->window;
    window->curr_key_stack_index++;
    StringCopy(window->key_stack[window->curr_key_stack_index], 256, key);
    ZeroArray(window->full_key_stack, 1024, char);
    u32 i = window->curr_key_stack_index;
    while (i--) {
        StringAppend(window->full_key_stack, 1024, window->key_stack[window->curr_key_stack_index - i]);
    }
    END_TIMED_FUNCTION();
}

function void
UI_PopKey(void) {
    BEGIN_TIMED_FUNCTION();
    UI_Window *window = ui->window;
    window->curr_key_stack_index--;
    ZeroArray(window->full_key_stack, 1024, char);
    u32 i = window->curr_key_stack_index;
    while (i--) {
        StringAppend(window->full_key_stack, 1024, window->key_stack[window->curr_key_stack_index - i]);
    }
    END_TIMED_FUNCTION();
}

function void
UI_PushClipRect(Rect rect) {

    ui->curr_layout_state++;
    ui->layout_state_index++;

    rect.min.x = Clamp(0, rect.min.x, 960);
    rect.min.y = Clamp(0, rect.min.y, 540);

    rect.max.x = Clamp(0, rect.max.x, 960);
    rect.max.y = Clamp(0, rect.max.y, 540);

    CopyMemory(ui->curr_layout_state,
               &ui->layout_states[ui->layout_state_index - 1],
               sizeof(UI_Layout_State));

    ui->curr_layout_state->clip_rect_set = true;
    ui->curr_layout_state->clip_rect = rect;
}

function void
UI_PopClipRect() {
    ZeroStruct(*ui->curr_layout_state);
    ui->curr_layout_state--;
    ui->layout_state_index--;
}

// TODO(hampus): Right now only 1 window at the same time works.
// Implement this as a stack instead!
function UI_Window *
UI_PushWindow(UI_Window *window) {
    ui->window_index++;
    ui->windows[ui->window_index] = window;
    ui->window = window;

    UI_PushKey(window->title);

    return ui->window;
}

// TODO(hampus): This should actcually be UI_EndWindow();
function UI_Window *
UI_PopWindow() {
    UI_PopKey();
    UI_PopParent();

    ui->windows[ui->window_index] = 0;
    ui->window_index--;
    ui->window = ui->windows[ui->window_index];

    return ui->window;
}

function void
UI_SkipScroll() {
    ui->curr_layout_state->skip_scroll = true;
}

function void
UI_PushSkipScroll() {
    ui->curr_layout_state++;
    ui->layout_state_index++;

    CopyMemory(ui->curr_layout_state,
               &ui->layout_states[ui->layout_state_index - 1],
               sizeof(UI_Layout_State));

    ui->curr_layout_state->skip_scroll = true;
}

function void
UI_PopSkipScroll() {
    ZeroStruct(*ui->curr_layout_state);
    ui->curr_layout_state--;
    ui->layout_state_index--;
}

function void
UI_PushIndent(f32 value) {
    ui->curr_layout_state++;
    ui->layout_state_index++;

    CopyMemory(ui->curr_layout_state,
               &ui->layout_states[ui->layout_state_index - 1],
               sizeof(UI_Layout_State));

    ui->curr_layout_state->indent = value;
}

function void
UI_PopIndent() {
    ZeroStruct(*ui->curr_layout_state);
    ui->curr_layout_state--;
    ui->layout_state_index--;
}

// NOTE(hampus): Relative to the prev box. 
function void
UI_SetNextRelativePos(f32 value) {
    ui->curr_layout_state->next_relative_pos = value;
    ui->curr_layout_state->next_relative_pos_set = true;
}

function void
UI_SetNextBackgroundColor(v4 color) {
    ui->curr_layout_state->next_background_color_set = true;
    ui->curr_layout_state->next_background_color = color;
}

function void
UI_SetNextPos(v2 pos) {
    ui->curr_layout_state->next_pos = pos;
    ui->curr_layout_state->next_pos_set = true;
}

function void
UI_SetNextTextAlign(UI_AlignmentFlag align) {
    ui->curr_layout_state->next_text_align = align;
}

// TODO(hampus): This is actually margin, not padding
function void
UI_NoPadding() {
    ui->curr_layout_state->no_padding = true;
}

function void
UI_NextAlign(UI_AlignmentFlag flag) {
    ui->curr_layout_state->next_align |= flag;
}

function void
UI_SetNextWidth(f32 value) {
    // TODO(hampus): This doesn't actually have anything to do with 
    // the layout state. next_width and the others can just be globals
    ui->curr_layout_state->next_width = value;
}

function void
UI_SetNextHeight(f32 value) {
    ui->curr_layout_state->next_height = value;
}

function void
UI_SkipNextPos() {
    ui->curr_layout_state->skip_next_pos = true;
}

function void
UI_PushRows() {
    ui->curr_layout_state++;
    ui->layout_state_index++;

    CopyMemory(ui->curr_layout_state,
               &ui->layout_states[ui->layout_state_index - 1],
               sizeof(UI_Layout_State));

    ui->curr_layout_state->rows = true;
    ui->curr_layout_state->columns = false;
}

function void
UI_PopRows() {
    ZeroStruct(*ui->curr_layout_state);
    ui->curr_layout_state--;
    ui->layout_state_index--;
}

function void
UI_PushColumns(UI_Box *source) {
    ui->curr_layout_state++;
    ui->layout_state_index++;

    CopyMemory(ui->curr_layout_state,
               &ui->layout_states[ui->layout_state_index - 1],
               sizeof(UI_Layout_State));

    ui->curr_layout_state->rows = false;
    ui->curr_layout_state->columns = true;

    UI_PushParent(source);
}

function void
UI_PopColumns() {
    ZeroStruct(*ui->curr_layout_state);
    ui->curr_layout_state--;
    ui->layout_state_index--;

    UI_PopParent();
}

function void
UI_SameLine() {
    UI_PushColumns(ui->curr_box);
    ui->curr_layout_state->same_line = true;
}


function void
UI_Begin() {
    UI_ZeroKey(&ui->hot_key);
    UI_ZeroKey(&ui->active_key);
    ui->curr_parent = ui->parents[0];
    ui->curr_layout_state = &ui->layout_states[0];
    ui->num_boxes_to_render = 0;
}

b32 show_border = false;

b32 hand_cursor = false;

function void
UI_End() {
    BEGIN_TIMED_FUNCTION();

    if (!UI_KeyIsNull(ui->hot_key) && !hand_cursor) {
        hand_cursor = true;
    } else {
        hand_cursor = false;
        platform->SetCursorType(CursorType_Arrow);
    }
#if 1
    for (u32 i = 0; i < ui->num_boxes_to_render; ++i) {
        UI_Box *box = ui->boxes_to_render[i];
        while (box) {
            if (!(box->flags & UI_BoxFlag_NoClip)) {
                if (box->window) {
                    g_clip_rect = box->window->clip_rect;
                } else {
                    // could be the case for the window.
                    g_clip_rect = (Rect){ 0, 0, (f32)g_buffer->width, (f32)g_buffer->height };
                }

            } else {
                g_clip_rect = (Rect){ 0, 0, (f32)g_buffer->width, (f32)g_buffer->height };
            }
            b32 is_hot = false;
            b32 is_active = false;

            if (!UI_KeyIsNull(box->key)) {
                is_hot = UI_SameKey(box->key, ui->hot_key) && box->flags & UI_BoxFlag_HotAnimation;
                is_active = UI_SameKey(box->key, ui->active_key) && box->flags & UI_BoxFlag_ActiveAnimation;
            }

            Rect rect = box->rect;
            v2 min = rect.min;
            v2 max = rect.max;

            if (box->flags & UI_BoxFlag_DrawBackground) {
                if (is_active) {
                    DrawRectAlphaClipped(g_buffer, min, max, ui_config.active_color);
                } else if (is_hot) {
                    DrawRectAlphaClipped(g_buffer, min, max, ui_config.hot_color);
                } else {
                    if (box->color_set) {
                        DrawRectAlphaClipped(g_buffer, min, max, box->color);
                    } else {
                        DrawRectAlphaClipped(g_buffer, min, max, ui_config.background_color);
                    }
                }
            }

            if (box->flags & UI_BoxFlag_DrawBorder) {
                DrawBorderAroundRectInPixels(g_buffer, min, max, ui_config.content_color, 1);
            }

            if (box->flags & UI_BoxFlag_DrawText) {
                Assert(box->text);

                v2 text_pos = { 0 };
                if (box->text_alignment) {
                    text_pos = AlignTextInRect(box->text, 1.0f, rect, box->text_alignment);
                } else {
                    text_pos = GetCenterAlignedPosOfText(box->text, 1.0f, rect);
                }

                // calculate how much the text is outside 
                // the box  and render accordingly

                u32 text_length = StringLength(box->text);
                f32 box_width = box->rect.max.x - box->rect.min.x;
                u32 max_characters_fit = (u32)(box_width / 8.0f);

                u32 characters_to_skip = 0;
                if (text_length > max_characters_fit) {
                    //characters_to_skip = (text_length - max_characters_fit);
                }

                DrawTextClipped(g_buffer, box->text + characters_to_skip, 1.0f, text_pos, ui_config.content_color);
            }

            if (show_border) {
                DrawBorderAroundRectInPixels(g_buffer, rect.min, rect.max, V4(1.0f, 0.0f, 0.0f, 1.0f), 1);
            }

            box = box->hash_next;
        }
    }
#endif
    ui->prev_active_key = ui->active_key;
    ui->prev_hot_key = ui->hot_key;

    END_TIMED_FUNCTION();
}

//
//~ NOTE(hampus): Internal stuff
//
function void
UI__PushWidgetInParent(UI_Box *box) {
    BEGIN_TIMED_FUNCTION();
    if (!ui->curr_parent) {
        // NOTE(hampus): This could be the case for a window.
        ui->curr_parent = box;
    } else {
        box->parent = ui->curr_parent;

        UI_Box *temp = box->parent->first;

        if (!temp) {
            box->parent->first = box;
        } else {
            while (temp->next != 0) {
                temp = temp->next;
            }
            temp->next = box;
            box->prev = temp;
        }
    }
    END_TIMED_FUNCTION();
}

function void
UI__CalcRelativePosition(UI_Box *box) {
    BEGIN_TIMED_FUNCTION();
    if (ui->curr_layout_state->next_pos_set) {
        box->rect.min = ui->curr_layout_state->next_pos;
        ui->curr_layout_state->next_pos_set = false;
    } else {
        UI_Box *box_to_align_to = 0;
        if (box->prev) {
            box_to_align_to = box->prev;
        } else {
            box_to_align_to = ui->curr_parent;
        }

        while (box_to_align_to->skip_pos) {
            if (box_to_align_to->prev) {
                box_to_align_to = box_to_align_to->prev;
            } else {
                box_to_align_to = box_to_align_to->parent;
            }
        }

        f32 prev_pos_x = box_to_align_to->pos_rect.max.x;
        f32 prev_pos_y = box_to_align_to->pos_rect.max.y;

        f32 prev_width = box_to_align_to->pos_rect.max.x - box_to_align_to->pos_rect.min.x;
        f32 prev_height = box_to_align_to->pos_rect.max.y - box_to_align_to->pos_rect.min.y;

        f32 curr_width = box->computed_size[0];
        f32 curr_height = box->computed_size[1];

        u32 padding_x = 3;
        u32 padding_y = 3;

        if (ui->curr_layout_state->no_padding) {
            padding_x = 0;
            padding_y = 0;

            ui->curr_layout_state->no_padding = false;
        }

        if (ui->curr_layout_state->next_relative_pos_set) {
            box->rect.min = V2(box_to_align_to->pos_rect.min.x + ui->curr_layout_state->next_relative_pos * prev_width, ui->curr_parent->rect.min.y);

            ui->curr_layout_state->next_relative_pos = 0;
            ui->curr_layout_state->next_relative_pos_set = false;
        } else if (ui->curr_layout_state->columns) {
            box->rect.min = V2(prev_pos_x + padding_x,
                               box_to_align_to->pos_rect.min.y);
        } else {
            if (ui->curr_layout_state->next_align & UI_AlignmentFlag_Right) {
                box->rect.min = V2(ui->curr_parent->rect.max.x - padding_y - curr_width,
                                   prev_pos_y + padding_x);
                ui->curr_layout_state->next_align = 0;
            } else {
                box->rect.min = V2(ui->curr_parent->rect.min.x + padding_y,
                                   prev_pos_y + padding_x);
            }
        }

        box->computed_rel_position[0] = box->rect.min.x - ui->curr_parent->computed_rel_position[0];
        box->computed_rel_position[1] = box->rect.min.y - ui->curr_parent->computed_rel_position[1];

        if (ui->curr_layout_state->indent) {
            box->rect.min.x += ui->curr_layout_state->indent;
        }
    }

    box->rect.max = V2AddV2(box->rect.min, V2(box->computed_size[0], box->computed_size[1]));

    // NOTE(hampus): Scrolling
    if (ui->window && !(box->flags & UI_BoxFlag_NoScroll)) {
        if (ui->window->box) {
            if ((box->rect.max.y) > (ui->window->box->max_pos_y) &&
                !box->skip_pos) {
                // TODO(hampus): Fix this. This max_pos_y should not be based
                // on the the rect
                ui->window->box->max_pos_y = box->rect.max.y;
            }

            if ((box->rect.max.x) > (ui->window->box->max_pos_x) &&
                !box->skip_pos) {
                // TODO(hampus): Fix this. This max_pos_y should not be based
                // on the the rect
                ui->window->box->max_pos_x = box->rect.max.x;
            }
        }
    }

    END_TIMED_FUNCTION();
}

function void
UI__CalcSemanticSize(UI_Box *box) {
    BEGIN_TIMED_FUNCTION();
    if (ui->curr_layout_state->next_width) {
        box->computed_size[0] = ui->curr_layout_state->next_width;
        ui->curr_layout_state->next_width = 0;
    } else {
        // compute the final sizes
        switch (box->semantic_size[0].kind) {
            case UI_SizeKind_Pixels:
            {
                box->computed_size[0] = box->semantic_size[0].value;
            } break;

            InvalidCase;
        }
        END_TIMED_FUNCTION();
    }

    if (ui->curr_layout_state->next_height) {
        box->computed_size[1] = ui->curr_layout_state->next_height;
        ui->curr_layout_state->next_height = 0;

    } else {
        // compute the final sizes
        switch (box->semantic_size[1].kind) {
            case UI_SizeKind_Pixels:
            {
                box->computed_size[1] = box->semantic_size[1].value;
            } break;

            InvalidCase;
        }
    }
}

global u32 GCrc32LookupTable[256] =
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
    const u32 *crc32_lut = GCrc32LookupTable;
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

function b32
UI__WidgetFlagsRequiresHashing(UI_BoxFlags flags) {
    b32 result;

    result =
        ((flags & UI_BoxFlag_HotAnimation) ||
         (flags & UI_BoxFlag_ActiveAnimation) ||
         (flags & UI_BoxFlag_Clickable));

    return(result);
}

function b32
UI__PointInsideRect(v2 point, Rect rect) {
    b32 result;

    result = (point.x >= rect.min.x && point.x < rect.max.x &&
              point.y >= rect.min.y && point.y < rect.max.y);

    return result;
}

#include "ui_widgets.c"