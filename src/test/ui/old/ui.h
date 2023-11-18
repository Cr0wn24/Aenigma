#ifndef UI_H
#define UI_H

typedef enum UI_SizeKind {
    UI_SizeKind_Null,
    UI_SizeKind_Pixels,
    UI_SizeKind_TextContents,
    UI_SizeKind_PercentOfParent,
    UI_SizeKind_ChildrenSum,
} UI_SizeKind;

typedef enum Axis2 {
    Axis2_X,
    Axis2_Y,
    Axis2_COUNT,
} Axis2;

typedef u32 UI_BoxFlags;
enum {
    UI_BoxFlag_Clickable = (1 << 0),
    UI_BoxFlag_ViewScroll = (1 << 1),
    UI_BoxFlag_DrawText = (1 << 2),
    UI_BoxFlag_DrawBorder = (1 << 3),
    UI_BoxFlag_DrawBackground = (1 << 4),
    UI_BoxFlag_DrawDropShadow = (1 << 5),
    UI_BoxFlag_NoClip = (1 << 6),
    UI_BoxFlag_HotAnimation = (1 << 7),
    UI_BoxFlag_ActiveAnimation = (1 << 8),
    UI_BoxFlag_NoScroll = (1 << 9),
};

typedef u32 UI_AlignmentFlag;
enum {
    UI_AlignmentFlag_Center = (1 << 0),
    UI_AlignmentFlag_Right = (1 << 1),
    UI_AlignmentFlag_Left = (1 << 2),
};

typedef struct UI_Size {
    UI_SizeKind kind;
    f32 value;
    f32 strictness;
} UI_Size;

typedef struct UI_Key UI_Key;
struct UI_Key {
    u32 key;
};

typedef struct UI_Window UI_Window;

typedef struct UI_Box UI_Box;
struct UI_Box {
    UI_Box *first; // first child
    UI_Box *last; // last child
    UI_Box *next; // next sibling
    UI_Box *prev; // previous sibling
    UI_Box *parent; // parent

    b32 used;

    UI_BoxFlags flags;
    char text[256];
    UI_Size semantic_size[Axis2_COUNT];

    f32 computed_rel_position[Axis2_COUNT]; // rel position to parent
    f32 computed_size[Axis2_COUNT]; // size in pixels
    Rect pos_rect; // rect used by children to calculate position
    Rect rect; // final rect which is used for input and rendering

    // additional styling options
    b32 color_set;
    v4 color;
    UI_AlignmentFlag text_alignment;

    UI_Box *hash_next;
    UI_Box *hash_prev;

    UI_Key key;
    u64 last_frame_touched_index;

    f32 hot_t;
    f32 active_t;

    b32 skip_pos;

    Rect clip_rect;

    b32 selected;

    // NOTE(hampus): For the window
    // TODO(hampus): Remove these and place them in the window.
    f32 max_pos_x;
    f32 max_pos_y;
    f32 scroll_x;
    f32 scroll_y;
    UI_Box *first_box;

    UI_Window *window;
};

typedef struct UI_Comm {
    UI_Box *box;
    v2 mouse;
    v2 drag_delta;
    b32 clicked;
    b32 double_clicked;
    b32 right_clicked;
    b32 pressed;
    b32 released;
    b32 dragging;
    b32 hovering;
    s32 scrolling;
} UI_Comm;

typedef struct UI_Layout_State {
    b32 columns;
    b32 rows;
    b32 same_line;
    b32 skip_next_pos;

    f32 indent;

    f32 next_width;
    f32 next_height;

    b32 next_relative_pos_set;
    f32 next_relative_pos;

    b32 next_pos_set;
    v2 next_pos;

    b32 next_background_color_set;
    v4 next_background_color;

    b32 no_padding;

    b32 skip_scroll;

    UI_AlignmentFlag next_align;
    UI_AlignmentFlag next_text_align;

    b32 clip_rect_set;
    Rect clip_rect;
} UI_Layout_State;

typedef struct UI_Window {
    UI_Box *box;
    char key_stack[32][256];
    u32 curr_key_stack_index;
    char full_key_stack[1024];
    char title[256];
    b32 need_scroll_y;
    b32 need_scroll_x;
    Rect clip_rect;
} UI_Window;

typedef struct UI {
    UI_Box *parents[64];
    u32 parent_index;
    UI_Box *curr_parent;

    UI_Layout_State layout_states[4096];
    u32 layout_state_index;
    UI_Layout_State *curr_layout_state;

    UI_Window *windows[128];
    u32 window_index;
    UI_Window *window;

    UI_Box *prev_box;
    UI_Box *curr_box;

    // NOTE(hampus): For rendering
    UI_Box **boxes;
    UI_Box **boxes_to_render;
    u32 num_boxes_to_render;

    // NOTE(hampus): To keep track of which widgets are active/hot
    UI_Key hot_key;
    UI_Key active_key;

    // NOTE(hampus): Needed when dragging a slider outside
    UI_Key prev_hot_key;
    UI_Key prev_active_key;

    // other
    MemoryArena permanent_arena;
    MemoryArena frame_arena;
    u64 curr_frame_index;

    v2 prev_mouse_pos;

} UI;

#endif