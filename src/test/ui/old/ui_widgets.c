function UI_Comm
UI_CommFromBox(UI_Box *box) {
    UI_Comm result = { 0 };

    result.box = box;

    Rect rect = box->rect;

    result.mouse = V2(g_input->mouse_x, g_input->mouse_y);

    b32 mouse_inside_rect = UI__PointInsideRect(result.mouse, rect);
    if (mouse_inside_rect) {
        if (box->flags & UI_BoxFlag_NoClip) {

        } else {
            mouse_inside_rect =
                UI__PointInsideRect(result.mouse, box->window->clip_rect);
        }
    }

    if (mouse_inside_rect) {
        // NOTE(hampus): To prevent interacting with other 
        // widgtes while still using one, could be the case with a slider
        // when dragging outside its rectangle
        if (KeyDown(g_input, KeyCode_MouseLeft) &&
            KeyWasDown(g_input, KeyCode_MouseLeft) &&
            !UI_SameKey(box->key, ui->prev_active_key)) {
            return result;
        }

        if (!UI_KeyIsNull(box->key))
            ui->hot_key = box->key;

        result.hovering = true;
        if (KeyPressed(g_input, KeyCode_MouseLeft)) {
            result.pressed = true;
            if (box->flags & UI_BoxFlag_Clickable) {
                result.clicked = true;

                if (!UI_KeyIsNull(box->key))
                    ui->active_key = box->key;
            }
        } else if (KeyDown(g_input, KeyCode_MouseLeft)) {
            if (UI_SameKey(box->key, ui->hot_key) &&
                !UI_KeyIsNull(box->key)) {
                result.dragging = true;
                ui->active_key = box->key;
            }
        } else if (KeyReleased(g_input, KeyCode_MouseLeft)) {
            result.released = true;
        }

        result.scrolling = g_input->mouse_wheel;
    } else {
        // NOTE(hampus): This could be the case for a slider,
        // when dragging outside the rectangle of a box.
        if (UI_SameKey(box->key, ui->prev_active_key) &&
            !UI_KeyIsNull(box->key)) {
            if (KeyDown(g_input, KeyCode_MouseLeft)) {
                result.dragging = true;
                ui->active_key = box->key;
            }
        }
    }

    if (result.dragging) {
        result.drag_delta = V2SubV2(result.mouse, ui->prev_mouse_pos);
    }

    return result;
}

function UI_Box *
UI_BoxMake(UI_BoxFlags flags, char *label) {
    BEGIN_TIMED_FUNCTION();
    UI_Box *result = 0;

    /* NOTE(hampus): There are 3 scenarios for the hashing fo the box:
// TODO(hampus): Maybe collapse scenario 1 and 2?
    1.
None of the flags needs persistent data
(UI_BoxFlag_HotAnimation, UI_BoxFlag_ActiveAnimation, etc.).
 This scenario is trivial, just don't hash the box and push it
to the frame arena

2.
The string is 0 and the flags needs persistent data (Checkbox etc.).
Use the current key stack and hope for the best?

3.
Ths string is not 0 and the flags needs persistent data. Hash the box.
*/

    UI_Key key = { 0 };

    b32 hashtag_found = false;
    char text[1024] = { 0 };
    if (!UI__WidgetFlagsRequiresHashing(flags)) {
        // NOTE(hampus): Scenario 1
        result = PushStruct(&ui->frame_arena, UI_Box);
        StringCopy(text, 1024, label);
    } else {

        for (u32 i = 0; i < StringLength(label); ++i) {
            if (label[i] == '#' && label[i + 1] == '#') {
                UI_PushKey(label + i + 2);
                hashtag_found = true;
                break;
            } else {
                text[i] = label[i];
            }
        }

        // NOTE(hampus): Scenario 2
        if (ui->window) {
            key = UI__HashStr(ui->window->full_key_stack, StringLength(ui->window->full_key_stack), 1);
        } else {
            // NOTE(hampus): This could be the case for a window.
            if (hashtag_found) {

            } else {
                key = UI__HashStr(label, StringLength(label), 1);
            }
        }

    }

    if (!UI_KeyIsNull(key)) {
        // NOTE(hampus): Hash the key
        u32 slot_index = key.key % MAX_NUM_WIDGETS;
        b32 slot_index_used = ui->boxes[slot_index]->used;

        if (slot_index_used) {
            UI_Box *existing_node = 0;
            UI_Box *n;
            for (n = ui->boxes[slot_index]; n != 0; n = n->hash_next) {
                if (UI_SameKey(n->key, key)) {
                    existing_node = n;
                    break;
                }
            }
            if (existing_node != 0) {
                result = existing_node;
            }

            if (existing_node == 0) {
                // TODO(hampus): Take from a permanent storage instead.
                // Otherwise some buttons with the same key won't be persistent.
                n = PushStruct(&ui->permanent_arena, UI_Box);
                n->hash_next = ui->boxes[slot_index];
                ui->boxes[slot_index] = n;
                result = n;
            }
        } else {
            result = ui->boxes[slot_index];
        }
    }

    ui->prev_box = ui->curr_box;
    ui->curr_box = result;

    result->key = key;
    result->flags = flags;

    if (ui->curr_layout_state->skip_next_pos) {
        result->skip_pos = true;
        ui->curr_layout_state->skip_next_pos = false;
    }

    StringCopy(result->text, 256, text);
    result->first = 0;
    result->last = 0;
    result->next = 0;
    result->prev = 0;
    result->parent = 0;

    // NOTE(hampus): Set the semantic sizes
    result->semantic_size[0].kind = UI_SizeKind_Pixels;
    result->semantic_size[0].value = (f32)TextLength(label);

    result->semantic_size[1].kind = UI_SizeKind_Pixels;
    result->semantic_size[1].value = 0;

    // NOTE(hampus): Extra padding
    result->semantic_size[0].value += 5;
    result->semantic_size[1].value += 12;

    // TODO(hampus): Use concept of a "cursor pos" like Dear ImGui? 
    // This would mean that UI_SameLine() wouldn't need to push parent,
    // it would only mean to change the cursor pos.
    result->computed_rel_position[0] = 0;
    result->computed_rel_position[1] = 0;

    result->computed_size[0] = 0;
    result->computed_size[1] = 0;

    UI__PushWidgetInParent(result);

    UI__CalcSemanticSize(result);

    UI__CalcRelativePosition(result);

    result->pos_rect = result->rect;

    b32 scroll = true;
    if (result->flags & UI_BoxFlag_NoScroll) {
        scroll = false;
    }

    if (scroll) {
        result->rect.min.y -= ui->window->box->scroll_y;
        result->rect.max.y -= ui->window->box->scroll_y;

        result->rect.min.x -= ui->window->box->scroll_x;
        result->rect.max.x -= ui->window->box->scroll_x;
    }

    result->used = true;

    if (UI_SameKey(result->key, ui->active_key)) {
        result->active_t += 0.01f;
    } else {
        result->active_t = 0;
    }

    if (UI_SameKey(result->key, ui->hot_key)) {
        result->hot_t += 0.01f;
    } else {
        result->hot_t = 0;
    }

    if (ui->curr_layout_state->next_background_color_set) {
        result->color = ui->curr_layout_state->next_background_color;
        result->color_set = true;
        ui->curr_layout_state->next_background_color_set = false;
    }

    result->hot_t = Clamp(0, result->hot_t, 1.0f);
    result->active_t = Clamp(0, result->active_t, 1.0f);

    result->last_frame_touched_index = ui->curr_frame_index;

    if (ui->curr_layout_state->next_text_align) {
        result->text_alignment = ui->curr_layout_state->next_text_align;
        ui->curr_layout_state->next_text_align = 0;
    }

    // TODO(hampus): Remove this.
    if (ui->curr_layout_state->same_line) {
        ui->curr_layout_state->same_line = false;
        UI_PopColumns();
    }

    ui->boxes_to_render[ui->num_boxes_to_render++] = result;

    result->window = ui->window;

    if (hashtag_found) {
        UI_PopKey();
    }

    END_TIMED_FUNCTION();
    return result;
}

function UI_Window *
UI_BeginWindow(char *title, f32 x0, f32 y0, f32 width, f32 height, b32 show_window) {
    BEGIN_TIMED_FUNCTION();
    UI_Window *window = PushStruct(&ui->frame_arena, UI_Window);
    UI_PushParent(window->box);
    StringCopy(window->title, 256, title);
    UI_PushWindow(window);

    UI_SetNextWidth(width);
    UI_SetNextPos(V2(x0, y0));
    UI_SkipScroll();
    UI_BoxFlags window_flags =
        UI_BoxFlag_Clickable |
        UI_BoxFlag_NoScroll |
        UI_BoxFlag_NoClip;

    if (show_window) {
        window_flags |= UI_BoxFlag_DrawText |
            UI_BoxFlag_DrawBackground |
            UI_BoxFlag_HotAnimation |
            UI_BoxFlag_ActiveAnimation;
    }

    window->box = UI_BoxMake(window_flags,
                             title);
    UI_PopParent();
        UI_PushParent(window->box);

    UI_Comm comm = UI_CommFromBox(window->box);

    UI_PushKey("background_rect");
    UI_SkipNextPos();
    UI_SetNextWidth(width);
    UI_SetNextHeight(height);
    UI_SetNextBackgroundColor(V4(0, 0, 0, 0.75f));
    UI_NoPadding();
    UI_BoxFlags background_flags = UI_BoxFlag_NoScroll | UI_BoxFlag_NoClip;
    if (show_window) {
        background_flags |= UI_BoxFlag_DrawBackground;
    }
    UI_Box *background = UI_BoxMake(background_flags, 0);

    // TODO(hampus): The +16 is only if a scrollbar is needed.
    // To fix this, the window need to be hashed
    f32 max_pos_y = window->box->max_pos_y + 16;
    window->box->max_pos_y = 0;

    f32 max_pos_x = window->box->max_pos_x + 16;
    window->box->max_pos_x = 0;

    f32 overflow_y = (max_pos_y - y0) / (height);
    f32 overflow_pixels_y = (overflow_y - 1.0f) * height;

    f32 overflow_x = (max_pos_x - x0) / (width);
    f32 overflow_pixels_x = (overflow_x - 1.0f) * width;

    f32 max_scroll_y = overflow_pixels_y;
    f32 max_scroll_x = overflow_pixels_x;

    f32 scroll_y = window->box->scroll_y;
    f32 scroll_x = window->box->scroll_x;

    b32 need_scroll_y = overflow_y > 1;
    b32 need_scroll_x = overflow_x > 1;

    window->need_scroll_y = need_scroll_y;
    window->need_scroll_x = need_scroll_x;

    UI_Comm background_comm = UI_CommFromBox(background);
    if (background_comm.scrolling && need_scroll_y) {
        window->box->scroll_y -= background_comm.scrolling * g_input->dt * 2000;
    }

    // TODO(hampus): Only works for one window. This applies
    // to all boxs, even though there are several windows
    Rect window_clip_rect = background->rect;

    // NOTE(hampus): For the scrollbars
    if (need_scroll_y)
        window_clip_rect.max.x -= 16;

    if (need_scroll_x)
        window_clip_rect.max.y -= 16;

    window_clip_rect.min.x = Clamp(0, window_clip_rect.min.x, 960);
    window_clip_rect.min.y = Clamp(0, window_clip_rect.min.y, 540);

    window_clip_rect.max.x = Clamp(0, window_clip_rect.max.x, 960);
    window_clip_rect.max.y = Clamp(0, window_clip_rect.max.y, 540);

    window->clip_rect = window_clip_rect;

    UI_PopKey();

    // NOTE(hampus): Vertical scrollbar
    if (need_scroll_y) {
        f32 rest_height = height - height / overflow_y;
        if (need_scroll_x) {
            rest_height -= 16;
            UI_SetNextHeight(height - 16);
        } else {
            UI_SetNextHeight(height);
        }

        UI_PushKey("scroll_bar_rect_y");

        UI_NoPadding();
        UI_SetNextWidth(16);
        UI_SkipNextPos();
        UI_NextAlign(UI_AlignmentFlag_Right);
        UI_SetNextBackgroundColor(V4(0, 0, 0, 0.75f));
        UI_SkipScroll();
        UI_Box *scrollbar_rect = UI_BoxMake(UI_BoxFlag_DrawBackground |
                                            UI_BoxFlag_Clickable |
                                            UI_BoxFlag_NoScroll |
                                            UI_BoxFlag_NoClip,
                                            0);
        UI_Comm scrollbar_comm = UI_CommFromBox(scrollbar_rect);
        if (scrollbar_comm.clicked) {
            f32 rel_y = scrollbar_comm.mouse.y - window->box->rect.max.y;

            // NOTE(hampus): To center the scrollbar
            rel_y -= (height / overflow_y) / 2;
            f32 share = rel_y / rest_height;

            share = Clamp(0, share, 1.0f);

            window->box->scroll_y = share * (max_scroll_y);
        }

        UI_PopKey();


        UI_PushKey("scroll_rect_y");

        if (overflow_y > 1.0f) {
            UI_SetNextHeight(height / overflow_y);
        } else {
            UI_SetNextHeight(height - 16.0f);
        }

        UI_SetNextWidth(12);
        UI_SkipNextPos();
        UI_SetNextPos(V2(window->box->rect.max.x - 14.0f, window->box->rect.max.y + rest_height * (scroll_y / max_scroll_y)));
        UI_SkipScroll();
        UI_Box *scroll_box = UI_BoxMake(UI_BoxFlag_DrawBackground |
                                        UI_BoxFlag_HotAnimation |
                                        UI_BoxFlag_Clickable |
                                        UI_BoxFlag_NoScroll |
                                        UI_BoxFlag_NoClip,
                                        0);

        UI_Comm scroll_comm = UI_CommFromBox(scroll_box);
        if (scroll_comm.dragging) {
            f32 rel_y = scrollbar_comm.mouse.y - window->box->rect.max.y;

            // NOTE(hampus): To center the scrollbar
            rel_y -= (height / overflow_y) / 2;
            f32 share = rel_y / rest_height;

            share = Clamp(0, share, 1.0f);

            window->box->scroll_y = share * (max_scroll_y);
        }

        window->box->scroll_y = Clamp(0, window->box->scroll_y, max_scroll_y);

        UI_PopKey();
    }


    // NOTE(hampus): Horizontal scrollbar
    if (need_scroll_x) {
        UI_PushKey("scroll_bar_rect_x");

        f32 rest_width = width - width / overflow_x;
        if (need_scroll_y) {
            rest_width -= 16;
            UI_SetNextWidth(width - 16);
        } else {
            UI_SetNextWidth(width);
        }

        UI_SetNextHeight(16);
        UI_SkipNextPos();
        UI_SetNextPos(V2(x0, y0 + height - 4));
        UI_SetNextBackgroundColor(V4(0, 0, 0, 0.75f));
        UI_SkipScroll();
        UI_Box *scrollbar_rect = UI_BoxMake(UI_BoxFlag_DrawBackground |
                                            UI_BoxFlag_Clickable |
                                            UI_BoxFlag_NoScroll |
                                            UI_BoxFlag_NoClip,
                                            0);
        UI_Comm scrollbar_comm = UI_CommFromBox(scrollbar_rect);
        if (scrollbar_comm.dragging) {
            f32 rel_x = scrollbar_comm.mouse.x - x0;

            // NOTE(hampus): To center the scrollbar
            rel_x -= (width / overflow_x) / 2;
            f32 share = rel_x / rest_width;

            share = Clamp(0, share, 1.0f);

            window->box->scroll_x = share * (max_scroll_x);
        }

        UI_PopKey();


        UI_PushKey("scroll_rect_x");

        UI_SetNextWidth(width / overflow_x);
        UI_SetNextHeight(12);
        UI_SkipNextPos();
        UI_SetNextPos(V2(x0 + rest_width * (scroll_x / max_scroll_x), y0 + height - 2));
        UI_SkipScroll();
        UI_Box *scroll_box = UI_BoxMake(UI_BoxFlag_DrawBackground |
                                        UI_BoxFlag_HotAnimation |
                                        UI_BoxFlag_Clickable |
                                        UI_BoxFlag_NoScroll |
                                        UI_BoxFlag_NoClip,
                                        0);

        UI_Comm scroll_comm = UI_CommFromBox(scroll_box);
        if (scroll_comm.dragging) {
            f32 rel_x = scroll_comm.mouse.x - x0;

            // NOTE(hampus): To center the scrollbar
            rel_x -= (width / overflow_x) / 2;
            f32 share = rel_x / rest_width;

            share = Clamp(0, share, 1.0f);

            window->box->scroll_x = share * (max_scroll_x);
        }

        window->box->scroll_x = Clamp(0, window->box->scroll_x, max_scroll_x);

        UI_PopKey();

    }

    window->box->first_box = 0;
    END_TIMED_FUNCTION();
    return window;
}

function UI_Comm
UI_Button(char *text) {
    UI_PushKey(text);
    UI_Box *box = UI_BoxMake(UI_BoxFlag_Clickable |
                             UI_BoxFlag_DrawText |
                             UI_BoxFlag_DrawBackground |
                             UI_BoxFlag_HotAnimation |
                             UI_BoxFlag_ActiveAnimation,
                             text);
    UI_PopKey();
    UI_Comm comm = UI_CommFromBox(box);

    return comm;
}

function UI_Comm
UI_Text(char *format, ...) {
    BEGIN_TIMED_FUNCTION();
    char text[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(text, 1024, format, args);
    va_end(args);

    UI_Box *box = UI_BoxMake(UI_BoxFlag_DrawText,
                             text);

    UI_Comm comm = UI_CommFromBox(box);
    END_TIMED_FUNCTION();
    return comm;
}

function UI_Comm
UI_Slider(char *test, f32 *val, f32 min, f32 max) {
    char label[1024] = { 0 };
    b32 hashtag_found = false;
    for (u32 i = 0; i < StringLength(test); ++i) {
        if (test[i] == '#' && test[i + 1] == '#') {
            UI_PushKey(test + i + 2);
            hashtag_found = true;
            break;
        } else {
            label[i] = test[i];
        }
    }

    UI_PushKey(label);
    UI_Box *label_box = UI_BoxMake(UI_BoxFlag_DrawText,
                                   label);

    UI_SameLine();
    UI_PushKey("rect");
    UI_SetNextWidth(60.0f);
    StringFormat(label, 1024, "%.02f", *val);
    UI_Box *box = UI_BoxMake(UI_BoxFlag_Clickable |
                             UI_BoxFlag_DrawText |
                             UI_BoxFlag_DrawBackground |
                             UI_BoxFlag_HotAnimation |
                             UI_BoxFlag_ActiveAnimation,
                             label);
    UI_PopKey();

    UI_Comm comm = UI_CommFromBox(box);

    f32 rect_width = box->rect.max.x - box->rect.min.x;
    if (comm.clicked || comm.dragging) {
        f32 rel_mouse_x = comm.mouse.x - box->rect.min.x + 0.5f;
        f32 share = rel_mouse_x / rect_width;
        *val = min + (max - min) * share;
    }

    *val = Clamp(min, *val, max);

    f32 distance_from_zero = 0 - min;
    f32 min0 = min + distance_from_zero;
    f32 max0 = max + distance_from_zero;
    f32 val0 = *val + distance_from_zero;
#if 0
    UI_SameLine();
    UI_SetNextWidth(1.0f);
    UI_SetNextRelativePos(val0 / max0 - (1.0f / rect_width));
    UI_SetNextBackgroundColor(V4(1.0f, 1.0f, 1.0f, 1.0f));
    UI_SkipNextPos();
    // TODO(hampus): The mark does not need to get scrolled
    // since it's position is based from the buttons top-left
    // corner
    UI_Box *mark_box = UI_BoxMake(UI_BoxFlag_DrawBackground | UI_BoxFlag_NoScroll, 0);
#endif
    UI_PopKey();

    if (hashtag_found) {
        UI_PopKey();
    }

    return comm;
}

function UI_Comm
UI_Check(char *text, b32 val) {

    char label[1024] = { 0 };
    b32 hashtag_found = false;
    for (u32 i = 0; i < StringLength(text); ++i) {
        if (text[i] == '#' && text[i + 1] == '#') {
            UI_PushKey(text + i + 2);
            hashtag_found = true;
            break;
        } else {
            label[i] = text[i];
        }
    }

    UI_PushKey(label);

    UI_Box *label_box = UI_BoxMake(UI_BoxFlag_DrawText,
                                   label);
    UI_SameLine();
    UI_SetNextWidth(12.0f);
    UI_SetNextHeight(12.0f);
    UI_PushKey("check_rect");
    UI_Box *check_box = UI_BoxMake(UI_BoxFlag_HotAnimation |
                                   UI_BoxFlag_ActiveAnimation |
                                   UI_BoxFlag_Clickable |
                                   UI_BoxFlag_DrawBackground,
                                   0);
    UI_PopKey();
    UI_Comm comm = UI_CommFromBox(check_box);


    UI_BoxFlags check_area_flags = 0;
    if (val) {
        check_area_flags |= UI_BoxFlag_DrawBackground;
    }

    UI_SameLine();
    UI_SetNextWidth(10.0f);
    UI_SetNextHeight(10.0f);
    UI_SetNextRelativePos(0.0f);
    UI_SetNextBackgroundColor(V4(1.0f, 1.0f, 1.0f, 1.0f));
    UI_Box *mark_box2 = UI_BoxMake(check_area_flags,
                                   0);

    UI_PopKey();

    if (hashtag_found) {
        UI_PopKey();
    }
    return comm;
}

function void
UI_ColorPicker(char *text, v4 *color) {
    char label[1024] = { 0 };
    b32 hashtag_found = false;
    for (u32 i = 0; i < StringLength(text); ++i) {
        if (text[i] == '#' && text[i + 1] == '#') {
            UI_PushKey(text + i + 2);
            hashtag_found = true;
            break;
        } else {
            label[i] = text[i];
        }
    }

    UI_PushKey(label);

    UI_Box *label_box = UI_BoxMake(UI_BoxFlag_DrawText,
                                   label);

    UI_Slider("R:", &color->r, 0.0f, 1.0f);

    UI_SameLine();
    UI_Slider("G:", &color->g, 0.0f, 1.0f);

    UI_SameLine();
    UI_Slider("B:", &color->b, 0.0f, 1.0f);

    UI_SameLine();
    UI_Slider("A:", &color->a, 0.0f, 1.0f);

    UI_SameLine();
    UI_SetNextWidth(12);
    UI_SetNextHeight(12);
    UI_SetNextBackgroundColor(*color);
    UI_Box *box = UI_BoxMake(UI_BoxFlag_DrawBackground | UI_BoxFlag_Clickable, 0);

    UI_Comm color_comm = UI_CommFromBox(box);
    local_persist b32 active = false;
    if (color_comm.clicked) {
        active = !active;
    }

    if (active) {

#if 1
        UI_BeginWindow("Color", 0, 0, 300, 300, true);

        UI_PopWindow();
#endif
    }

    UI_PopKey(); // label

    if (hashtag_found) {
        UI_PopKey(); // "##"
    }
}

function UI_Comm
UI_Divider() {
    if (ui->window->need_scroll_y) {
        UI_SetNextWidth(ui->window->box->computed_size[0] - 6 - 16);
    } else {
        UI_SetNextWidth(ui->window->box->computed_size[0] - 20);
    }
    UI_SetNextHeight(1);
    UI_Box *box = UI_BoxMake(UI_BoxFlag_DrawBackground, 0);

    UI_Comm comm = UI_CommFromBox(box);

    return(comm);
}

char test[256] = { 0 };
s32 test_index = 0;

enum ASCII {
    NUL = 0, // null
    SOH = 1, // start of heading,
    STX = 2, // start of text
    ETX = 3, // end of text,
    EOT = 4, // end of transmission
    ENQ = 5, // enquiry
    ACK = 6, // acknowledge
    BEL = 7, // bell
    BACKSPACE = 8,  // backspace
    TAB = 9, // horizontal tab
    LF = 10, // NL line feed, new line
    VT = 11, // vertical tab
    FF = 12, // NP form feed, new page
    RETURN = 13, // carriage return
    SO = 14, // shift out
    SI = 15, // shift in
    DLE = 16, // data link escape
    DC1 = 17, // device control 1
    DC2 = 18, // device control 2
    DC3 = 19, // device control 3
    DC4 = 20, // device control 4
};

function UI_Comm
UI_InputBox(char *text) {
    UI_PushKey(text);
    if (text) {
        UI_Text(text);
        UI_SameLine();
    }
    UI_SetNextWidth(100.0f);
    UI_SetNextTextAlign(UI_AlignmentFlag_Left);

    char hash[128] = { 0 };
    UI_PushKey("text_box");
    StringFormat(hash, 128, "%s##", test);
    UI_Box *text_box = UI_BoxMake(UI_BoxFlag_Clickable |
                                  UI_BoxFlag_DrawBackground |
                                  UI_BoxFlag_DrawText |
                                  UI_BoxFlag_HotAnimation |
                                  UI_BoxFlag_ActiveAnimation,
                                  hash);
    UI_PopKey();

    UI_Comm comm = UI_CommFromBox(text_box);

    if (comm.clicked) {
        comm.box->selected = true;
        platform->ToggleCursor();
    }

    if (comm.box->selected) {
        ui->active_key = comm.box->key;
        ui->hot_key = comm.box->key;
        if (KeyPressed(g_input, KeyCode_Escape)) {
            comm.box->selected = false;
            platform->ToggleCursor();
        } else {
            if (g_input->last_char) {
                char ch = g_input->last_char;
                switch (ch) {
                    case BACKSPACE: {
                        if (StringLength(test) != 0) {
                            test[StringLength(test) - 1] = 0;
                            test_index--;
                        }
                    } break;

                    case RETURN: {

                    } break;

                    default:
                    {
                        test[test_index++] = ch;
                    } break;
                }
            }
        }
    }

    // TODO(hampus): Render the cursor

    UI_PopKey();

    return(comm);
}
