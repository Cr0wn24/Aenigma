#ifndef INTRINSICS_H
#define INTRINSICS_H

#include <math.h> 

#include "math.h"

#include <immintrin.h>

function inline s32
RoundReal32ToInt32(f32 real)
{
    // s32 result = (s32)roundf(real);
    s32 result = (s32)(real + 0.5f);
    return result;
}

function inline u32
RoundReal32ToUInt32(f32 real)
{
    u32 result = (u32)roundf(real);
    // u32 result = (u32)(real + 0.5f);
    return result;
}

function inline s32
TruncateReal32ToInt32(f32 real)
{
    s32 result = (s32)real;
    return result;
}

function inline u32
TruncateReal32ToUInt32(f32 real)
{
    u32 result = (u32)real;
    return result;
}

function inline s32
FloorReal32ToInt32(f32 real)
{
    s32 result = (s32)floorf(real);
    return result;
}

function inline u32
FloorReal32ToUInt32(f32 real)
{
    u32 result = (u32)floorf(real);
    return result;
}

typedef struct BitScanResult
{
    b32 found;
    u32 index;
} BitScanResult;

function inline BitScanResult
FindLeastSignificantSetBit(u32 value)
{
    BitScanResult result = { 0 };
#if COMPILER_MSVC
    result.found = _BitScanForward((unsigned long *)&result.index, value);
#else
    for(u32 test = 0; test < 32; ++test)
    {
        if(value & (1 << test))
        {
            result.index = test;
            result.found = true;
            break;
        }
    }
#endif
    return result;
}

#endif