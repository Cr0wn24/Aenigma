#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <dlfcn.h>

#include <time.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#include "platform.h"

#include "linux_main.h"

#include "linux_platform.c"

global b32 g_running = false;
global Display *g_display;
global XVisualInfo *g_info;
global GC *g_gc;
global Window *g_window;

function void
LinuxResizeRenderBuffer(Linux_RenderBuffer *buffer, s32 width, s32 height) {

    if (buffer->ximage) {
        XDestroyImage(buffer->ximage);
    }

    s32 bits_per_pixel = 32;
    s32 bytes_per_pixel = bits_per_pixel / 8;
    s32 size = width * height * bytes_per_pixel;
    // void *memory = mmap(0, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    void *memory = malloc(size);

    buffer->memory = memory;
    buffer->width = width;
    buffer->height = height;
    buffer->bytes_per_pixel = bytes_per_pixel;
    buffer->pitch = buffer->bytes_per_pixel * buffer->width;

    buffer->ximage = XCreateImage(g_display, g_info->visual, g_info->depth,
                                  ZPixmap, 0, memory, width, height,
                                  bits_per_pixel, 0);

}

function void
LinuxDisplayBufferInWindow(Linux_RenderBuffer *buffer, s32 window_width, s32 window_height) {
    XPutImage(g_display, *g_window,
              *g_gc, buffer->ximage, 0, 0, 0, 0,
              window_width, window_height);
}

function void
LinuxLockWindowSize(Display *display, Window window, s32 width, s32 height) {
    XSizeHints hints = {};
    hints.flags |= PMinSize;
    hints.flags |= PMaxSize;

    hints.min_width = width;
    hints.min_height = height;
    hints.max_width = width;
    hints.max_height = height;

    XSetWMNormalHints(display, window, &hints);
}

function Status
LinuxToggleFullscreen(Display *display, Window window) {
    XClientMessageEvent event = {};
    Atom wm_state = XInternAtom(display, "_NET_WM_STATE", False);
    Atom max_horizontal = XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
    Atom max_vertical = XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_VERT", False);

    if (wm_state == None) return 0;

    event.type = ClientMessage;
    event.format = 32;
    event.window = window;
    event.message_type = wm_state;
    event.data.l[0] = 2;
    event.data.l[1] = max_horizontal;
    event.data.l[2] = max_vertical;
    event.data.l[3] = 1;

    return XSendEvent(display, DefaultRootWindow(display), False,
                      SubstructureNotifyMask,
                      (XEvent *)&event);
}

function void
LinuxCopyFile(char *src, char *dest) {
}

function LinuxGameCode
LinuxLoadGameCode(const char *source_dll_name, const char *temp_dll_name) {
    LinuxGameCode result = { 0 };
    // result.DLLLastWriteTime = Linux32GetLastWriteTime(source_dll_filename);

    // LinuxCopyFile(source_dll_filename, temp_dll_name);
    result.game_code_dll = dlopen(source_dll_name, RTLD_NOW);
    char *error = dlerror();
    if (result.game_code_dll) {
        result.update_and_render = dlsym(result.game_code_dll, "game_update_and_render");

        result.valid = (result.update_and_render != 0);
    } else {
        // TODO: Logging
    }

    if (!result.valid) {
        result.update_and_render = 0;
    }

    return result;
}

function void
LinuxUnloadGameCode(LinuxGameCode *game_code) {
    if (game_code->game_code_dll) {
        dlclose(game_code->game_code_dll);
        game_code->game_code_dll = 0;
    }

    game_code->valid = false;

    game_code->update_and_render = 0;
}

s32
main(s32 argc, char *argv[]) {
    LinuxState linux_state = { 0 };

    Display *display = XOpenDisplay(0);
    g_display = display;

    if (!display) {
        printf("No display available\n");
        exit(1);
    }

    s32 root = DefaultRootWindow(display);
    s32 default_screen = DefaultScreen(display);
    s32 screen_bit_depth = 24;
    XVisualInfo info = { 0 };
    g_info = &info;

    if (!XMatchVisualInfo(display, default_screen, screen_bit_depth, TrueColor, &info)) {
        printf("No matching visual info found\n");
        exit(1);
    }

    XSetWindowAttributes window_attr;
    window_attr.bit_gravity = StaticGravity;
    window_attr.background_pixel = 0;
    window_attr.colormap = XCreateColormap(display, root, info.visual, AllocNone);
    window_attr.event_mask = StructureNotifyMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask;

    u32 attribute_mask = CWBackPixel | CWColormap | CWEventMask | CWBitGravity;
    Window window = XCreateWindow(display, root, 0, 0, 960, 540, 0, info.depth, InputOutput, info.visual, attribute_mask, &window_attr);
    g_window = &window;

    if (!window) {
        printf("Failed to create windown");
        exit(1);
    }

    XStoreName(display, window, "Aenigma");

    s32 window_width = 960;
    s32 window_height = 540;
    //LinuxLockWindowSize(display, window, window_width, window_height);

    XMapWindow(display, window);

    XFlush(display);

    Linux_RenderBuffer linux_buffer = { 0 };
    LinuxResizeRenderBuffer(&linux_buffer, window_width, window_height);

    Linux_RenderBuffer window_buffer = { 0 };
    LinuxResizeRenderBuffer(&window_buffer, window_width, window_height);

    GC gc = DefaultGC(display, default_screen);
    g_gc = &gc;

    Atom WM_DELETE_WINDOW = XInternAtom(display, "WM_DELETE_WINDOW", False);
    if (!XSetWMProtocols(display, window, &WM_DELETE_WINDOW, 1)) {
        printf("Couldn't register WM_DELETE_WINDOW property\n");
    }

    GameMemory memory = { 0 };
    memory.permanent_storage_size = MEGABYTES(1024);
    memory.transient_storage_size = GIGABYTES(1);

    linux_state.total_size = memory.permanent_storage_size + memory.transient_storage_size;
    linux_state.game_memory_block = mmap(0, linux_state.total_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

    memory.permanent_storage = linux_state.game_memory_block;
    memory.transient_storage = ((u8 *)memory.permanent_storage + memory.transient_storage_size);

    memory.platform.ReadEntireFile = platform_ReadEntireFile;
    memory.platform.AllocateMemory = platform_AllocateMemory;
    memory.platform.FreeMemory = platform_FreeMemory;
    memory.platform.ToggleCursor = PlatformToggleCursor;
    memory.platform.FindFilesInDirectory = platform_FindFilesInDirectory;

    LinuxGameCode game_code = { 0 };

    game_code = LinuxLoadGameCode("../build/debug/libgame.so", "./aibtempgame.so");

    f32 target_seconds_per_frame = 1.0f / 155.0f;

    b32 changed_size = false;

    Input input[2] = { 0 };
    Input *new_input = &input[0];
    Input *old_input = &input[1];

    g_running = true;
    while (g_running) {
        struct timespec start_counter = { 0 };
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_counter);

        for (u32 button_index = 0; button_index < ArrayCount(new_input->key_states); ++button_index) {
            new_input->key_states[button_index].is_down = old_input->key_states[button_index].is_down;
            new_input->key_states[button_index].was_down = old_input->key_states[button_index].is_down;
        }

        new_input->mouse_wheel = 0;
        new_input->last_char = 0;

        XEvent event = { 0 };
        while (XPending(display) > 0) {
            XNextEvent(display, &event);
            switch (event.type) {
                case DestroyNotify: \n
            }
            XDestroyWindowEvent *e = (XDestroyWindowEvent *)&event;
            if (e->window == window) {
                g_running = false;
            }
        } break;

        case ClientMessage: {
            XClientMessageEvent *e = (XClientMessageEvent *)&event;
            if ((Atom)e->data.l[0] == WM_DELETE_WINDOW) {
                XDestroyWindow(display, window);
                g_running = false;
            }
        } break;

        case ConfigureNotify: {
            XConfigureEvent *e = (XConfigureEvent *)&event;
            window_width = e->width;
            window_height = e->height;
            changed_size = true;
        } break;

        case ButtonRelease:
        case ButtonPress: {
            XButtonPressedEvent *e = (XButtonPressedEvent *)&event;
            b32 is_down = event.type == ButtonPress;
            switch (e->button) {
                case Button1: {
                } break;
                    new_input->key_states[KeyCode_MouseLeft].is_down = is_down;
            } break;
        case Button2: {
            new_input->key_states[KeyCode_MouseMiddle].is_down = is_down;
        } break;
        case Button3: {
            new_input->key_states[KeyCode_MouseRight].is_down = is_down;
        } break;
        case Button4: {

        } break;
        case Button5: {

        } break;
        }


        case KeyRelease:
        case KeyPress: {
            XKeyPressedEvent *e = (XKeyPressedEvent *)&event;
            b32 is_down = event.type == KeyPress;

#define ProcessKeyMessage(vk, key) \
else if (e->keycode == XKeysymToKeycode(display, vk)) { \
new_input->key_states[key].is_down = is_down; \
}

            // dummy
            if (false) {
            }
            ProcessKeyMessage(XK_W, KeyCode_W)
                ProcessKeyMessage(XK_S, KeyCode_S)
                ProcessKeyMessage(XK_D, KeyCode_D)
                ProcessKeyMessage(XK_A, KeyCode_A)

                ProcessKeyMessage(XK_Q, KeyCode_Q)
                ProcessKeyMessage(XK_E, KeyCode_E)
                ProcessKeyMessage(XK_R, KeyCode_R)
                ProcessKeyMessage(XK_O, KeyCode_O)
                ProcessKeyMessage(XK_N, KeyCode_N)
                ProcessKeyMessage(XK_P, KeyCode_P)
                ProcessKeyMessage(XK_L, KeyCode_L)

                ProcessKeyMessage(XK_Escape, KeyCode_Escape)
                ProcessKeyMessage(XK_Return, KeyCode_Enter)
                ProcessKeyMessage(XK_BackSpace, KeyCode_Backspace)
                ProcessKeyMessage(XK_space, KeyCode_Space)

                ProcessKeyMessage(XK_Up, KeyCode_Up)
                ProcessKeyMessage(XK_Down, KeyCode_Down)
                ProcessKeyMessage(XK_Left, KeyCode_Left)
                ProcessKeyMessage(XK_Right, KeyCode_Right)

                ProcessKeyMessage(XK_0, KeyCode_0)
                ProcessKeyMessage(XK_1, KeyCode_1)
                ProcessKeyMessage(XK_2, KeyCode_2)
                ProcessKeyMessage(XK_3, KeyCode_3)
                ProcessKeyMessage(XK_4, KeyCode_4)
                ProcessKeyMessage(XK_5, KeyCode_5)
                ProcessKeyMessage(XK_6, KeyCode_6)
                ProcessKeyMessage(XK_7, KeyCode_7)
                ProcessKeyMessage(XK_8, KeyCode_8)
                ProcessKeyMessage(XK_9, KeyCode_9)

                ProcessKeyMessage(XK_F1, KeyCode_F1)
                ProcessKeyMessage(XK_F1, KeyCode_F2)
                ProcessKeyMessage(XK_F2, KeyCode_F2)
                ProcessKeyMessage(XK_F3, KeyCode_F3)
                ProcessKeyMessage(XK_F4, KeyCode_F4)
                ProcessKeyMessage(XK_F5, KeyCode_F5)
                ProcessKeyMessage(XK_F6, KeyCode_F6)
                ProcessKeyMessage(XK_F7, KeyCode_F7)
                ProcessKeyMessage(XK_F8, KeyCode_F8)
                ProcessKeyMessage(XK_F9, KeyCode_F9)
                ProcessKeyMessage(XK_F10, KeyCode_F10)
                ProcessKeyMessage(XK_F11, KeyCode_F11)
                ProcessKeyMessage(XK_F12, KeyCode_F12)
        } break;

        default: {

        } break;
    }

    if (changed_size) {
        LinuxResizeRenderBuffer(&window_buffer, window_width, window_height);
    }

    s32 mouse_root_x = 0;
    s32 mouse_root_y = 0;
    s32 mouse_win_x = 0;
    s32 mouse_win_y = 0;
    Window windows_returned[2];
    u32 mask_returned;
    XQueryPointer(display, window,
                  &windows_returned[0], &windows_returned[1],
                  &mouse_root_x, &mouse_root_y,
                  &mouse_win_x, &mouse_win_y,
                  &mask_returned);

    new_input->mouse_x = (f32)mouse_win_x;
    new_input->mouse_y = (f32)mouse_win_y;

    RenderBuffer render_buffer = { 0 };
    render_buffer.height = linux_buffer.height;
    render_buffer.width = linux_buffer.width;
    render_buffer.bytes_per_pixel = linux_buffer.bytes_per_pixel;
    render_buffer.pitch = linux_buffer.pitch;
    render_buffer.memory = linux_buffer.memory;

    new_input->dt = target_seconds_per_frame;
    if (game_code.update_and_render) {
        game_code.update_and_render(0, &memory, new_input, &render_buffer);
    }
#if 0
    u32 *dest = (u32 *)window_buffer.memory;
    u32 *src = (u32 *)linux_buffer.memory;

    f32 width_scale = (f32)linux_buffer.width / (f32)window_buffer.width;
    f32 height_scale = (f32)linux_buffer.height / (f32)window_buffer.height;

    for (u32 y = 0; y < window_height; ++y) {
        u32 *dest_pixel = dest;
        u32 *src_pixel = src;
        u32 src_y = (u32)(y * height_scale);
        for (u32 x = 0; x < window_width; ++x) {
            u32 src_x = (u32)(x * width_scale);
            *dest_pixel++ = *(src_pixel + src_x + src_y * linux_buffer.width);
        }
        dest += window_buffer.width;
    }
    LinuxDisplayBufferInWindow(&window_buffer, window_buffer.width, window_buffer.height);
#else
    LinuxDisplayBufferInWindow(&linux_buffer, linux_buffer.width, linux_buffer.height);
#endif
    struct timespec end_counter = { 0 };
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_counter);

    f32 nanoseconds_elapsed = 0;

    if ((end_counter.tv_nsec - start_counter.tv_nsec) < 0) {
        nanoseconds_elapsed = (f32)(1000000000 + end_counter.tv_nsec - start_counter.tv_nsec);
    } else {
        nanoseconds_elapsed = (f32)(end_counter.tv_nsec - start_counter.tv_nsec);
    }

    f32 ms_elapsed = nanoseconds_elapsed / (f32)10e6;
    printf("ms elapsed: %.02f\n", ms_elapsed);

    Swap(new_input, old_input, Input *);

    new_input->dt = ms_elapsed / 1000.0f;
    new_input->dt_raw = ms_elapsed / 1000.0f;
}

return 0;
}