#define _CRT_SECURE_NO_WARNINGS

#if AENIGMA_INTERNAL
#include <stdio.h>
#endif

#include <windows.h>
#include <xinput.h>
#include <dsound.h>

#include "platform.h"
#include "win32_main.h"

global b32 g_running;
global Win32RenderBuffer g_backbuffer;
global s64 g_perf_count_freq;
global b32 g_pause;
global b32 DEBUG_g_show_cursor;
global WINDOWPLACEMENT g_window_pos = { sizeof(g_window_pos) };

#include "win32_platform.c"

function void
Win32GetEXEFileName(Win32State *state) {
    DWORD size_of_file_name = GetModuleFileNameA(0, state->exe_file_name, sizeof(state->exe_file_name));
    state->one_past_exe_file_name_slash = state->exe_file_name;
    for (char *scan = state->exe_file_name; *scan; ++scan) {
        if (*scan == '\\') {
            state->one_past_exe_file_name_slash = scan + 1;
        }
    }
}

function void
Win32BuildEXEPathFileName(Win32State *state, char *file_name, s32 dest_count, char *dest) {
    StringCat(state->exe_file_name, state->one_past_exe_file_name_slash - state->exe_file_name,
              file_name, StringLength(file_name),
              dest, dest_count);
}

function inline FILETIME
Win32GetLastWriteTime(char *file_name) {
    FILETIME last_write_time = { 0 };

    WIN32_FILE_ATTRIBUTE_DATA data = { 0 };
    if (GetFileAttributesExA(file_name, GetFileExInfoStandard, &data)) {
        last_write_time = data.ftLastWriteTime;
    }

    return last_write_time;
}

function Win32GameCode
Win32LoadGameCode(char *source_dll_name, char *temp_dll_name) {
    Win32GameCode result = { 0 };
#if AENIGMA_INTERNAL
    result.dll_last_write_time = Win32GetLastWriteTime(source_dll_name);

    CopyFileA(source_dll_name, temp_dll_name, FALSE);
    DWORD error = GetLastError();
    result.game_code_dll = LoadLibraryA(temp_dll_name);
    if (result.game_code_dll) {
        result.update_and_render = (Game_Update_And_Render *)GetProcAddress(result.game_code_dll, "GameUpdateAndRender");

        result.valid = (result.update_and_render != 0);
    }
#elif AENIGMA_RELEASE
    result.game_code_dll = LoadLibraryA(source_dll_name);
    if (result.game_code_dll) {
        result.update_and_render = (Game_Update_And_Render *)GetProcAddress(result.game_code_dll, "GameUpdateAndRender");

        result.valid = (result.update_and_render != 0);
    }
#endif
    if (!result.valid) {
        result.update_and_render = 0;
    }

    return result;
}

function void
Win32UnloadGameCode(Win32GameCode *game_code) {
    if (game_code->game_code_dll) {
        FreeLibrary(game_code->game_code_dll);
        game_code->game_code_dll = 0;
    }

    if (game_code->valid) {
        game_code->update_and_render = 0;
        game_code->valid = false;
    }
}

function inline Win32WindowDimension
Win32GetWindowDimension(HWND window) {

    Win32WindowDimension result;

    RECT client_rect;
    GetClientRect(window, &client_rect);

    result.width = client_rect.right - client_rect.left;
    result.height = client_rect.bottom - client_rect.top;

    return result;
}

function void
Win32ResizeRenderBuffer(Win32RenderBuffer *buffer, s32 width, s32 height) {

    // @Incomplete:  Bulletproof this
    // Maybe don't free first, free after, then free first if that fails.
    if (buffer->memory) {
        VirtualFree(buffer->memory, 0, MEM_RELEASE);
    }

    buffer->width = width;
    buffer->height = height;
    buffer->bytes_per_pixel = 4;
    buffer->pitch = buffer->bytes_per_pixel * buffer->width;

    buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
    buffer->info.bmiHeader.biWidth = buffer->width;
    buffer->info.bmiHeader.biHeight = -buffer->height;
    buffer->info.bmiHeader.biPlanes = 1;
    buffer->info.bmiHeader.biBitCount = 32;
    buffer->info.bmiHeader.biCompression = BI_RGB;

    s32 bitmap_memory_size = buffer->width * buffer->height * buffer->bytes_per_pixel;
    buffer->memory = VirtualAlloc(0, bitmap_memory_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
}

function inline void
Win32DisplayBufferInWindow(Win32RenderBuffer *buffer, HDC device_context, s32 window_width, s32 window_height) {
#if 0
    if ((window_width >= buffer->width * 2) && (window_height >= buffer->height * 2)) {
        s32 offset_x = (window_width - buffer->width * 2) / 2;
        s32 offset_y = (window_height - buffer->height * 2) / 2;

        PatBlt(device_context, 0, 0, window_width, offset_y, BLACKNESS);
        PatBlt(device_context, 0, 0, offset_x, window_height, BLACKNESS);

        PatBlt(device_context, 0, window_height - offset_y, window_width, window_height, BLACKNESS);
        PatBlt(device_context, window_width - offset_x, 0, window_width, window_height, BLACKNESS);

        StretchDIBits(device_context,
                      offset_x, offset_y, buffer->width * 2, buffer->height * 2, // destination
                      0, 0, buffer->width, buffer->height, // source
                      buffer->memory,
                      &buffer->info,
                      DIB_RGB_COLORS, SRCCOPY);
    } else {
        s32 offset_x = (window_width - buffer->width) / 2;
        s32 offset_y = (window_height - buffer->height) / 2;

        PatBlt(device_context, 0, 0, window_width, offset_y, BLACKNESS);
        PatBlt(device_context, 0, 0, offset_x, window_height, BLACKNESS);

        PatBlt(device_context, 0, window_height - offset_y, window_width, window_height, BLACKNESS);
        PatBlt(device_context, window_width - offset_x, 0, window_width, window_height, BLACKNESS);
        // @Important: For prototyping purposes, we're going to always blit
        // 1-to-1 pixels to make sure we don't introduce artifacts with
        // stretching while we are learning to code the renderer!

        StretchDIBits(device_context,
                      offset_x, offset_y, buffer->width, buffer->height, // destination
                      0, 0, buffer->width, buffer->height, // source
                      buffer->memory,
                      &buffer->info,
                      DIB_RGB_COLORS, SRCCOPY);
    }
#else
    StretchDIBits(device_context,
                  0, 0, window_width, window_height, // destination
                  0, 0, buffer->width, buffer->height, // source
                  buffer->memory,
                  &buffer->info,
                  DIB_RGB_COLORS, SRCCOPY);
#endif

}

function void
Win32GetInputFileLocation(Win32State *state, s32 slot_index, b32 input_stream, char *dest, s32 dest_count) {
#if AENIGMA_INTERNAL
    char temp[64];
    sprintf_s(temp, sizeof(temp), "loop_edit_%d_%s.hmi", slot_index, input_stream ? "input" : "state");
    Win32BuildEXEPathFileName(state, temp, dest_count, dest);
#endif
}

function Win32ReplayBuffer *
Win32GetReplayBuffer(Win32State *state, u32 input_recording_index) {
    Assert(input_recording_index < ArrayCount(state->replay_buffers));
    Win32ReplayBuffer *result = &state->replay_buffers[input_recording_index];
    return result;
}

function void
Win32BeginRecordingInput(Win32State *state, u32 input_recording_index) {
    Win32ReplayBuffer *replay_buffer = Win32GetReplayBuffer(state, input_recording_index);
    if (replay_buffer->memory_block) {

        state->input_recording_index = input_recording_index;

        char file_name[WIN32_STATE_FILE_NAME_COUNT];
        Win32GetInputFileLocation(state, input_recording_index, true, file_name, sizeof(file_name));
        state->recording_handle = CreateFileA(file_name, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
        CopyMemory(replay_buffer->memory_block, state->game_memory_block, state->total_size);
    }
}

function void
Win32EndRecordingInput(Win32State *state) {
    CloseHandle(state->recording_handle);
    state->input_recording_index = 0;
}

function void
Wi3n2BeginInputPlayBack(Win32State *state, s32 input_playing_index) {
    Win32ReplayBuffer *replay_buffer = Win32GetReplayBuffer(state, input_playing_index);
    if (replay_buffer->memory_block) {

        char file_name[WIN32_STATE_FILE_NAME_COUNT];
        Win32GetInputFileLocation(state, input_playing_index, true, file_name, sizeof(file_name));
        state->input_playing_index = input_playing_index;
        state->play_back_handle = CreateFileA(file_name, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
        CopyMemory(state->game_memory_block, replay_buffer->memory_block, state->total_size);
    }
}

function void
Win32EndInputPlayBack(Win32State *state) {
    CloseHandle(state->play_back_handle);
    state->input_playing_index = 0;
}

function void
Win32RecordInput(Win32State *state, Input *input) {
    DWORD bytes_written;
    WriteFile(state->recording_handle, input, sizeof(*input), &bytes_written, 0);
}

function void
Win32PlayBackInput(Win32State *state, Input *input) {
    DWORD bytes_read;
    ReadFile(state->play_back_handle, input, sizeof(*input), &bytes_read, 0);
    if (bytes_read == 0) {
        // We've hit the end of the stream, go back to the beginning
        s32 playing_index = state->input_playing_index;
        Win32EndInputPlayBack(state);
        Wi3n2BeginInputPlayBack(state, playing_index);
    } else {
        // There is still input
    }
}

function void
Win32ToggleFullscreen(HWND window) {
    DWORD window_style = GetWindowLong(window, GWL_STYLE);
    if (window_style & WS_OVERLAPPEDWINDOW) {
        MONITORINFO monitor_info = { sizeof(monitor_info) };
        if (GetWindowPlacement(window, &g_window_pos) &&
            GetMonitorInfo(MonitorFromWindow(window, MONITOR_DEFAULTTOPRIMARY), &monitor_info)) {
            SetWindowLong(window, GWL_STYLE, window_style & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(window, HWND_TOP,
                         monitor_info.rcMonitor.left, monitor_info.rcMonitor.top,
                         monitor_info.rcMonitor.right - monitor_info.rcMonitor.left,
                         monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    } else {
        SetWindowLong(window, GWL_STYLE, window_style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(window, &g_window_pos);
        SetWindowPos(window, NULL, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}

Input *g_input;
Win32State *g_state;
LRESULT CALLBACK
Win32WindowProcedure(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {

    LRESULT result = 0;

    switch (message) {
        case WM_SETCURSOR: {
            if (DEBUG_g_show_cursor) {
                result = DefWindowProcA(window, message, wparam, lparam);
            } else {
                SetCursor(0);
            }
        } break;

        case WM_SIZE: {
        } break;

        case WM_DESTROY: {
            // @Incomplete:  Handle this as an error - recreate window?
            g_running = false;
        } break;

        case WM_CLOSE: {
            // @Incomplete:  Handle this with a message to the user?
            g_running = false;
        } break;

        case WM_ACTIVATEAPP: {
        } break;

        case WM_CHAR: {
            g_input->last_char = (char)wparam;
        } break;

        case WM_QUIT: {
            g_running = false;
        } break;

        case WM_MOUSEWHEEL: {
            g_input->mouse_wheel = GET_WHEEL_DELTA_WPARAM(wparam) / WHEEL_DELTA;
        } break;

        case WM_LBUTTONDOWN: {
            g_input->left_mouse_btn.is_down = true;
        } break;

        case WM_LBUTTONUP: {
            g_input->left_mouse_btn.is_down = false;
        } break;

        case WM_RBUTTONDOWN: {
            g_input->right_mouse_btn.is_down = true;
        } break;

        case WM_RBUTTONUP: {
            g_input->right_mouse_btn.is_down = false;
        } break;

        case WM_MBUTTONDOWN: {
            g_input->middle_mouse_btn.is_down = true;
        } break;

        case WM_MBUTTONUP: {
            g_input->middle_mouse_btn.is_down = false;
        } break;

        case WM_SYSKEYUP:
        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:
        case WM_KEYUP: {
            u32 vk_code = (u32)wparam;
            b32 was_down = ((lparam & (1 << 30)) != 0);
            b32 is_down = ((lparam & (1 << 31)) == 0);

            if (was_down != is_down) {
                if (vk_code == 'W') {
                    g_input->w.is_down = is_down;
                } else if (vk_code == 'S') {
                    g_input->s.is_down = is_down;
                } else if (vk_code == 'A') {
                    g_input->a.is_down = is_down;
                } else if (vk_code == 'D') {
                    g_input->d.is_down = is_down;
                } else if (vk_code == 'Q') {
                    g_input->q.is_down = is_down;
                } else if (vk_code == 'E') {
                    g_input->e.is_down = is_down;
                } else if (vk_code == 'Q') {
                    g_input->q.is_down = is_down;
                } else if (vk_code == 'E') {
                    g_input->e.is_down = is_down;
                } else if (vk_code == 'N') {
                    g_input->n.is_down = is_down;
                } else if (vk_code == VK_UP) {
                    g_input->up.is_down = is_down;
                } else if (vk_code == VK_DOWN) {
                    g_input->down.is_down = is_down;
                } else if (vk_code == VK_LEFT) {
                    g_input->left.is_down = is_down;
                } else if (vk_code == VK_RIGHT) {
                    g_input->right.is_down = is_down;
                } else if (vk_code == VK_ESCAPE) {
                    g_input->escape.is_down = is_down;
                } else if (vk_code == VK_SPACE) {
                    g_input->space.is_down = is_down;
                } else if (vk_code == VK_RETURN) {
                    g_input->enter.is_down = is_down;
                } else if (vk_code == VK_SHIFT) {
                    g_input->shift.is_down = is_down;
                } else if (vk_code == 'P') {
                    if (!was_down) {
                        // g_pause = !g_pause;
                    }
                } else if (vk_code == 'L') {
                    g_input->l.is_down = is_down;
                } else if (vk_code == VK_F1) {
                    g_input->f1.is_down = is_down;
                } else if (vk_code == VK_F2) {
                    g_input->f2.is_down = is_down;
                } else if (vk_code == VK_F3) {
                    if (is_down) {
                        STARTUPINFOA test = { 0 };
                        PROCESS_INFORMATION info = { 0 };
                        CreateProcessA("..\\src\\build.bat", 0, 0, 0, FALSE, NORMAL_PRIORITY_CLASS, 0, 0, &test, &info);
                    }
                    g_input->f3.is_down = is_down;
                } else if (vk_code == VK_F4) {
                    g_input->f4.is_down = is_down;
                } else if (vk_code == VK_F5) {
                    g_input->f5.is_down = is_down;
                    if (is_down) {
                        if (g_state->input_playing_index == 0) {
                            if (g_state->input_recording_index == 0) {
                                Win32BeginRecordingInput(g_state, 1);
                            } else {
                                Win32EndRecordingInput(g_state);
                            }
                        } else {
                            Win32EndInputPlayBack(g_state);
                        }
                    }
                } else if (vk_code == VK_F6) {
                    g_input->f6.is_down = is_down;
                    if (is_down) {
                        if (g_state->input_playing_index == 0) {
                            Wi3n2BeginInputPlayBack(g_state, 1);
                        } else {
                            Win32EndInputPlayBack(g_state);
                        }
                    }
                } else if (vk_code == VK_F7) {
                    g_input->f7.is_down = is_down;
                } else if (vk_code == VK_F8) {
                    g_input->f8.is_down = is_down;
                } else if (vk_code == VK_F9) {
                    g_input->f9.is_down = is_down;
                } else if (vk_code == VK_F10) {
                    g_input->f10.is_down = is_down;
                } else if (vk_code == VK_F11) {
                    g_input->f11.is_down = is_down;
                    if (is_down) {
                        Win32ToggleFullscreen(window);
                    }
                } else if (vk_code == VK_F12) {
                    g_input->f12.is_down = is_down;
                } else if (vk_code == VK_DELETE) {
                    g_input->delete.is_down = is_down;
                } else if (vk_code == 220) {
                    g_input->console.is_down = is_down;
                } else if (vk_code == '0') {
                    g_input->num0.is_down = is_down;
                } else if (vk_code == '1') {
                    g_input->num1.is_down = is_down;
                } else if (vk_code == '2') {
                    g_input->num2.is_down = is_down;
                } else if (vk_code == '3') {
                    g_input->num3.is_down = is_down;
                } else if (vk_code == '4') {
                    g_input->num4.is_down = is_down;
                } else if (vk_code == '5') {
                    g_input->num5.is_down = is_down;
                } else if (vk_code == '6') {
                    g_input->num6.is_down = is_down;
                } else if (vk_code == '7') {
                    g_input->num7.is_down = is_down;
                } else if (vk_code == '8') {
                    g_input->num8.is_down = is_down;
                } else if (vk_code == '9') {
                    g_input->num9.is_down = is_down;
                }
            }

            b32 alt_key_was_down = ((lparam & (1 << 29)));

            g_input->alt_down = alt_key_was_down;

            if (vk_code == VK_F4 && alt_key_was_down) {
                g_running = false;
            }
        } break;


        case WM_PAINT: {
            PAINTSTRUCT paint;
            HDC device_context = BeginPaint(window, &paint);

            Win32WindowDimension dimension = Win32GetWindowDimension(window);
            Win32DisplayBufferInWindow(&g_backbuffer, device_context, dimension.width, dimension.height);

            EndPaint(window, &paint);
        } break;

        default: {
            result = DefWindowProcA(window, message, wparam, lparam);
        } break;
    }

    return result;
}

function void
Win32ProcessPendingMessages(Win32State *state, Input *input) {

    MSG message;
    while (PeekMessageA(&message, 0, 0, 0, PM_REMOVE)) {

        switch (message.message) {
            default: {
                TranslateMessage(&message);
                DispatchMessageA(&message);
            } break;
        }
    }
}

function inline LARGE_INTEGER
Win32GetWallClock(void) {
    LARGE_INTEGER result;
    QueryPerformanceCounter(&result);
    return result;
}

function inline f32
Win32GetSecondsElapsed(LARGE_INTEGER start, LARGE_INTEGER end) {
    f32 seconds_elapsed;
    seconds_elapsed = (((f32)(end.QuadPart - start.QuadPart)) / (f32)g_perf_count_freq);
    return seconds_elapsed;
}

function LRESULT CALLBACK
Win32FadeWindowCallback(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    LRESULT result = 0;

    switch (message) {
        case WM_CLOSE: {
        } break;

        case WM_SETCURSOR: {
            SetCursor(0);
        } break;

        default: {
            result = DefWindowProcA(window, message, wparam, lparam);
        } break;
    }

    return result;
}


function void
Win32SetFadeAlpha(HWND window, f32 alpha) {
    BYTE windows_alpha = (BYTE)(alpha * 255.0f);
    if (alpha == 0) {
        if (IsWindowVisible(window)) {
            ShowWindow(window, SW_HIDE);
        }
    } else {
        SetLayeredWindowAttributes(window, RGB(0, 0, 0), windows_alpha, LWA_ALPHA);
        if (!IsWindowVisible(window)) {
            ShowWindow(window, SW_SHOW);
        }
    }
}

function void
Win32InitFader(Win32Fader *fader, HINSTANCE instance) {
    WNDCLASSA window_class = { 0 };

    window_class.style = CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc = Win32FadeWindowCallback;
    window_class.hInstance = instance;
    window_class.hCursor = LoadCursor(0, IDC_ARROW);
    window_class.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    window_class.lpszClassName = "AenigmaFadeWindowClassName";

    if (RegisterClassA(&window_class)) {
        fader->window =
            CreateWindowExA(
                WS_EX_LAYERED,
                window_class.lpszClassName,
                "Aenigma",
                WS_OVERLAPPEDWINDOW,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                0,
                0,
                instance,
                0);
        if (fader->window) {
            Win32ToggleFullscreen(fader->window);
        }
    }
}

function void
Win32BeginFadeToGame(Win32Fader *fader) {
    fader->state = WIN32_FADE_FADING_IN;
    fader->alpha = 0.0f;
}

function void
Win32BeginFadeToDesktop(Win32Fader *fader) {
    if (fader->state == WIN32_FADE_INACTIVE) {
        fader->state = WIN32_FADE_FADING_GAME;
        fader->alpha = 0.0f;
    }
}

function Win32FaderState
Win32UpdateFade(Win32Fader *fader, f32 dt, HWND game_window) {
    switch (fader->state) {
        case WIN32_FADE_FADING_IN: {
            if (fader->alpha >= 1.0f) {
                Win32SetFadeAlpha(fader->window, 1.0f);
                ShowWindow(game_window, SW_SHOW);
                InvalidateRect(game_window, 0, TRUE);
                UpdateWindow(game_window);

                fader->state = WIN32_FADE_WAITING_FOR_SHOW;
            } else {
                Win32SetFadeAlpha(fader->window, fader->alpha);
                fader->alpha += dt;
            }
        } break;

        case WIN32_FADE_WAITING_FOR_SHOW: {
            Win32SetFadeAlpha(fader->window, 0.0f);
            fader->state = WIN32_FADE_INACTIVE;
        } break;

        case WIN32_FADE_INACTIVE: {
        } break;

        case WIN32_FADE_FADING_GAME: {
            if (fader->alpha >= 1.0f) {
                Win32SetFadeAlpha(fader->window, 1.0f);
                ShowWindow(game_window, SW_HIDE);
                fader->state = WIN32_FADE_FADING_OUT;
            } else {
                Win32SetFadeAlpha(fader->window, fader->alpha);
                fader->alpha += dt;
            }
        } break;

        case WIN32_FADE_FADING_OUT: {
            fader->alpha -= dt;
            if (fader->alpha <= 0.0f) {
                Win32SetFadeAlpha(fader->window, 0.0f);
                fader->state = WIN32_FADE_WAITING_FOR_CLOSE;
            } else {
                Win32SetFadeAlpha(fader->window, fader->alpha);
            }
        } break;

        case WIN32_FADE_WAITING_FOR_CLOSE: {
        } break;

        default: {
            Assert(!"Unrecognized fader state!");
        } break;
    }

    return fader->state;
}

s32 CALLBACK WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR command_line, s32 show_code) {
#if 1
    AllocConsole();
    FILE *dummy;
    freopen_s(&dummy, "CONOUT$", "w", stdout);
#endif

    Win32State state = { 0 };
    g_state = &state;

#if AENIGMA_INTERNAL
    HANDLE assets_folder = FindFirstChangeNotification(".", false, FILE_NOTIFY_CHANGE_LAST_WRITE);
#endif
    //HANDLE src_folder = FindFirstChangeNotification("..\\src\\", false, FILE_NOTIFY_CHANGE_LAST_WRITE);

    g_hand_cursor = LoadCursorA(0, IDC_HAND);
    g_arrow_cursor = LoadCursorA(0, IDC_ARROW);
    g_slider_cursor = LoadCursorA(0, IDC_SIZEWE);

    Win32GetEXEFileName(&state);

#if AENIGMA_INTERNAL
    char source_game_code_dll_full_path[WIN32_STATE_FILE_NAME_COUNT];
    Win32BuildEXEPathFileName(&state, "aenigma.dll", WIN32_STATE_FILE_NAME_COUNT, source_game_code_dll_full_path);

    char temp_game_code_dll_full_path[WIN32_STATE_FILE_NAME_COUNT];
    Win32BuildEXEPathFileName(&state, "aenigma_temp.dll", WIN32_STATE_FILE_NAME_COUNT, temp_game_code_dll_full_path);
#endif

    LARGE_INTEGER perf_count_freq_result;
    QueryPerformanceFrequency(&perf_count_freq_result);
    g_perf_count_freq = perf_count_freq_result.QuadPart;

    b32 sleep_is_granular = (timeBeginPeriod(1) == TIMERR_NOERROR);

#if AENIGMA_FADE
    Win32Fader fader = { 0 };
    Win32InitFader(&fader, instance);
#endif

    Win32ResizeRenderBuffer(&g_backbuffer, config.game_res_width, config.game_res_height);

    DEBUG_g_show_cursor = true;
    WNDCLASSEXA window_class = { 0 };

    window_class.cbSize = sizeof(window_class);
    window_class.style = CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc = Win32WindowProcedure;
    window_class.hInstance = instance;
    window_class.lpszClassName = "AenigmaWindowClass";
    window_class.hCursor = LoadCursorA(0, IDC_ARROW);

    if (!RegisterClassExA(&window_class)) {
        // @Incomplete:  Logging
        OutputDebugStringA("Failed to register window class!\n");
        return -1;
    }

    HWND window = CreateWindowExA(0, window_class.lpszClassName, "Aenigma", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, instance, 0);

#if AENIGMA_FADE
    Win32ToggleFullscreen(window);
#elif AENIGMA_RELEASE
    Win32ToggleFullscreen(window);
    ShowWindow(window, true);
#else
    ShowWindow(window, true);
#endif

    if (!window) {
        // @Incomplete: Logging
        OutputDebugStringA("Failed to register window class!\n");
        return -1;
    }

    // @Important: Since we specified CS_OWNDC, we can just 
    // get one device context and use it forever because we 
    // are not sharing it with anyone.
    HDC device_context = GetDC(window);

    s32 monitor_refresh_hz = 60;
    s32 win32_refresh_rate = GetDeviceCaps(device_context, VREFRESH);

    if (win32_refresh_rate > 1) {
        monitor_refresh_hz = win32_refresh_rate;
    }

    // @Incomplete:  How do we reliably query this on Windows?
    f32 game_update_hz = ((f32)monitor_refresh_hz / 1.0f);
    f32 target_seconds_per_frame = 1.0f / game_update_hz;

    g_running = true;

#if AENIGMA_INTERNAL
    LPVOID base_adress = (LPVOID)TERABYTES(2);
#else
    LPVOID base_adress = 0;
#endif

    GameMemory game_memory = { 0 };
    game_memory.permanent_storage_size = MEGABYTES(512);
    game_memory.transient_storage_size = GIGABYTES(2);

    state.total_size = game_memory.permanent_storage_size + game_memory.transient_storage_size;

    state.game_memory_block = VirtualAlloc(base_adress, state.total_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    game_memory.permanent_storage = state.game_memory_block;

    if (!game_memory.permanent_storage) {
        // @Incomplete:  Logging
        return -1;
    }

    game_memory.transient_storage = ((u8 *)game_memory.permanent_storage + game_memory.permanent_storage_size);

    if (!game_memory.transient_storage) {
        // @Incomplete:  Logging
        return -1;
    }

    game_memory.platform.FindFilesInDirectory = platform_FindFilesInDirectory;
    game_memory.platform.DeleteFile = platform_DeleteFile;
    game_memory.platform.ToggleCursor = Win32PlatformToggleCursor;

    game_memory.platform.ReadEntireFile = platform_ReadEntireFile;
    game_memory.platform.FreeMemory = platform_FreeMemory;
    game_memory.platform.WriteEntireFile = platform_WriteEntireFile;

    game_memory.platform.ResizeRenderBuffer = platform_ResizeRenderBuffer;
    game_memory.platform.AllocateMemory = platform_AllocateMemory;

    game_memory.platform.SetCursorType = platform_SetCursorType;

#if AENIGMA_INTERNAL
    for (s32 replay_index = 0; replay_index < ArrayCount(state.replay_buffers); ++replay_index) {
        Win32ReplayBuffer *replay_buffer = &state.replay_buffers[replay_index];

        Win32GetInputFileLocation(&state, replay_index, false, replay_buffer->file_name, sizeof(replay_buffer->file_name));

        replay_buffer->file_handle = CreateFileA(replay_buffer->file_name, GENERIC_WRITE | GENERIC_READ, 0, 0, CREATE_ALWAYS, 0, 0);

        replay_buffer->memory_map = CreateFileMapping(replay_buffer->file_handle, 0, PAGE_READWRITE, state.total_size >> 32, state.total_size & 0xffffffff, 0);

        replay_buffer->memory_block = MapViewOfFile(replay_buffer->memory_map, FILE_MAP_ALL_ACCESS, 0, 0, state.total_size);
        // VirtualAlloc(0, state.total_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        if (replay_buffer->memory_block) {
            // @Incomplete:  Logging
        }
    }
#endif
    Input input[2] = { 0 };
    Input *new_input = &input[0];
    Input *old_input = &input[1];

    LARGE_INTEGER last_counter = Win32GetWallClock();

#if AENIGMA_INTERNAL
    Win32GameCode game = Win32LoadGameCode(source_game_code_dll_full_path, temp_game_code_dll_full_path);
#endif

    u64 last_cycle_count = __rdtsc();
    while (g_running) {
#if AENIGMA_FADE 
        if (Win32UpdateFade(&fader, new_input->dt, window) == WIN32_FADE_WAITING_FOR_CLOSE) {
            g_running = false;
        }
#endif

#if AENIGMA_INTERNAL
        FILETIME new_dll_write_time = Win32GetLastWriteTime(source_game_code_dll_full_path);
        if (CompareFileTime(&new_dll_write_time, &game.dll_last_write_time)) {
            Win32UnloadGameCode(&game);
            game = Win32LoadGameCode(source_game_code_dll_full_path, temp_game_code_dll_full_path);
        }

        DWORD test = WaitForSingleObject(assets_folder, 0);
        if (test == WAIT_OBJECT_0) {
            FindNextChangeNotification(assets_folder);
            game_memory.assets_folder_changed = true;
        }

#if 0
        DWORD test2 = WaitForSingleObject(src_folder, 0);
        if (test2 == WAIT_OBJECT_0) {
            FindNextChangeNotification(src_folder);
            STARTUPINFOA test = { 0 };
            PROCESS_INFORMATION info = { 0 };
            CreateProcessA("..\\src\\build.bat", 0, 0, 0, FALSE, NORMAL_PRIORITY_CLASS, 0, 0, &test, &info);
        }
#endif
#endif

        for (u32 button_index = 0; button_index < ArrayCount(new_input->key_states); ++button_index) {
            new_input->key_states[button_index].is_down = old_input->key_states[button_index].is_down;
            new_input->key_states[button_index].was_down = old_input->key_states[button_index].is_down;
        }

        new_input->mouse_wheel = 0;
        new_input->last_char = 0;
        g_input = new_input;

        Win32ProcessPendingMessages(&state, new_input);

        if (!g_pause) {
            POINT mouse_pos;
            GetCursorPos(&mouse_pos);
            ScreenToClient(window, &mouse_pos);

            // Client to game
            Win32WindowDimension window_dim = Win32GetWindowDimension(window);
            f32 real_mouse_pos_x = (f32)mouse_pos.x / window_dim.width;
            f32 real_mouse_pos_y = (f32)mouse_pos.y / window_dim.height;

            new_input->mouse_x = real_mouse_pos_x;
            new_input->mouse_y = real_mouse_pos_y;
#if 0
            win32_process_keyboard_message(&new_keyboard->mouse_buttons[0], GetKeyState(VK_LBUTTON) & (1 << 15));
            win32_process_keyboard_message(&new_keyboard->mouse_buttons[1], GetKeyState(VK_MBUTTON) & (1 << 15));
            win32_process_keyboard_message(&new_keyboard->mouse_buttons[2], GetKeyState(VK_RBUTTON) & (1 << 15));
            win32_process_keyboard_message(&new_keyboard->mouse_buttons[3], GetKeyState(VK_XBUTTON1) & (1 << 15));
            win32_process_keyboard_message(&new_keyboard->mouse_buttons[4], GetKeyState(VK_XBUTTON2) & (1 << 15));
#endif
            ThreadContext thread = { 0 };
            RenderBuffer buffer = { 0 };
            buffer.memory = g_backbuffer.memory;
            buffer.width = g_backbuffer.width;
            buffer.height = g_backbuffer.height;
            buffer.pitch = g_backbuffer.pitch;
            buffer.bytes_per_pixel = g_backbuffer.bytes_per_pixel;

#if !AENIGMA_RELEASE
            if (state.input_recording_index) {
                Win32RecordInput(&state, new_input);
            }
            if (state.input_playing_index) {
                Win32PlayBackInput(&state, new_input);
            }
#endif

#if AENIGMA_RELEASE
            GameUpdateAndRender(&thread, &game_memory, new_input, &buffer);
            if (game_memory.exit_requested) {
#if AENIGMA_FADE
                Win32BeginFadeToDesktop(&fader);
#else
                g_running = false;
#endif
            }
#else

            if (game.update_and_render) {
                game.update_and_render(&thread, &game_memory, new_input, &buffer);
                if (game_memory.exit_requested) {
#if AENIGMA_FADE
                    Win32BeginFadeToDesktop(&fader);
#else
                    g_running = false;
#endif

                }
            }
#endif
            LARGE_INTEGER work_counter = Win32GetWallClock();
            f32 seconds_elapsed_for_work = Win32GetSecondsElapsed(last_counter, work_counter);

#if AENIGMA_INTERNAL
            char text_buffer[256];
            sprintf_s(text_buffer, 256, "Work fps: %d ", (s32)(1.0f / seconds_elapsed_for_work + 0.5f));
            OutputDebugStringA(text_buffer);
#endif

#if 1
            // @Incomplete: REAL vsync (opengl or direct3d)
            f32 seconds_elapsed_for_frame = seconds_elapsed_for_work;
            if (seconds_elapsed_for_frame < target_seconds_per_frame) {
                if (sleep_is_granular) {
                    DWORD sleep_ms = (DWORD)(1000.0f * (target_seconds_per_frame - seconds_elapsed_for_frame) - 1);
                    if (sleep_ms > 0) {
                        Sleep(sleep_ms);
                    }
                }
                while (seconds_elapsed_for_frame < target_seconds_per_frame) {
                    seconds_elapsed_for_frame = Win32GetSecondsElapsed(last_counter, Win32GetWallClock());
                }
            } else {
                // @Incomplete: MISSED FRAME RATE!
                // @Incomplete: Logging
            }
#endif
            LARGE_INTEGER end_counter = Win32GetWallClock();
            f32 ms_per_frame = 1000.0f * Win32GetSecondsElapsed(last_counter, end_counter);
            last_counter = end_counter;

            Win32WindowDimension dimension = Win32GetWindowDimension(window);
            Win32DisplayBufferInWindow(&g_backbuffer, device_context, dimension.width, dimension.height);

            Input *temp = new_input;
            new_input = old_input;
            old_input = temp;

            new_input->dt_raw = seconds_elapsed_for_work;
            new_input->dt = ms_per_frame / 1000.0f;

            u64 end_cycle_count = __rdtsc();

#if AENIGMA_INTERNAL
            sprintf_s(text_buffer, 256, "fps: %d\n", (s32)(1.0f / (ms_per_frame / 1000.0f) + 0.5f));
            OutputDebugStringA(text_buffer);
#endif

            last_cycle_count = end_cycle_count;
        }
    }

    return 0;
}