#ifndef ASSET_H
#define ASSET_H

typedef struct Bitmap {
    s32 width;
    s32 height;
    u32 *pixels;
} Bitmap;

#pragma pack(push, 1)
typedef struct TimBitmaps {
    // first [0][n] is not pushing, second [1][n] is pushing
    Bitmap still_up;
    Bitmap still_down;
    Bitmap still_left;
    Bitmap still_right;

    Bitmap run_up[4];
    Bitmap run_down[4];
    Bitmap run_left[4];
    Bitmap run_right[4];

    Bitmap pushing_up[4];
    Bitmap pushing_down[4];
    Bitmap pushing_left[4];
    Bitmap pushing_right[4];
} TimBitmaps;

typedef struct GameAssets {
    Bitmap water[2];

    Bitmap grass[2];
    Bitmap grass_top[2];
    Bitmap wall;
    Bitmap mossy_wall;
    Bitmap moveable_wall;
    Bitmap door_portal_horizontal;
    Bitmap door_portal_vertical;
    Bitmap tree_port_pier;
    Bitmap tree_port_pier_pole;
    Bitmap shadows[4];
    Bitmap door;
    TimBitmaps tim_bitmaps;

    Bitmap demo_map;
    Bitmap scroll_on_ground;

    Bitmap missing_bitmap;
    Bitmap wsad_guide;

    Bitmap pen;
    Bitmap bin;
} GameAssets;

typedef struct BitmapHeader {
    u16 file_type;
    u32 file_size;
    u16 reserved0;
    u16 reserved1;
    u32 bitmap_offset;
    u32 size;
    s32 width;
    s32 height;
    u16 planes;
    u16 bits_per_pixel;
    u32 compression;
    u32 size_of_bitmap;
    s32 horizontal_res;
    s32 vertical_res;
    u32 colors_used;
    u32 colors_important;

    u32 red_mask;
    u32 green_mask;
    u32 blue_mask;
} BitmapHeader;
#pragma pack(pop)

#endif