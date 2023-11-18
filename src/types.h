#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <stddef.h>

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef s32 b32;

typedef float  f32;
typedef double f64;

typedef size_t memory_index;

typedef union v2 {
    struct {
        f32 x, y;
    };
    f32 m[2];
} v2;

typedef union v2s {
    struct {
        s32 x, y;
    };
    struct {
        s32 width, height;
    };
    s32 m[2];
} v2s;

typedef union v2u {
    struct {
        u32 x, y;
    };
    u32 m[2];
} v2u;

typedef union v3 {
    struct {
        f32 x, y, z;
    };
    f32 m[3];
} v3;

typedef union v3s {
    struct {
        s32 x, y, z;
    };
    s32 m[3];
} v3s;

typedef union v3u {
    struct {
        u32 x, y, z;
    };
    u32 m[3];
} Vectoru;

typedef union v4 {
    struct {
        f32 x, y, z, w;
    };

    struct {
        f32 r, g, b, a;
    };

    f32 m[4];
} v4;

typedef union Vector4s {
    struct {
        s32 x, y, z, w;
    };

    struct {
        s32 r, g, b, a;
    };

    s32 m[4];
} Vector4s;

typedef union Vector4u {
    struct {
        u32 x, y, z, w;
    };

    struct {
        u32 r, g, b, a;
    };

    u32 m[4];
} Vector4u;

#endif