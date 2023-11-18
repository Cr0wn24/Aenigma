#ifndef ASSET_BUILDER_H
#define ASSET_BUILDER_H

#define bool int

#define false 0
#define true 1

#pragma pack(push, 1)
typedef struct Bitmap_Header {
    unsigned char signature[2];
    unsigned int file_size;
    unsigned short reserved0;
    unsigned short reserved1;
    unsigned int bitmap_offset;
    unsigned int size;
    int width;
    int height;
    unsigned short planes;
    unsigned short bits_per_pixel;
    unsigned int compression;
    unsigned int size_of_bitmap;
    int horizontal_res;
    int vertical_res;
    unsigned int colors_used;
    unsigned int colors_important;
    
    unsigned int red_mask;
    unsigned int green_mask;
    unsigned int blue_mask;
} Bitmap_Header;
#pragma pack(pop)

typedef struct Bitmap {
    int width;
    int height;
    unsigned int *pixels;
} Bitmap;

typedef struct Bit_Scan_Result {
    bool found;
    unsigned int index;
} Bit_Scan_Result;

inline Bit_Scan_Result find_least_signifcant_set_bit(unsigned int value) {
    Bit_Scan_Result result = {0};
#if COMPILER_MSVC
    result.found = _BitScanForward((unsigned long *)&result.index, value);
#else
    for (unsigned int test = 0; test < 32; ++test) {
        if (value & (1 << test)) {
            result.index = test;
            result.found = true;
            break;
        }
    }
#endif
    return result;
}

#endif
