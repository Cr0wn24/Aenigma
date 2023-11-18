global v4 white = { 1.0f, 1.0f, 1.0f, 1.0f };
global v4 gray = { 0.6f, 0.6f, 0.6f, 1.0f };
global b32 currently_typing = false;

global UI_Layout *curr_layout;

function f32
GetTextWidth(char *text, f32 text_size) {
    f32 result;
    s32 text_length = StringLength(text);
    result = (10.0f / config.game_res_width) * text_length * text_size;
    // this is to remove the space from the last character
    result -= (3.0f / config.game_res_width) * text_size;
    return result;
}

function f32
GetTextHeight(char *text, f32 text_size) {
    f32 result;

    result = (8.0f / config.game_res_height) * text_size;

    return result;
}

function inline Rect
MakeRect(v2 min, v2 max) {
    Rect result;

    result.min = min;
    result.max = max;

    return result;
}

function void
CenterRect(Rect *src, Rect dest) {
    f32 src_width = src->max.x - src->min.x;
    f32 src_height = src->max.y - src->min.y;

    f32 dest_width = dest.max.x - dest.min.x;
    f32 dest_height = dest.max.y - dest.min.y;

    f32 offset_x = (dest_width - src_width) / 2.0f;
    f32 offset_y = (dest_height - src_height) / 2.0f;

    src->min = V2(offset_x, offset_y);
    src->min = V2AddV2(src->min, dest.min);
    src->max = V2AddV2(src->min, V2(src_width, src_height));
}

function Rect
GetTextRect(char *text, f32 text_size) {
    Rect result = { 0 };

    f32 character_width = text_size * 10;
    f32 character_height = text_size * 8;

    s32 text_length = StringLength(text);

    result.max.x = character_width * text_length - 3;
    result.max.y = character_height;

    return result;
}

function b32
PointInsideRect(v2 point, Rect rect) {
    b32 result = false;

    result = (point.x >= rect.min.x && point.x < rect.max.x &&
              point.y >= rect.min.y && point.y < rect.max.y);

    return result;
}

function v2
AlignTextInRectInPixels(char *text, f32 text_size, Rect dest, Alignment_Flags align) {
    v2 result = dest.min;
    u32 text_length = StringLength(text);
    f32 src_width = text_length * text_size * 10.0f - 3;
    f32 src_height = text_size * 8.0f;

    f32 dest_width = dest.max.x - dest.min.x;
    f32 dest_height = dest.max.y - dest.min.y;

    f32 offset_y = (dest_height - src_height) / 2.0f;

    if (align & AlignFlag_CenterHeight) {
        result.y += offset_y;
    }

    return result;
}

function v2
AlignTextInRect(char *text, f32 text_size, Rect dest, Alignment_Flags align) {
    v2 result = dest.min;
    u32 text_length = StringLength(text);
    f32 src_width = GetTextWidth(text, text_size);
    f32 src_height = GetTextHeight(text, text_size);

    f32 dest_width = dest.max.x - dest.min.x;
    f32 dest_height = dest.max.y - dest.min.y;

    f32 offset_y = (dest_height - src_height) / 2.0f;

    if (align & AlignFlag_CenterHeight) {
        result.y += offset_y;
    }

    return result;
}

function v2
GetCenterAlignedPosOfText(char *text, f32 text_size, Rect dest) {
    v2 result = { 0 };
    u32 text_length = StringLength(text);
    f32 src_width = GetTextWidth(text, text_size);
    f32 src_height = GetTextHeight(text, text_size);

    f32 dest_width = dest.max.x - dest.min.x;
    f32 dest_height = dest.max.y - dest.min.y;

    f32 offset_x = (dest_width - src_width) / 2.0f;
    f32 offset_y = (dest_height - src_height) / 2.0f;

    result = V2(offset_x, offset_y);
    result = V2AddV2(result, dest.min);

    return result;
}

function b32
InputButton(v2 min, v2 max, char text_buffer[], u32 buffer_size, b32 *typed) {
    b32 clicked = false;

    Input *input = curr_layout->input;
    v2 mouse_pos = V2(input->mouse_x, input->mouse_y);

    Rect rect = { 0 };
    rect.min = min;
    rect.max = max;

    if (PointInsideRect(mouse_pos, rect)) {
        if (KeyPressed(input, KeyCode_MouseLeft)) {
            currently_typing = true;
        }
    }

    if (KeyPressed(input, KeyCode_Escape) && currently_typing) {
        currently_typing = false;
        goto end;
    }

    if (currently_typing) {
        if (KeyPressed(input, KeyCode_Enter) && !input->alt_down) {
            clicked = true;
            currently_typing = false;
            if (typed) {
                *typed = true;
            }
            goto end;
        }

        if (input->last_char) {
            // 8 is ascii for backspace
            if (input->last_char == 8) {
                if (!StringEmpty(text_buffer)) {
                    text_buffer[StringLength(text_buffer) - 1] = 0;
                }
                if (typed) {
                    *typed = true;
                }
            } else if (input->last_char >= 0 && input->last_char < ArrayCount(ascii_table)) {
                b32 at_last_character = (StringLength(text_buffer) + 1) == buffer_size;
                b32 game_supports_char = (StringLength(ascii_table[input->last_char]) != 0);
                if (!at_last_character && game_supports_char) {
                    text_buffer[StringLength(text_buffer)] = input->last_char;
                    if (typed) {
                        *typed = true;
                    }
                }
            }
        }
    }

    DrawRect(curr_layout->buffer, min, max, V4(0.5f, 0.5f, 0.5f, 1.0f));
    DrawBorderAroundRect(curr_layout->buffer, min, max, V4(0.2f, 0.2f, 0.2f, 1.0f), 1);

    f32 text_offset = 0.005f;
    s32 max_characters_in_rect = (u32)((rect.max.x - rect.min.x - text_offset) / (10.0f / config.game_res_width));

    s32 num_char_overflow = StringLength(text_buffer) - max_characters_in_rect;

    b32 char_overflow = num_char_overflow > 0;
    if (char_overflow) {
        text_buffer += num_char_overflow;
    }

    v2 text_pos = AlignTextInRect(text_buffer, 1.0f, rect, AlignFlag_CenterHeight);
    text_pos.x += text_offset;
    DrawText(curr_layout->buffer, text_buffer, 1.0f, text_pos, V4(1.0f, 1.0f, 1.0f, 1.0f));

end:
    return clicked;
}

function b32
BitmapButton(Bitmap *bitmap, v2 min, v2 max) {
    Rect rect = { 0 };
    rect.min = min;
    rect.max = max;
    b32 clicked = false;
    b32 hovered = false;
    v4 text_color = gray;
    Input *input = curr_layout->input;
    v2 mouse_pos = V2(input->mouse_x, input->mouse_y);
    if (PointInsideRect(mouse_pos, rect)) {
        hovered = true;
        if (KeyPressed(input, KeyCode_MouseLeft)) {
            clicked = true;
        }
    }

    if (hovered) {
        text_color = white;
    }

    DrawBitmapStretch(curr_layout->buffer, bitmap, min, max);

    return clicked;

}

function b32
TextButton(char *text, v2 pos, f32 text_size) {
    Rect rect = MakeRect(pos, pos);
    rect.max.x += GetTextWidth(text, text_size);
    rect.max.y += GetTextHeight(text, text_size);

    b32 clicked = false;
    b32 hovered = false;

    v4 text_color = gray;
    Input *input = curr_layout->input;
    v2 mouse_pos = V2(input->mouse_x, input->mouse_y);
    if (PointInsideRect(mouse_pos, rect)) {
        hovered = true;
        if (KeyPressed(input, KeyCode_MouseLeft)) {
            clicked = true;
        }
    }

    if (hovered) {
        text_color = white;
    }

    DrawText(curr_layout->buffer, text, text_size, rect.min, text_color);

    return clicked;
}

function void
UI_BeginLayout(UI_Layout *layout, RenderBuffer *buffer, Input *input) {
    layout->input = input;
    layout->buffer = buffer;
    curr_layout = layout;
}

function void
UI_EndLayout() {
    curr_layout = 0;
}
