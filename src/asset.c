#if AENIGMA_RELEASE
#include "asset_builder_result.c"
#endif

#if 0
function inline u32 Unpack4x8(v4 a);
function inline v4 Pack4x8(u32 color);

function f32
BilinearInterpolation(f32 q11, f32 q12, f32 q21, f32 q22, f32 x1, f32 x2, f32 y1, f32 y2, f32 x, f32 y) {
    f32 x2x1, y2y1, x2x, y2y, yy1, xx1;
    x2x1 = x2 - x1;
    y2y1 = y2 - y1;
    x2x = x2 - x;
    y2y = y2 - y;
    yy1 = y - y1;
    xx1 = x - x1;
    return 1.0f / (x2x1 * y2y1) * (
        q11 * x2x * y2y +
        q21 * xx1 * y2y +
        q12 * x2x * yy1 +
        q22 * xx1 * yy1
        );
}

function v4
BilinearInterpolationV4(v4 q11, v4 q12, v4 q21, v4 q22, f32 x1, f32 x2, f32 y1, f32 y2, f32 x, f32 y) {
    v4 result;

    result.r = BilinearInterpolation(q11.r, q12.r, q21.r, q22.r, x1, x2, y1, y2, x, y);
    result.g = BilinearInterpolation(q11.g, q12.g, q21.g, q22.g, x1, x2, y1, y2, x, y);
    result.b = BilinearInterpolation(q11.b, q12.b, q21.b, q22.b, x1, x2, y1, y2, x, y);
    result.a = BilinearInterpolation(q11.a, q12.a, q21.a, q22.a, x1, x2, y1, y2, x, y);

    return result;
}
function Bitmap
ResizeBitmapBilinear(Bitmap *bitmap, s32 new_width, s32 new_height) {
    Bitmap new_bitmap ={ 0 };
    if (bitmap->pixels && bitmap->height && bitmap->width) {
        new_bitmap.pixels = platform->AllocateMemory(new_width * new_height * 4);
        new_bitmap.width = new_width;
        new_bitmap.height = new_height;

        f32 width_factor = (f32)bitmap->width / new_width;
        f32 height_factor = (f32)bitmap->height / new_height;

        f32 inv_width_factor = 1.0f / width_factor;
        f32 inv_height_factor = 1.0f / height_factor;

        for (s32 y = 0; y < new_height; ++y) {
            for (s32 x = 0; x < new_width; ++x) {
                s32 src_x = (s32)floorf(x * width_factor);
                s32 src_y = (s32)floorf(y * height_factor);

                s32 x0 = src_x;
                s32 y0 = src_y;

                x0 = Clamp(0, x0, bitmap->width - 1);
                y0 = Clamp(0, y0, bitmap->height - 1);

                s32 x1 = src_x + 1;
                s32 y1 = src_y;

                x1 = Clamp(0, x1, bitmap->width - 1);
                y1 = Clamp(0, y1, bitmap->height - 1);

                s32 x2 = src_x;
                s32 y2 = src_y + 1;

                x2 = Clamp(0, x2, bitmap->width - 1);
                y2 = Clamp(0, y2, bitmap->height - 1);

                s32 x3 = src_x + 1;
                s32 y3 = src_y + 1;

                x3 = Clamp(0, x3, bitmap->width - 1);
                y3 = Clamp(0, y3, bitmap->height - 1);

                u32 srgb0 = bitmap->pixels[x0 + y0 * bitmap->width];             // (0, 0)
                u32 srgb1 = bitmap->pixels[x1 + y1 * bitmap->width];            // (1, 0)
                u32 srgb2 = bitmap->pixels[x2 + y2 * bitmap->width];            // (0, 1)
                u32 srgb3 = bitmap->pixels[x3 + y3 * bitmap->width];      // (1, 1)

                f32 c0 = x / inv_width_factor;
                f32 c1 = y / inv_height_factor;

                f32 weight_x = (c0 - ((s32)c0));
                f32 weight_y = (c1 - ((s32)c1));

                v4 color0 = Pack4x8(srgb0);
                v4 color1 = Pack4x8(srgb1);
                v4 color2 = Pack4x8(srgb2);
                v4 color3 = Pack4x8(srgb3);

                v4 result_color3 = BilinearInterpolationV4(color0, color2, color1, color3, 0, 1.0f, 0, 1.0f, weight_x, weight_y);

                u32 colorsrgb = Unpack4x8(result_color3);

                new_bitmap.pixels[x + y * new_width] = colorsrgb;
            }
        }
    }
    return new_bitmap;
}
#endif

function void
ResizeBitmapNearest(Bitmap *bitmap, s32 new_width, s32 new_height) {
    if (bitmap->pixels && bitmap->height && bitmap->width) {
        Bitmap new_bitmap ={ 0 };

        new_bitmap.pixels = platform->AllocateMemory(new_width * new_height * 4);
        new_bitmap.width = new_width;
        new_bitmap.height = new_height;

        f32 width_factor = (f32)bitmap->width / new_width;
        f32 height_factor = (f32)bitmap->height / new_height;

        u32 *dest_row = new_bitmap.pixels;

        for (s32 y = 0; y < new_height; ++y) {
            s32 src_y = (s32)floorf(y * height_factor);
            u32 *dest_pixel = dest_row;
            for (s32 x = 0; x < new_width; ++x) {
                s32 src_x = (s32)floorf(x * width_factor);

                *dest_pixel++ = bitmap->pixels[src_x + src_y * bitmap->width];
            }
            dest_row += new_width;
        }

        platform->FreeMemory(0, bitmap->pixels);
        bitmap->pixels = new_bitmap.pixels;
        bitmap->width = new_bitmap.width;
        bitmap->height = new_bitmap.height;
    }
}

function void
ResizeAssets(GameAssets *assets, f32 factor) {
    u32 num_bitmaps = sizeof(*assets) / sizeof(Bitmap);
    Bitmap *bitmap = (Bitmap *)assets;
    for (u32 i = 0; i < num_bitmaps; ++i) {
        ResizeBitmapNearest(bitmap, RoundReal32ToInt32(bitmap->width * factor), RoundReal32ToInt32(bitmap->height * factor));
        ++bitmap;
    }

}

function Bitmap
DEBUGLoadBMP(ThreadContext *thread, PlatformReadEntireFile *ReadEntireFile, char *file_name) {
    Bitmap result ={ 0 };

    ReadFileResult read_file_result = ReadEntireFile(thread, file_name);
    if (read_file_result.content_size) {
        BitmapHeader *header = (BitmapHeader *)read_file_result.contents;
        result.pixels = (u32 *)((u8 *)read_file_result.contents + header->bitmap_offset);
        result.width = header->width;
        result.height = header->height;

        u32 red_mask = header->red_mask;
        u32 green_mask = header->green_mask;
        u32 blue_mask = header->blue_mask;
        u32 alpha_mask = ~(red_mask | green_mask | blue_mask);

        BitScanResult red_shift = FindLeastSignificantSetBit(red_mask);
        BitScanResult green_shift = FindLeastSignificantSetBit(green_mask);
        BitScanResult blue_shift = FindLeastSignificantSetBit(blue_mask);
        BitScanResult alpha_shift = FindLeastSignificantSetBit(alpha_mask);

        Assert(red_shift.found);
        Assert(green_shift.found);
        Assert(blue_shift.found);
        Assert(alpha_shift.found);

        u32 *source_dest = result.pixels;
        for (s32 y = 0; y < header->height; ++y) {
            for (s32 x = 0; x < header->width; ++x) {
                u32 color = *source_dest;
                *source_dest =
                    ((((color >> alpha_shift.index) & 0xff) << 24) |
                     (((color >> red_shift.index) & 0xff) << 16) |
                     (((color >> green_shift.index) & 0xff) << 8) |
                     (((color >> blue_shift.index) & 0xff) << 0));

                f32 a = (f32)((*source_dest >> 24) & 0xff) / 255.0f;

                f32 sr = (f32)((*source_dest >> 16) & 0xff);
                f32 sg = (f32)((*source_dest >> 8) & 0xff);
                f32 sb = (f32)((*source_dest >> 0) & 0xff);

#if 1

                *source_dest =
                    ((u32)(a * 255.0f) << 24) |
                    (((u32)(sr * a + 0.5f) << 16) |
                     ((u32)(sg * a + 0.5f) << 8) |
                     ((u32)(sb * a + 0.5f) << 0));
#endif

                source_dest++;
            }
        }
    }

    return result;
}

function void
DEBUGLoadGameAssets(ThreadContext *thread, GameAssets *assets, PlatformReadEntireFile *ReadEntireFile) {
    u32 num_bitmaps = sizeof(*assets) / sizeof(Bitmap);
    Bitmap *bitmap = (Bitmap *)assets;
    for (u32 i = 0; i < num_bitmaps; ++i) {
        if (bitmap->pixels) {
            platform->FreeMemory(0, bitmap->pixels);
        }
        bitmap->width = 0;
        bitmap->height = 0;
        ++bitmap;
    }

    assets->water[0] = DEBUGLoadBMP(thread, ReadEntireFile, "water.bmp");

    assets->grass[0] = DEBUGLoadBMP(thread, ReadEntireFile, "grass00.bmp");
    assets->grass[1] = DEBUGLoadBMP(thread, ReadEntireFile, "grass01.bmp");

    assets->grass_top[0] = DEBUGLoadBMP(thread, ReadEntireFile, "grass_top00.bmp");
    assets->grass_top[1] = DEBUGLoadBMP(thread, ReadEntireFile, "grass_top01.bmp");

    assets->wall = DEBUGLoadBMP(thread, ReadEntireFile, "wall.bmp");

    assets->mossy_wall = DEBUGLoadBMP(thread, ReadEntireFile, "mossy_wall.bmp");
    assets->moveable_wall = DEBUGLoadBMP(thread, ReadEntireFile, "moveable_wall.bmp");
    assets->door_portal_horizontal = DEBUGLoadBMP(thread, ReadEntireFile, "door_portal_horizontal.bmp");
    assets->door_portal_vertical = DEBUGLoadBMP(thread, ReadEntireFile, "door_portal_vertical.bmp");
    assets->tree_port_pier = DEBUGLoadBMP(thread, ReadEntireFile, "tree_port_pier.bmp");
    assets->tree_port_pier_pole = DEBUGLoadBMP(thread, ReadEntireFile, "tree_port_pier_pole.bmp");

    assets->shadows[ShadowType_Small] = DEBUGLoadBMP(thread, ReadEntireFile, "shadow_small.bmp");
    assets->shadows[ShadowType_Medium] = DEBUGLoadBMP(thread, ReadEntireFile, "shadow_medium.bmp");
    assets->shadows[ShadowType_Large] = DEBUGLoadBMP(thread, ReadEntireFile, "shadow_large.bmp");

    assets->shadows[ShadowType_Tim] = DEBUGLoadBMP(thread, ReadEntireFile, "shadow_tim.bmp");

    assets->tim_bitmaps.run_up[0] = DEBUGLoadBMP(thread, ReadEntireFile, "tim_up_run1.bmp");
    assets->tim_bitmaps.run_up[1] = DEBUGLoadBMP(thread, ReadEntireFile, "tim_up_run2.bmp");
    assets->tim_bitmaps.run_up[2] = DEBUGLoadBMP(thread, ReadEntireFile, "tim_up_run3.bmp");
    assets->tim_bitmaps.run_up[3] = DEBUGLoadBMP(thread, ReadEntireFile, "tim_up_run4.bmp");

    assets->tim_bitmaps.run_down[0] = DEBUGLoadBMP(thread, ReadEntireFile, "tim_down_run1.bmp");
    assets->tim_bitmaps.run_down[1] = DEBUGLoadBMP(thread, ReadEntireFile, "tim_down_run2.bmp");
    assets->tim_bitmaps.run_down[2] = DEBUGLoadBMP(thread, ReadEntireFile, "tim_down_run3.bmp");
    assets->tim_bitmaps.run_down[3] = DEBUGLoadBMP(thread, ReadEntireFile, "tim_down_run4.bmp");

    assets->tim_bitmaps.run_left[0] = DEBUGLoadBMP(thread, ReadEntireFile, "tim_left_run1.bmp");
    assets->tim_bitmaps.run_left[1] = DEBUGLoadBMP(thread, ReadEntireFile, "tim_left_run2.bmp");
    assets->tim_bitmaps.run_left[2] = DEBUGLoadBMP(thread, ReadEntireFile, "tim_left_run3.bmp");
    assets->tim_bitmaps.run_left[3] = DEBUGLoadBMP(thread, ReadEntireFile, "tim_left_run4.bmp");

    assets->tim_bitmaps.run_right[0] = DEBUGLoadBMP(thread, ReadEntireFile, "tim_right_run1.bmp");
    assets->tim_bitmaps.run_right[1] = DEBUGLoadBMP(thread, ReadEntireFile, "tim_right_run2.bmp");
    assets->tim_bitmaps.run_right[2] = DEBUGLoadBMP(thread, ReadEntireFile, "tim_right_run3.bmp");
    assets->tim_bitmaps.run_right[3] = DEBUGLoadBMP(thread, ReadEntireFile, "tim_right_run4.bmp");

    assets->tim_bitmaps.still_up = DEBUGLoadBMP(thread, ReadEntireFile, "tim_up_still.bmp");
    assets->tim_bitmaps.still_down = DEBUGLoadBMP(thread, ReadEntireFile, "tim_down_still.bmp");
    assets->tim_bitmaps.still_left = DEBUGLoadBMP(thread, ReadEntireFile, "tim_left_still.bmp");
    assets->tim_bitmaps.still_right = DEBUGLoadBMP(thread, ReadEntireFile, "tim_right_still.bmp");

    assets->tim_bitmaps.pushing_up[0] = DEBUGLoadBMP(thread, ReadEntireFile, "tim_pushing_up.bmp");
    assets->tim_bitmaps.pushing_up[1] = DEBUGLoadBMP(thread, ReadEntireFile, "tim_pushing_up.bmp");
    assets->tim_bitmaps.pushing_up[2] = DEBUGLoadBMP(thread, ReadEntireFile, "tim_pushing_up.bmp");
    assets->tim_bitmaps.pushing_up[3] = DEBUGLoadBMP(thread, ReadEntireFile, "tim_pushing_up.bmp");

    assets->tim_bitmaps.pushing_down[0] = DEBUGLoadBMP(thread, ReadEntireFile, "tim_pushing_down.bmp");
    assets->tim_bitmaps.pushing_down[1] = DEBUGLoadBMP(thread, ReadEntireFile, "tim_pushing_down.bmp");
    assets->tim_bitmaps.pushing_down[2] = DEBUGLoadBMP(thread, ReadEntireFile, "tim_pushing_down.bmp");
    assets->tim_bitmaps.pushing_down[3] = DEBUGLoadBMP(thread, ReadEntireFile, "tim_pushing_down.bmp");

    assets->tim_bitmaps.pushing_left[0] = DEBUGLoadBMP(thread, ReadEntireFile, "tim_pushing_left.bmp");
    assets->tim_bitmaps.pushing_left[1] = DEBUGLoadBMP(thread, ReadEntireFile, "tim_pushing_left.bmp");
    assets->tim_bitmaps.pushing_left[2] = DEBUGLoadBMP(thread, ReadEntireFile, "tim_pushing_left.bmp");
    assets->tim_bitmaps.pushing_left[3] = DEBUGLoadBMP(thread, ReadEntireFile, "tim_pushing_left.bmp");

    assets->tim_bitmaps.pushing_right[0] = DEBUGLoadBMP(thread, ReadEntireFile, "tim_pushing_right.bmp");
    assets->tim_bitmaps.pushing_right[1] = DEBUGLoadBMP(thread, ReadEntireFile, "tim_pushing_right.bmp");
    assets->tim_bitmaps.pushing_right[2] = DEBUGLoadBMP(thread, ReadEntireFile, "tim_pushing_right.bmp");
    assets->tim_bitmaps.pushing_right[3] = DEBUGLoadBMP(thread, ReadEntireFile, "tim_pushing_right.bmp");

    assets->demo_map = DEBUGLoadBMP(thread, ReadEntireFile, "demo_scroll.bmp");
    assets->scroll_on_ground = DEBUGLoadBMP(thread, ReadEntireFile, "scroll_on_ground.bmp");

    assets->door = DEBUGLoadBMP(thread, ReadEntireFile, "door.bmp");

    assets->missing_bitmap = DEBUGLoadBMP(thread, ReadEntireFile, "missing_image.bmp");
    assets->wsad_guide = DEBUGLoadBMP(thread, ReadEntireFile, "wsad_guide.bmp");

    assets->pen = DEBUGLoadBMP(thread, ReadEntireFile, "pen.bmp");
    assets->bin = DEBUGLoadBMP(thread, ReadEntireFile, "bin.bmp");

    ResizeAssets(assets, SCALE);
}

function Bitmap
LoadAenigmaAssets(u32 *location) {
    Bitmap result ={ 0 };

    result.width = *location++;
    result.height = *location++;
    result.pixels = location;

    return result;
}

function void
LoadGameAssets(GameAssets *assets) {

    Assert(assets);
#if 0
    assets->water[0] = LoadAenigmaAssets(AENIGMA_ASSET_WATER0);
    assets->water[1] = LoadAenigmaAssets(AENIGMA_ASSET_WATER1);

    assets->grass[0] = LoadAenigmaAssets(AENIGMA_ASSET_GRASS0);
    assets->grass[1] = LoadAenigmaAssets(AENIGMA_ASSET_GRASS1);

    assets->wall = LoadAenigmaAssets(AENIGMA_ASSET_WALL);
    assets->moveable_wall = LoadAenigmaAssets(AENIGMA_ASSET_MOVEABLE_WALL);
    assets->door_portal_horizontal = LoadAenigmaAssets(AENIGMA_ASSET_DOOR_PORTAL_HORIZONTAL);
    assets->door_portal_vertical = LoadAenigmaAssets(AENIGMA_ASSET_DOOR_PORTAL_VERTICAL);
    assets->tree_port_pier = LoadAenigmaAssets(AENIGMA_ASSET_TREE_PORT_PIER);
    assets->tree_port_pier_pole = LoadAenigmaAssets(AENIGMA_ASSET_TREE_PORT_PIER_POLE);

    assets->shadows[ShadowType_Small] = LoadAenigmaAssets(AENIGMA_ASSET_SHADOW_TYPE_SMALL);
    assets->shadows[ShadowType_Medium] = LoadAenigmaAssets(AENIGMA_ASSET_ShadowType_Medium);
    assets->shadows[ShadowType_Large] = LoadAenigmaAssets(AENIGMA_ASSET_ShadowType_Large);

    assets->tim_bitmaps.up[0] = LoadAenigmaAssets(AENIGMA_ASSET_TIM_UP0);
    assets->tim_bitmaps.up[1] = LoadAenigmaAssets(AENIGMA_ASSET_TIM_UP1);

    assets->tim_bitmaps.left[0] = LoadAenigmaAssets(AENIGMA_ASSET_TIM_LEFT0);
    assets->tim_bitmaps.left[1] = LoadAenigmaAssets(AENIGMA_ASSET_TIM_LEFT1);

    assets->tim_bitmaps.right[0] = LoadAenigmaAssets(AENIGMA_ASSET_TIM_RIGHT0);
    assets->tim_bitmaps.right[1] = LoadAenigmaAssets(AENIGMA_ASSET_TIM_RIGHT1);

    assets->tim_bitmaps.down[0] = LoadAenigmaAssets(AENIGMA_ASSET_TIM_DOWN0);
    assets->tim_bitmaps.down[1] = LoadAenigmaAssets(AENIGMA_ASSET_TIM_DOWN1);

    assets->demo_map = LoadAenigmaAssets(AENIGMA_ASSET_DEMO_MAP);

    assets->door = LoadAenigmaAssets(AENIGMA_ASSET_DOOR);

    assets->wsad_guide = LoadAenigmaAssets(AENIGMA_ASSET_WSAD_GUIDE);
#endif
}