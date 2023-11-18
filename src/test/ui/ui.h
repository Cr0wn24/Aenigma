#ifndef UI_H
#define UI_H

// NOTE(hampus): General
typedef u32 UI_WidgetFlags;
enum {
    UI_WidgetFlag_Clickable = (1 << 0),
    UI_WidgetFlag_ViewScroll = (1 << 1),
    UI_WidgetFlag_DrawText = (1 << 2),
    UI_WidgetFlag_DrawBorder = (1 << 3),
    UI_WidgetFlag_DrawBackground = (1 << 4),
    UI_WidgetFlag_DrawDropShadow = (1 << 5),
    UI_WidgetFlag_Clip = (1 << 6),
    UI_WidgetFlag_HotAnimation = (1 << 7),
    UI_WidgetFlag_ActiveAnimation = (1 << 8),
    UI_WidgetFlag_NoHide = (1 << 9),
};

typedef u32 UI_SizeKind;
enum {
    UI_SizeKind_Null,
    UI_SizeKind_Pixels,
    UI_SizeKind_TextContents,
    UI_SizeKind_PercentOfParent,
    UI_SizeKind_ChildrenSum,
};

typedef u32 Axis2;
enum {
    Axis2_X,
    Axis2_Y,
    Axis2_COUNT,
};

typedef struct UI_Size UI_Size;
struct UI_Size {
    UI_SizeKind kind;
    f32 value;
    f32 strictness;
};

// NOTE(hampus): Keying
typedef struct UI_Key UI_Key;
struct UI_Key {
    u32 key;
};

typedef struct UI_KeyStack UI_KeyStack;
struct UI_KeyStack {
    char keys[64][128];
    s32 current_pos;
};

// NOTE(hampus): Window
typedef struct UI_Window UI_Window;
struct UI_Window {
    Rect rect; // The rect of the whole window
    Rect content_rect; // the rect where the content is drawn

    v2 cursor_pos;

    s32 z;

    UI_Key key; // identifier for the window

    UI_Window *parent; // the prev window

    b32 hide_content;

    b32 dragging; // if the user is currently dragging the winndow

    char title[256];
};

typedef struct UI_WindowTable UI_WindowTable;
struct UI_WindowTable {
    UI_Window **windows;
    s32 length;
};

typedef struct UI_WindowGroup UI_WindowGroup;
struct UI_WindowGroup {
    UI_Window **windows;
    s32 count;
    s32 max_count;
};

typedef struct UI_Layout UI_Layout;
struct UI_Layout {
    b32 next_width_set;
    UI_Size next_width;

    b32 next_height_set;
    UI_Size next_height;

    b32 next_pos_set;
    v2 next_pos;

    b32 next_background_set;
    v4 next_background;

    b32 skip_pos;
};

typedef struct UI_LayoutStack UI_LayoutStack;
struct UI_LayoutStack {
    UI_Layout layouts[256];
    s32 count;
    s32 max_count;
};

// NOTE(hampus): Widgets
typedef struct UI_Widget UI_Widget;
struct UI_Widget {
    UI_Widget *first; // first child
    UI_Widget *last; // last child
    UI_Widget *next; // next sibling
    UI_Widget *prev; // previous sibling
    UI_Widget *parent; // parent

    UI_Window *window;

    UI_WidgetFlags flags;
    char text[256];

    UI_Size semantic_size[Axis2_COUNT];

    f32 computed_rel_position[Axis2_COUNT]; // rel position to window
    f32 computed_size[Axis2_COUNT]; // size in pixels
    Rect rect; // final rect which is used for input and rendering

    UI_Widget *hash_next;
    UI_Widget *hash_prev;

    v4 background;

    UI_Key key;
};

typedef struct UI_Comm UI_Comm;
struct UI_Comm {
    UI_Widget *widget;
    v2 mouse;
    v2 drag_delta;
    b32 clicked;
    b32 double_clicked;
    b32 right_clicked;
    b32 pressed;
    b32 released;
    b32 dragging;
    b32 hovering;
};

typedef struct UI_WidgetTable UI_WidgetTable;
struct UI_WidgetTable {
    UI_Widget **widgets;
    s32 length;
};

typedef struct UI_WidgetGroup UI_WidgetGroup;
struct UI_WidgetGroup {
    UI_Widget **widgets;
    s32 count;
    s32 max_count;
};

typedef struct UI UI;
struct UI {
    UI_WindowTable window_table;
    UI_WidgetTable widget_table;

    UI_Window *window; // the current window

    UI_WidgetGroup active_widgets;
    UI_WindowGroup active_windows;

    UI_LayoutStack layout_stack;
    UI_Layout *layout;

    UI_KeyStack key_stack;

    UI_Key hot_key; // the current hovered widget
    UI_Key active_key; // the current active widget

    UI_Key prev_hot_key;
    UI_Key prev_active_key;

    UI_Key active_window; // the current hovered window
};

// NOTE(hampus): General
function void UI_Begin();

function void UI_End();

// NOTE(hampus): Window
function UI_Window *UI_BeginWindow(char *string, s32 x0, s32 y0, s32 x1, s32 y1);

function UI_Window *UI_EndWindow();

// NOTE(hampus): Widgets
function UI_Widget *UI_WidgetMake(char *string, UI_WidgetFlags flags);

// NOTE(hampus): Layouting
function UI_Size UI_Pixels(f32 value);

function void UI_NextWidth(UI_Size size);

function void UI_NextHeight(UI_Size size);


#endif