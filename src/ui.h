#ifndef UI_H
#define UI_H

typedef struct UI_Layout
{
    u32 num_buttons;
    RenderBuffer *buffer;
    Input *input;
} UI_Layout;

typedef struct Rect
{
    v2 min;
    v2 max;
    v4 color;
} Rect;

typedef enum Alignment_Flags
{
    AlignFlag_None = (1 << 0),
    AlignFlag_CenterHeight = (1 << 1),
} Alignment_Flags;

#endif