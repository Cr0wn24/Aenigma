#include <stdlib.h>
#include <stdio.h>

#include "asset_builder.h"

#define assert(expr) if (!expr) { * (int *)0 = 0; }

static int 
get_file_size(FILE *file) {
    int result = 0;

    fseek(file, 0L, SEEK_END);
    result = ftell(file);
    rewind(file);

    return result;
}

static Bitmap 
read_file(char *file_name) {
    Bitmap result = { 0 };

    char *input_file_name = file_name;
    FILE *input_file = fopen(input_file_name, "r");

    if (!input_file) {
        printf("Failed to open file: %s, ", input_file_name);
        return result;
    }

    int input_file_size = get_file_size(input_file);

    if (!input_file_size) {
        printf("Failed to get the size of: %s", input_file_name);
        return result;
    }

    char *input_contents = (char *)malloc(input_file_size);

    fread(input_contents, input_file_size, 1, input_file);

    BitmapHeader *header = (BitmapHeader *)input_contents;

    result.pixels = (unsigned int *)(input_contents + header->bitmap_offset);

    result.width = header->width;
    result.height = header->height;

    unsigned int red_mask = header->red_mask;
    unsigned int green_mask = header->green_mask;
    unsigned int blue_mask = header->blue_mask;
    unsigned int alpha_mask = ~(red_mask | green_mask | blue_mask);

    BitScanResult red_shift = find_least_signifcant_set_bit(red_mask);
    BitScanResult green_shift = find_least_signifcant_set_bit(green_mask);
    BitScanResult blue_shift = find_least_signifcant_set_bit(blue_mask);
    BitScanResult alpha_shift = find_least_signifcant_set_bit(alpha_mask);

    assert(red_shift.found);
    assert(green_shift.found);
    assert(blue_shift.found);
    assert(alpha_shift.found);

    unsigned int *source_dest = result.pixels;
    for (int y = 0; y < header->height; ++y) {
        for (int x = 0; x < header->width; ++x) {
            unsigned int color = *source_dest;
            *source_dest =
                ((((color >> alpha_shift.index) & 0xff) << 24) |
                 (((color >> red_shift.index) & 0xff) << 16) |
                 (((color >> green_shift.index) & 0xff) << 8) |
                 (((color >> blue_shift.index) & 0xff) << 0));
            ++source_dest;
        }
    }

    fclose(input_file);

    return result;
}

static void 
process_asset(FILE *output_file, char *file_name, char *name) {
    fprintf(output_file, "static unsigned int %s[] = {\n", name);

    Bitmap bitmap = read_file(file_name);

    unsigned int *pixels = bitmap.pixels;
    fprintf(output_file, "0x%x, ", bitmap.width);
    fprintf(output_file, "0x%x, ", bitmap.height);
    for (unsigned int y = 0; y < bitmap.height; ++y) {
        for (unsigned int x = 0; x < bitmap.width; ++x) {
            fprintf(output_file, "0x%x, ", *pixels);
            ++pixels;
        }
        fprintf(output_file, "\n", *pixels);
    }

    fprintf(output_file, "};\n\n");

}

int 
main(int argc, char **argv) {
    char *output_file_name = "..\\..\\src\\asset_builder_result.c";

    FILE *output_file = fopen(output_file_name, "w");

    if (!output_file) {
        printf("Failed to open file: %s, ", output_file_name);
        return -1;
    }

    process_asset(output_file, "..\\..\\res\\water00.bmp", "AENIGMA_ASSET_WATER0");
    process_asset(output_file, "..\\..\\res\\water01.bmp", "AENIGMA_ASSET_WATER1");

    process_asset(output_file, "..\\..\\res\\grass00.bmp", "AENIGMA_ASSET_GRASS0");
    process_asset(output_file, "..\\..\\res\\grass01.bmp", "AENIGMA_ASSET_GRASS1");

    process_asset(output_file, "..\\..\\res\\wall.bmp", "AENIGMA_ASSET_WALL");
    process_asset(output_file, "..\\..\\res\\moveable_wall.bmp", "AENIGMA_ASSET_MOVEABLE_WALL");
    process_asset(output_file, "..\\..\\res\\door_portal_horizontal.bmp", "AENIGMA_ASSET_DOOR_PORTAL_HORIZONTAL");
    process_asset(output_file, "..\\..\\res\\door_portal_vertical.bmp", "AENIGMA_ASSET_DOOR_PORTAL_VERTICAL");
    process_asset(output_file, "..\\..\\res\\tree_port_pier.bmp", "AENIGMA_ASSET_TREE_PORT_PIER");
    process_asset(output_file, "..\\..\\res\\tree_port_pier_pole.bmp", "AENIGMA_ASSET_TREE_PORT_PIER_POLE");

    process_asset(output_file, "..\\..\\res\\shadow_small.bmp", "AENIGMA_ASSET_ShadowType_Small");
    process_asset(output_file, "..\\..\\res\\shadow_medium.bmp", "AENIGMA_ASSET_ShadowType_Medium");
    process_asset(output_file, "..\\..\\res\\shadow_large.bmp", "AENIGMA_ASSET_ShadowType_Large");

    process_asset(output_file, "..\\..\\res\\tim_up00.bmp", "AENIGMA_ASSET_TIM_UP0");
    process_asset(output_file, "..\\..\\res\\tim_up01.bmp", "AENIGMA_ASSET_TIM_UP1");

    process_asset(output_file, "..\\..\\res\\tim_down00.bmp", "AENIGMA_ASSET_TIM_DOWN0");
    process_asset(output_file, "..\\..\\res\\tim_down01.bmp", "AENIGMA_ASSET_TIM_DOWN1");

    process_asset(output_file, "..\\..\\res\\tim_left00.bmp", "AENIGMA_ASSET_TIM_LEFT0");
    process_asset(output_file, "..\\..\\res\\tim_left01.bmp", "AENIGMA_ASSET_TIM_LEFT1");

    process_asset(output_file, "..\\..\\res\\tim_right00.bmp", "AENIGMA_ASSET_TIM_RIGHT0");
    process_asset(output_file, "..\\..\\res\\tim_right01.bmp", "AENIGMA_ASSET_TIM_RIGHT1");

    process_asset(output_file, "..\\..\\res\\demo_scroll.bmp", "AENIGMA_ASSET_DEMO_MAP");

    process_asset(output_file, "..\\..\\res\\door.bmp", "AENIGMA_ASSET_DOOR");

    process_asset(output_file, "..\\..\\res\\wsad_guide.bmp", "AENIGMA_ASSET_WSAD_GUIDE");

    fclose(output_file);
}