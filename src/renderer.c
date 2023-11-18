// @Speed: Use SIMD

#include "renderer.h"

function inline u32
Unpack4x8(v4 a) {
    u32 result;

    result =
        ((RoundReal32ToInt32(a.a) << 24) |
         (RoundReal32ToInt32(a.r) << 16) |
         (RoundReal32ToInt32(a.g) << 8) |
         (RoundReal32ToInt32(a.b)));

    return result;
}

// @Speed: SIMD    
function inline v4
Pack4x8(u32 value) {
    v4 result;

    result.a = (f32)((value >> (8 * 3)) & 0xff);
    result.r = (f32)((value >> (8 * 2)) & 0xff);
    result.g = (f32)((value >> (8 * 1)) & 0xff);
    result.b = (f32)((value >> (8 * 0)) & 0xff);

    return result;

}

function inline void
PlotPixelUnchecked(RenderBuffer *buffer, s32 x, s32 y, u32 color) {
    *((u32 *)buffer->memory + x + y * buffer->width) = color;
}

function inline void
PlotPixelChecked(RenderBuffer *buffer, s32 x, s32 y, u32 color) {
    if (x >= 0 && y >= 0 && x < buffer->width && y < buffer->height) {
        *((u32 *)buffer->memory + x + y * buffer->width) = color;
    }
}

function inline void
PlotPixelAlpha(RenderBuffer *buffer, s32 x, s32 y, v4 color) {
    color.a = Clamp(0, color.a, 255.0f);
    u32 src_color = Unpack4x8(color);
    if (x >= 0 && y >= 0 && x < buffer->width && y < buffer->height) {
        u32 *dest_pixel = (u32 *)buffer->memory + x + y * buffer->width;
        f32 a = (f32)((src_color >> 24) & 0xff) / 255.0f;

        if (a == 1.0f) {
            *dest_pixel = src_color;
        } else if (a != 0.0f) {
            f32 sr = (f32)((src_color >> 16) & 0xff);
            f32 sg = (f32)((src_color >> 8) & 0xff);
            f32 sb = (f32)((src_color >> 0) & 0xff);

            f32 dr = (f32)((*dest_pixel >> 16) & 0xff);
            f32 dg = (f32)((*dest_pixel >> 8) & 0xff);
            f32 db = (f32)((*dest_pixel >> 0) & 0xff);

            f32 r = (1.0f - a) * dr + a * sr;
            f32 g = (1.0f - a) * dg + a * sg;
            f32 b = (1.0f - a) * db + a * sb;

            *dest_pixel =
                (((u32)(r + 0.5f) << 16) |
                 ((u32)(g + 0.5f) << 8) |
                 ((u32)(b + 0.5f) << 0));
        }
    }
}

function void
ClearScreen(RenderBuffer *buffer, v4 color) {
    u32 color_unpacked = Unpack4x8(V4MulF32(color, 255.0f));
    u32 *pixels = (u32 *)buffer->memory;
#if AENIGMA_SIMD
    __m256i color_8x = _mm256_set1_epi32(color_unpacked);
    __m256i *pixels_8x = (__m256i *)pixels;
    for (s32 i = 0; i < buffer->height * buffer->width; i += 8) {
        _mm256_store_si256(pixels_8x++, color_8x);
    }
#else
    for (s32 i = 0; i < buffer->height * buffer->width; ++i) {
        *pixels++ = color_unpacked;
    }
#endif
}

function inline b32
HasArea(v2 min, v2 max) {
    b32 result = false;
    f32 width = max.x - min.x;
    f32 height = max.y - min.y;

    result = ((width * height) > 0);

    return result;
}

// @Speed: SIMD
function void
DrawRectAlphaInPixels(RenderBuffer *buffer, v2 min, v2 max, v4 color) {

    if (!HasArea(min, max)) {
        return;
    }

    s32 x0 = RoundReal32ToInt32(min.x);
    s32 y0 = RoundReal32ToInt32(min.y);
    s32 x1 = RoundReal32ToInt32(max.x);
    s32 y1 = RoundReal32ToInt32(max.y);

    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 > buffer->width)  x1 = buffer->width;
    if (y1 > buffer->height) y1 = buffer->height;

    f32 alpha = color.a;

    color = V4MulF32(color, 255.0f);
    u32 color_unpacked = Unpack4x8(color);

    u8 *row = ((u8 *)buffer->memory + x0 * buffer->bytes_per_pixel + y0 * buffer->pitch);
    for (s32 y = y0; y < y1; ++y) {
        u32 *pixel = (u32 *)row;
        for (s32 x = x0; x < x1; ++x) {
            if (alpha == 1.0f) {
                *(pixel) = color_unpacked;
            } else if (alpha != 0.0f) {
                f32 dr = (f32)((*pixel >> 16) & 0xff);
                f32 dg = (f32)((*pixel >> 8) & 0xff);
                f32 db = (f32)((*pixel >> 0) & 0xff);

                f32 r = (1.0f - alpha) * dr + alpha * color.r;
                f32 g = (1.0f - alpha) * dg + alpha * color.g;
                f32 b = (1.0f - alpha) * db + alpha * color.b;

                *pixel =
                    (((u32)(r + 0.5f) << 16) |
                     ((u32)(g + 0.5f) << 8) |
                     ((u32)(b + 0.5f) << 0));
            }
            ++pixel;
        }
        row += buffer->pitch;
    }
}
function void
DrawRectAlpha(RenderBuffer *buffer, v2 min, v2 max, v4 color) {
    min.x *= buffer->width;
    min.y *= buffer->height;
    max.x *= buffer->width;
    max.y *= buffer->height;
    DrawRectAlphaInPixels(buffer, min, max, color);
}

// @Incomplete @Speed: Make this cleaner, try to use one loop instead of 4, SIMD
function void
DrawRoundedRectInPixels(RenderBuffer *buffer, v2 min, v2 max, s32 roundness, v4 color) {
    if (!HasArea(min, max)) {
        return;
    }

    f32 width = (max.x - min.x);
    s32 radius = RoundReal32ToInt32((f32)roundness / 100.0f * width);

    s32 x0 = RoundReal32ToInt32(min.x);
    s32 y0 = RoundReal32ToInt32(min.y);
    s32 x1 = RoundReal32ToInt32(max.x);
    s32 y1 = RoundReal32ToInt32(max.y);

    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 > buffer->width)  x1 = buffer->width;
    if (y1 > buffer->height) y1 = buffer->height;


    // rectangle 1
    DrawRectAlphaInPixels(buffer,
                          V2(min.x + radius, min.y + 1),
                          V2(max.x - radius, max.y - 1),
                          color);

    // rectangle 2
    DrawRectAlphaInPixels(buffer,
                          V2(min.x + 1, min.y + radius),
                          V2(min.x + radius, max.y - radius),
                          color);

    // rectangle 3
    DrawRectAlphaInPixels(buffer,
                          V2(max.x - radius, min.y + radius),
                          V2(max.x - 1, max.y - radius),
                          color);

    color = V4MulF32(color, 255.0f);
    u32 color_unpacked = Unpack4x8(color);

    f32 aa_factor = 1000.0f;

    // upper-left
    u8 *row = ((u8 *)buffer->memory + x0 * buffer->bytes_per_pixel + y0 * buffer->pitch);
    for (s32 y = 0; y < radius; ++y) {
        u32 *pixel = (u32 *)row;
        for (s32 x = 0; x < radius; ++x) {
            s32 dx = x - radius;
            s32 dy = y - radius;
            f32 alpha = 1.0f;
            f32 distance = (sqrtf((f32)(Square(dx) + Square(dy))));
            distance /= (f32)radius;
            alpha = 1.0f - distance;
            alpha *= aa_factor;
            alpha = Clamp(0, alpha, 1.0f);
            if (alpha > 0.0f) {
                if (alpha == 1.0f) {
                    *pixel = color_unpacked;
                } else if (alpha != 0.0f) {
                    f32 dr = (f32)((*pixel >> 16) & 0xff);
                    f32 dg = (f32)((*pixel >> 8) & 0xff);
                    f32 db = (f32)((*pixel >> 0) & 0xff);

                    f32 r = (1.0f - alpha) * dr + alpha * color.r;
                    f32 g = (1.0f - alpha) * dg + alpha * color.g;
                    f32 b = (1.0f - alpha) * db + alpha * color.b;

                    *pixel =
                        (((u32)(r + 0.5f) << 16) |
                         ((u32)(g + 0.5f) << 8) |
                         ((u32)(b + 0.5f) << 0));
                }
            }
            ++pixel;
        }
        row += buffer->pitch;
    }

    // top-right
    row = ((u8 *)buffer->memory + (x1 - radius) * buffer->bytes_per_pixel + y0 * buffer->pitch);
    for (s32 y = 0; y < radius; ++y) {
        u32 *pixel = (u32 *)row;
        for (s32 x = (radius - 1); x >= 0; --x) {
            s32 dx = x - radius;
            s32 dy = y - radius;
            f32 alpha = 1.0f;
            f32 distance = (sqrtf((f32)(Square(dx) + Square(dy))));
            distance /= (f32)radius;
            alpha = 1.0f - distance;
            alpha *= aa_factor;
            alpha = Clamp(0, alpha, 1.0f);
            if (alpha > 0.0f) {
                if (alpha == 1.0f) {
                    *pixel = color_unpacked;
                } else if (alpha != 0.0f) {
                    f32 dr = (f32)((*pixel >> 16) & 0xff);
                    f32 dg = (f32)((*pixel >> 8) & 0xff);
                    f32 db = (f32)((*pixel >> 0) & 0xff);

                    f32 r = (1.0f - alpha) * dr + alpha * color.r;
                    f32 g = (1.0f - alpha) * dg + alpha * color.g;
                    f32 b = (1.0f - alpha) * db + alpha * color.b;

                    *pixel =
                        (((u32)(r + 0.5f) << 16) |
                         ((u32)(g + 0.5f) << 8) |
                         ((u32)(b + 0.5f) << 0));
                }
            }
            ++pixel;
        }
        row += buffer->pitch;
    }

    // bottom-left
    row = ((u8 *)buffer->memory + (x0)*buffer->bytes_per_pixel + (y1 - radius) * buffer->pitch);
    for (s32 y = (radius - 1); y >= 0; --y) {
        u32 *pixel = (u32 *)row;
        for (s32 x = 0; x < radius; ++x) {
            s32 dx = x - radius;
            s32 dy = y - radius;
            f32 alpha = 1.0f;
            f32 distance = (sqrtf((f32)(Square(dx) + Square(dy))));
            distance /= (f32)radius;
            alpha = 1.0f - distance;
            alpha *= aa_factor;
            alpha = Clamp(0, alpha, 1.0f);
            if (alpha > 0.0f) {
                if (alpha == 1.0f) {
                    *pixel = color_unpacked;
                } else if (alpha != 0.0f) {
                    f32 dr = (f32)((*pixel >> 16) & 0xff);
                    f32 dg = (f32)((*pixel >> 8) & 0xff);
                    f32 db = (f32)((*pixel >> 0) & 0xff);

                    f32 r = (1.0f - alpha) * dr + alpha * color.r;
                    f32 g = (1.0f - alpha) * dg + alpha * color.g;
                    f32 b = (1.0f - alpha) * db + alpha * color.b;

                    *pixel =
                        (((u32)(r + 0.5f) << 16) |
                         ((u32)(g + 0.5f) << 8) |
                         ((u32)(b + 0.5f) << 0));
                }
            }
            ++pixel;
        }
        row += buffer->pitch;
    }
    // bottom-righht
    row = ((u8 *)buffer->memory + (x1 - radius) * buffer->bytes_per_pixel + (y1 - radius) * buffer->pitch);
    for (s32 y = (radius - 1); y >= 0; --y) {
        u32 *pixel = (u32 *)row;
        for (s32 x = (radius - 1); x >= 0; --x) {
            s32 dx = x - radius;
            s32 dy = y - radius;
            f32 alpha = 1.0f;
            f32 distance = (sqrtf((f32)(Square(dx) + Square(dy))));
            distance /= (f32)radius;
            alpha = 1.0f - distance;
            alpha *= aa_factor;
            alpha = Clamp(0, alpha, 1.0f);
            if (alpha > 0.0f) {
                if (alpha == 1.0f) {
                    *pixel = color_unpacked;
                } else if (alpha != 0.0f) {
                    f32 dr = (f32)((*pixel >> 16) & 0xff);
                    f32 dg = (f32)((*pixel >> 8) & 0xff);
                    f32 db = (f32)((*pixel >> 0) & 0xff);

                    f32 r = (1.0f - alpha) * dr + alpha * color.r;
                    f32 g = (1.0f - alpha) * dg + alpha * color.g;
                    f32 b = (1.0f - alpha) * db + alpha * color.b;

                    *pixel =
                        (((u32)(r + 0.5f) << 16) |
                         ((u32)(g + 0.5f) << 8) |
                         ((u32)(b + 0.5f) << 0));
                }
            }
            ++pixel;
        }
        row += buffer->pitch;
    }
}

function void
DrawRoundedRect(RenderBuffer *buffer, v2 min, v2 max, s32 roundness, v4 color) {
    min.x *= buffer->width;
    min.y *= buffer->height;
    max.x *= buffer->width;
    max.y *= buffer->height;
    DrawRoundedRectInPixels(buffer, min, max, roundness, color);
}

// @Speed: SIMD
function void
DrawRectInPixels(RenderBuffer *buffer, v2 min, v2 max, v4 color) {

    s32 x0 = RoundReal32ToInt32(min.x);
    s32 y0 = RoundReal32ToInt32(min.y);
    s32 x1 = RoundReal32ToInt32(max.x);
    s32 y1 = RoundReal32ToInt32(max.y);

    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 > buffer->width)  x1 = buffer->width;
    if (y1 > buffer->height) y1 = buffer->height;

    color = V4MulF32(color, 255.0f);
    u32 color_unpacked = Unpack4x8(color);

    u8 *row = ((u8 *)buffer->memory + x0 * buffer->bytes_per_pixel + y0 * buffer->pitch);
    for (s32 y = y0; y < y1; ++y) {
        u32 *pixel = (u32 *)row;
        for (s32 x = x0; x < x1; ++x) {
            *(pixel++) = color_unpacked;
        }
        row += buffer->pitch;
    }
}

function void
DrawRect(RenderBuffer *buffer, v2 min, v2 max, v4 color) {
    min.x *= buffer->width;
    min.y *= buffer->height;
    max.x *= buffer->width;
    max.y *= buffer->height;
    DrawRectInPixels(buffer, min, max, color);
}

// @Speed: SIMD
function void
DrawBitmapLightnessInPixels(RenderBuffer *buffer, Bitmap *bitmap, v2 min, f32 lightness) {
    Assert(bitmap);
    s32 min_x = RoundReal32ToInt32(min.x);
    s32 min_y = RoundReal32ToInt32(min.y);
    s32 max_x = RoundReal32ToInt32(min.x + (f32)bitmap->width);
    s32 max_y = RoundReal32ToInt32(min.y + (f32)bitmap->height);

    if (min_x >= buffer->width || min_y >= buffer->height || max_x < 0 || max_y < 0) {
        return;
    }

    s32 src_offset_x = 0;
    s32 src_offset_y = 0;
    if (min_x < 0) {
        src_offset_x = -min_x;
        min_x = 0;
    }

    if (min_y < 0) {
        src_offset_y = -min_y;
        min_y = 0;
    }

    if (max_x > buffer->width) {
        max_x = buffer->width;
    }

    if (max_y > buffer->height) {
        max_y = buffer->height;
    }

    u32 *src_row = bitmap->pixels + bitmap->width * (bitmap->height - 1);
    s32 test1 = src_offset_x;
    s32 test2 = -src_offset_y * bitmap->width;
    src_row += test1 + test2;

    u8 *dest_row = ((u8 *)buffer->memory + min_x * buffer->bytes_per_pixel + min_y * buffer->pitch);

    for (s32 y = min_y; y < max_y; ++y) {

        u32 *dest_pixel = (u32 *)dest_row;
        u32 *src_pixel = src_row;

        for (s32 x = min_x; x < max_x; ++x) {

            f32 a = (f32)((*src_pixel >> 24) & 0xff) / 255.0f;

            f32 sr = (f32)((*src_pixel >> 16) & 0xff);
            f32 sg = (f32)((*src_pixel >> 8) & 0xff);
            f32 sb = (f32)((*src_pixel >> 0) & 0xff);

            sr *= lightness;
            sg *= lightness;
            sb *= lightness;

            sr = Clamp(0, sr, 255);
            sg = Clamp(0, sg, 255);
            sb = Clamp(0, sb, 255);

            if (a == 1.0f) {
                *dest_pixel = ((RoundReal32ToInt32(sr)) << 16 |
                               (RoundReal32ToInt32(sg) << 8) |
                               (RoundReal32ToInt32(sb) << 0));
            } else if (a != 0.0f) {
                f32 dr = (f32)((*dest_pixel >> 16) & 0xff);
                f32 dg = (f32)((*dest_pixel >> 8) & 0xff);
                f32 db = (f32)((*dest_pixel >> 0) & 0xff);

                f32 r = (1.0f - a) * dr + a * sr;
                f32 g = (1.0f - a) * dg + a * sg;
                f32 b = (1.0f - a) * db + a * sb;

                *dest_pixel =
                    (((u32)(r + 0.5f) << 16) |
                     ((u32)(g + 0.5f) << 8) |
                     ((u32)(b + 0.5f) << 0));
            }

            dest_pixel++;
            src_pixel++;
        }
        dest_row += buffer->pitch;
        src_row -= bitmap->width;
    }
}

#if 1
function void
DrawBitmapInPixels(RenderBuffer *buffer, Bitmap *bitmap, v2 min) {

    Assert(bitmap);
    s32 min_x = RoundReal32ToInt32(min.x);
    s32 min_y = RoundReal32ToInt32(min.y);
    s32 max_x = RoundReal32ToInt32(min.x + (f32)bitmap->width);
    s32 max_y = RoundReal32ToInt32(min.y + (f32)bitmap->height);

    if (min_x >= buffer->width || min_y >= buffer->height || max_x < 0 || max_y < 0) {
        return;
    }

    s32 src_offset_x = 0;
    if (min_x < 0) {
        src_offset_x = -min_x;
        min_x = 0;
    }

    s32 src_offset_y = 0;
    if (min_y < 0) {
        src_offset_y = -min_y;
        min_y = 0;
    }

    if (max_x > buffer->width) {
        max_x = buffer->width;
    }

    if (max_y > buffer->height) {
        max_y = buffer->height;
    }

    u32 *src_row = bitmap->pixels + bitmap->width * (bitmap->height - 1);
    src_row += src_offset_x - src_offset_y * bitmap->width;

    u8 *dest_row = ((u8 *)buffer->memory + min_x * buffer->bytes_per_pixel + min_y * buffer->pitch);

    for (s32 y = min_y; y < max_y; ++y) {

        u32 *dest_pixel = (u32 *)dest_row;
        u32 *src_pixel = src_row;

        s32 x;
        for (x = min_x; (x + 8) <= max_x; x += 8) {

            __m256i src_color = _mm256_load_si256((__m256i *)src_pixel);
            __m256i dest_color = _mm256_load_si256((__m256i *)dest_pixel);

            // convert to 0-255

            // destination 
            __m256i dr = _mm256_srli_epi32(dest_color, 16);
            dr = _mm256_and_si256(dr, _mm256_set1_epi32(0xff));

            __m256i dg = _mm256_srli_epi32(dest_color, 8);
            dg = _mm256_and_si256(dg, _mm256_set1_epi32(0xff));

            __m256i db = _mm256_and_si256(dest_color, _mm256_set1_epi32(0xff));

            // source
            __m256i sa = _mm256_srli_epi32(src_color, 24);
            sa = _mm256_and_si256(sa, _mm256_set1_epi32(0xff));

            __m256i sr = _mm256_srli_epi32(src_color, 16);
            sr = _mm256_and_si256(sr, _mm256_set1_epi32(0xff));

            __m256i sg = _mm256_srli_epi32(src_color, 8);
            sg = _mm256_and_si256(sg, _mm256_set1_epi32(0xff));

            __m256i sb = _mm256_and_si256(src_color, _mm256_set1_epi32(0xff));

            // convert to floating point in order for alpha blending
            __m256 drf = _mm256_cvtepi32_ps(dr);
            __m256 dgf = _mm256_cvtepi32_ps(dg);
            __m256 dbf = _mm256_cvtepi32_ps(db);

            __m256 saf = _mm256_cvtepi32_ps(sa);
            __m256 srf = _mm256_cvtepi32_ps(sr);
            __m256 sgf = _mm256_cvtepi32_ps(sg);
            __m256 sbf = _mm256_cvtepi32_ps(sb);

            // divide alpha to get 0.0f-1.0f
            saf = _mm256_div_ps(saf, _mm256_set1_ps(255.0f));

            // do the alpha blending
            srf = _mm256_add_ps(srf, _mm256_mul_ps(_mm256_sub_ps(_mm256_set1_ps(1.0), saf), drf));

            sgf = _mm256_add_ps(sgf, _mm256_mul_ps(_mm256_sub_ps(_mm256_set1_ps(1.0), saf), dgf));

            sbf = _mm256_add_ps(sbf, _mm256_mul_ps(_mm256_sub_ps(_mm256_set1_ps(1.0), saf), dbf));

            // store back to 0-255 srgb
            __m256i rr = _mm256_slli_epi32(_mm256_cvtps_epi32(srf), 16);
            __m256i rg = _mm256_slli_epi32(_mm256_cvtps_epi32(sgf), 8);
            __m256i rb = _mm256_cvtps_epi32(sbf);

            // OR together all colors
            __m256i result_color = _mm256_or_epi32(_mm256_or_epi32(rr, rg), rb);

            _mm256_store_si256((__m256i *)dest_pixel, result_color);

            dest_pixel += 8;
            src_pixel += 8;
        }

        // process remaining pixels
        for (; x < max_x; ++x) {
            f32 a = (f32)((*src_pixel >> 24) & 0xff) / 255.0f;

            if (a == 1.0f) {
                *dest_pixel = *src_pixel;
            } else if (a > 0.0f) {
                f32 sr = (f32)((*src_pixel >> 16) & 0xff);
                f32 sg = (f32)((*src_pixel >> 8) & 0xff);
                f32 sb = (f32)((*src_pixel >> 0) & 0xff);

                f32 dr = (f32)((*dest_pixel >> 16) & 0xff);
                f32 dg = (f32)((*dest_pixel >> 8) & 0xff);
                f32 db = (f32)((*dest_pixel >> 0) & 0xff);

                f32 r = (1.0f - a) * dr + a * sr;
                f32 g = (1.0f - a) * dg + a * sg;
                f32 b = (1.0f - a) * db + a * sb;

                *dest_pixel =
                    (((u32)(r + 0.5f) << 16) |
                     ((u32)(g + 0.5f) << 8) |
                     ((u32)(b + 0.5f) << 0));
            }

            dest_pixel++;
            src_pixel++;
        }

        dest_row += buffer->pitch;
        src_row -= bitmap->width;
    }
}

#else

function void
DrawBitmapInPixels(RenderBuffer *buffer, Bitmap *bitmap, v2 min) {
    Assert(bitmap);
    s32 min_x = RoundReal32ToInt32(min.x);
    s32 min_y = RoundReal32ToInt32(min.y);
    s32 max_x = RoundReal32ToInt32(min.x + (f32)bitmap->width);
    s32 max_y = RoundReal32ToInt32(min.y + (f32)bitmap->height);

    if (min_x >= buffer->width || min_y >= buffer->height || max_x < 0 || max_y < 0) {
        return;
    }

    s32 src_offset_x = 0;
    if (min_x < 0) {
        src_offset_x = -min_x;
        min_x = 0;
    }

    s32 src_offset_y = 0;
    if (min_y < 0) {
        src_offset_y = -min_y;
        min_y = 0;
    }

    if (max_x > buffer->width) {
        max_x = buffer->width;
    }

    if (max_y > buffer->height) {
        max_y = buffer->height;
    }

    u32 *src_row = bitmap->pixels + bitmap->width * (bitmap->height - 1);
    src_row += src_offset_x - src_offset_y * bitmap->width;

    u8 *dest_row = ((u8 *)buffer->memory + min_x * buffer->bytes_per_pixel + min_y * buffer->pitch);

    for (s32 y = min_y; y < max_y; ++y) {

        u32 *dest_pixel = (u32 *)dest_row;
        u32 *src_pixel = src_row;

        for (s32 x = min_x; x < max_x; ++x) {
            f32 a = (f32)((*src_pixel >> 24) & 0xff) / 255.0f;

            if (a == 1.0f) {
                *dest_pixel = *src_pixel;
            } else if (a > 0.0f) {

                f32 sr = (f32)((*src_pixel >> 16) & 0xff);
                f32 sg = (f32)((*src_pixel >> 8) & 0xff);
                f32 sb = (f32)((*src_pixel >> 0) & 0xff);

                f32 dr = (f32)((*dest_pixel >> 16) & 0xff);
                f32 dg = (f32)((*dest_pixel >> 8) & 0xff);
                f32 db = (f32)((*dest_pixel >> 0) & 0xff);

                f32 r = (1.0f - a) * dr + sr;
                f32 g = (1.0f - a) * dg + sg;
                f32 b = (1.0f - a) * db + sb;

                *dest_pixel =
                    (((u32)(r + 0.5f) << 16) |
                     ((u32)(g + 0.5f) << 8) |
                     ((u32)(b + 0.5f) << 0));
            }

            dest_pixel++;
            src_pixel++;
        }

        dest_row += buffer->pitch;
        src_row -= bitmap->width;
    }
}
#endif

// @Speed: SIMD
function void
DrawBitmapAlphaInPixels(RenderBuffer *buffer, Bitmap *bitmap, v2 min, f32 alpha) {
    Assert(bitmap);
    s32 min_x = RoundReal32ToInt32(min.x);
    s32 min_y = RoundReal32ToInt32(min.y);
    s32 max_x = RoundReal32ToInt32(min.x + (f32)bitmap->width);
    s32 max_y = RoundReal32ToInt32(min.y + (f32)bitmap->height);

    if (min_x >= buffer->width || min_y >= buffer->height || max_x < 0 || max_y < 0) {
        return;
    }

    s32 src_offset_x = 0;
    if (min_x < 0) {
        src_offset_x = -min_x;
        min_x = 0;
    }

    s32 src_offset_y = 0;
    if (min_y < 0) {
        src_offset_y = -min_y;
        min_y = 0;
    }

    if (max_x > buffer->width) {
        max_x = buffer->width;
    }

    if (max_y > buffer->height) {
        max_y = buffer->height;
    }

    u32 *src_row = bitmap->pixels + bitmap->width * (bitmap->height - 1);
    src_row += src_offset_x - src_offset_y * bitmap->width;

    u8 *dest_row = ((u8 *)buffer->memory + min_x * buffer->bytes_per_pixel + min_y * buffer->pitch);
    for (s32 y = min_y; y < max_y; ++y) {

        u32 *dest_pixel = (u32 *)dest_row;
        u32 *src_pixel = src_row;

        for (s32 x = min_x; x < max_x; ++x) {

            f32 a = (f32)((*src_pixel >> 24) & 0xff) / 255.0f;
            a -= alpha;
            a = Clamp(0, a, 1.0f);

            if (a == 1.0f) {
                *dest_pixel = *src_pixel;
            } else if (a != 0.0f) {
                f32 sr = (f32)((*src_pixel >> 16) & 0xff);
                f32 sg = (f32)((*src_pixel >> 8) & 0xff);
                f32 sb = (f32)((*src_pixel >> 0) & 0xff);

                f32 dr = (f32)((*dest_pixel >> 16) & 0xff);
                f32 dg = (f32)((*dest_pixel >> 8) & 0xff);
                f32 db = (f32)((*dest_pixel >> 0) & 0xff);

                f32 r = (1.0f - a) * dr + a * sr;
                f32 g = (1.0f - a) * dg + a * sg;
                f32 b = (1.0f - a) * db + a * sb;

                *dest_pixel =
                    (((u32)(r + 0.5f) << 16) |
                     ((u32)(g + 0.5f) << 8) |
                     ((u32)(b + 0.5f) << 0));
            }

            dest_pixel++;
            src_pixel++;
        }
        dest_row += buffer->pitch;
        src_row -= bitmap->width;
    }
}

function void
DrawBitmapAlignedInPixels(RenderBuffer *buffer, Bitmap *bitmap, v2 min, v2 align) {
    min.x -= align.x;
    min.y -= align.y;
    DrawBitmapInPixels(buffer, bitmap, min);
}

function void
DrawLineInPixels(RenderBuffer *buffer, v2 min, v2 max, v4 color) {

    // @Speed @Incomplete: Proper line clipping

    s32 x0 = RoundReal32ToInt32(min.x);
    s32 y0 = RoundReal32ToInt32(min.y);
    s32 x1 = RoundReal32ToInt32(max.x);
    s32 y1 = RoundReal32ToInt32(max.y);

    b32 steep = false;
    if (Abs(x0 - x1) < Abs(y0 - y1)) {
        Swap(x0, y0, s32);
        Swap(x1, y1, s32);
        steep = true;
    }
    if (x0 > x1) {
        Swap(x0, x1, s32);
        Swap(y0, y1, s32);
    }

    s32 dx = x1 - x0;
    s32 dy = y1 - y0;
    s32 derror2 = Abs(dy) * 2;
    s32 error2 = 0;
    s32 y = y0;
    s32 y_incr = (y1 > y0 ? 1 : -1);
    u32 color_unpacked = Unpack4x8(V4MulF32(color, 255.0f));
    if (steep) {
        for (s32 x = x0; x <= x1; x++) {
            PlotPixelChecked(buffer, y, x, color_unpacked);
            error2 += derror2;
            if (error2 > dx) {
                y += y_incr;
                error2 -= dx * 2;
            }
        }
    } else {

        for (s32 x = x0; x <= x1; x++) {
            PlotPixelChecked(buffer, x, y, color_unpacked);
            error2 += derror2;
            if (error2 > dx) {
                y += y_incr;
                error2 -= dx * 2;
            }
        }
    }
}

function void
DrawLine(RenderBuffer *buffer, v2 min, v2 max, v4 color) {
    min.x *= buffer->width;
    min.y *= buffer->height;
    max.x *= buffer->width;
    max.y *= buffer->height;
    DrawLineInPixels(buffer, min, max, color);
}

function void
DrawBitmapZoomInPixels2(RenderBuffer *buffer, Bitmap *bitmap, v2 min, f32 zoom) {
    s32 min_x = RoundReal32ToInt32(min.x);
    s32 min_y = RoundReal32ToInt32(min.y);
    s32 max_x = RoundReal32ToInt32(min.x + zoom * bitmap->width);
    s32 max_y = RoundReal32ToInt32(min.y + zoom * bitmap->height);

    if (min_x >= buffer->width || min_y >= buffer->height || max_x < 0 || max_y < 0) {
        return;
    }

    if (max_y > buffer->height) {
        max_y = buffer->height;
    }

    if (max_x > buffer->width) {
        max_x = buffer->width;
    }

    s32 start_y = min_y;
    s32 start_x = min_x;

    s32 dest_width = max_x - min_x;
    s32 dest_height = max_y - min_y;

    min_x = 0;
    min_y = 0;

    if (start_y < 0) {
        min_y = -start_y;
        start_y = 0;
    }

    if (start_x < 0) {
        min_x = -start_x;
        start_x = 0;
    }

    u32 *src_row = (u32 *)bitmap->pixels + bitmap->width * (bitmap->height - 1);

    u32 *dest_row = (u32 *)buffer->memory + start_x + start_y * buffer->width;

#if 0
    s32 dest_width = (s32)roundf(bitmap->width * zoom);
    s32 dest_height = (s32)roundf(bitmap->height * zoom);
#else
#endif
    for (s32 y = start_y; y < max_y; ++y) {
        u32 *dest_pixel = dest_row;
        for (s32 x = start_x; x < max_x; ++x) {
            s32 rel_x = x - start_x;
            s32 rel_y = y - start_y;

            f32 test_x = (f32)(rel_x + min_x) / (f32)dest_width;
            f32 test_y = (f32)(rel_y + min_y) / (f32)dest_height;

            s32 source_x = (s32)floorf(test_x * bitmap->width);
            s32 source_y = (s32)floorf(test_y * bitmap->height);

            u32 src_color = *(src_row + source_x - source_y * bitmap->width);
            f32 a = (f32)((src_color >> 24) & 0xff) / 255.0f;

            if (a == 1.0f) {
                *dest_pixel = src_color;
            } else if (a > 0.0f) {
                f32 sr = (f32)((src_color >> 16) & 0xff);
                f32 sg = (f32)((src_color >> 8) & 0xff);
                f32 sb = (f32)((src_color >> 0) & 0xff);

                f32 dr = (f32)((*dest_pixel >> 16) & 0xff);
                f32 dg = (f32)((*dest_pixel >> 8) & 0xff);
                f32 db = (f32)((*dest_pixel >> 0) & 0xff);

                f32 r = (1.0f - a) * dr + a * sr;
                f32 g = (1.0f - a) * dg + a * sg;
                f32 b = (1.0f - a) * db + a * sb;

                *dest_pixel =
                    (((u32)(r + 0.5f) << 16) |
                     ((u32)(g + 0.5f) << 8) |
                     ((u32)(b + 0.5f) << 0));
            }
            dest_pixel++;
        }
        dest_row += buffer->width;
    }
}

function void
DrawBitmapZoomInPixels(RenderBuffer *buffer, Bitmap *bitmap, v2 min, f32 zoom) {
    f32 inv_scale = 1.0f / zoom;

    s32 min_x = RoundReal32ToInt32(min.x);
    s32 min_y = RoundReal32ToInt32(min.y);
    s32 max_x = RoundReal32ToInt32(min.x + zoom * bitmap->width);
    s32 max_y = RoundReal32ToInt32(min.y + zoom * bitmap->height);

    if (min_x >= buffer->width || min_y >= buffer->height || max_x < 0 || max_y < 0) {
        return;
    }

    if (max_y > buffer->height) {
        max_y = buffer->height;
    }

    if (max_x > buffer->width) {
        max_x = buffer->width;
    }

    s32 start_y = min_y;
    s32 start_x = min_x;

    min_x = 0;
    min_y = 0;

    if (start_y < 0) {
        min_y = -start_y;
        start_y = 0;
    }

    if (start_x < 0) {
        min_x = -start_x;
        start_x = 0;
    }

    u32 *src_row = (u32 *)bitmap->pixels + bitmap->width * (bitmap->height - 1);

    u32 *dest_row = (u32 *)buffer->memory + start_x + start_y * buffer->width;

    for (s32 y = start_y; y < max_y; ++y) {
        u32 *src_pixel = src_row - (s32)((y + min_y - start_y) * inv_scale) * bitmap->width;

        u32 *dest_pixel = dest_row;

        for (s32 x = (min_x); x < (max_x + min_x - start_x); x++) {
            s32 src_x = (s32)(x * inv_scale);

            u32 src_color = *(src_pixel + src_x);
            f32 a = (f32)((src_color >> 24) & 0xff) / 255.0f;

            if (a == 1.0f) {
                *dest_pixel = src_color;
            } else if (a > 0.0f) {
                f32 sr = (f32)((src_color >> 16) & 0xff);
                f32 sg = (f32)((src_color >> 8) & 0xff);
                f32 sb = (f32)((src_color >> 0) & 0xff);

                f32 dr = (f32)((*dest_pixel >> 16) & 0xff);
                f32 dg = (f32)((*dest_pixel >> 8) & 0xff);
                f32 db = (f32)((*dest_pixel >> 0) & 0xff);

                f32 r = (1.0f - a) * dr + a * sr;
                f32 g = (1.0f - a) * dg + a * sg;
                f32 b = (1.0f - a) * db + a * sb;

                *dest_pixel =
                    (((u32)(r + 0.5f) << 16) |
                     ((u32)(g + 0.5f) << 8) |
                     ((u32)(b + 0.5f) << 0));
            }
            dest_pixel++;
        }

        dest_row += buffer->width;
    }
}

// @Speed: SIMD

function void
DrawBitmapStretchInPixels(RenderBuffer *buffer, Bitmap *bitmap, v2 min, v2 max) {

    Assert(bitmap);

    f32 dest_width = max.x - min.x;
    f32 dest_height = max.y - min.y;

    f32 zoom_x = dest_width / bitmap->width;
    f32 zoom_y = dest_height / bitmap->height;

    Assert(zoom_x != 0);
    Assert(zoom_y != 0);
    f32 inv_scale_x = 1.0f / zoom_x;
    f32 inv_scale_y = 1.0f / zoom_y;

    s32 min_x = RoundReal32ToInt32(min.x);
    s32 min_y = RoundReal32ToInt32(min.y);
    s32 max_x = RoundReal32ToInt32(min.x + zoom_x * bitmap->width);
    s32 max_y = RoundReal32ToInt32(min.y + zoom_y * bitmap->height);

    if (min_x >= buffer->width || min_y >= buffer->height || max_x < 0 || max_y < 0) {
        return;
    }

    if (max_y > buffer->height) {
        max_y = buffer->height;
    }

    if (max_x > buffer->width) {
        max_x = buffer->width;
    }

    s32 start_y = min_y;
    s32 start_x = min_x;

    min_x = 0;
    min_y = 0;

    if (start_y < 0) {
        min_y = -start_y;
        start_y = 0;
    }

    if (start_x < 0) {
        min_x = -start_x;
        start_x = 0;
    }

    u32 *src_row = (u32 *)bitmap->pixels + bitmap->width * (bitmap->height - 1);

    u32 *dest_row = (u32 *)buffer->memory + start_x + start_y * buffer->width;

    for (s32 y = start_y; y < max_y; ++y) {

        u32 *src_pixel = src_row - (s32)((y - min_y - start_y) * inv_scale_y) * bitmap->width;

        u32 *dest_pixel = dest_row;

        for (s32 x = start_x; x < max_x; ++x) {
            s32 src_x = (s32)((x + min_x - start_x) * inv_scale_x);

            u32 src_color = *(src_pixel + src_x);
            f32 a = (f32)((src_color >> 24) & 0xff) / 255.0f;

            if (a == 1.0f) {
                *dest_pixel = src_color;
            } else if (a > 0.0f) {
                f32 sr = (f32)((src_color >> 16) & 0xff);
                f32 sg = (f32)((src_color >> 8) & 0xff);
                f32 sb = (f32)((src_color >> 0) & 0xff);

                f32 dr = (f32)((*dest_pixel >> 16) & 0xff);
                f32 dg = (f32)((*dest_pixel >> 8) & 0xff);
                f32 db = (f32)((*dest_pixel >> 0) & 0xff);

                f32 r = (1.0f - a) * dr + a * sr;
                f32 g = (1.0f - a) * dg + a * sg;
                f32 b = (1.0f - a) * db + a * sb;

                *dest_pixel =
                    (((u32)(r + 0.5f) << 16) |
                     ((u32)(g + 0.5f) << 8) |
                     ((u32)(b + 0.5f) << 0));
            }
            dest_pixel++;
        }

        dest_row += buffer->width;
    }
}

function void
DrawBitmapStretch(RenderBuffer *buffer, Bitmap *bitmap, v2 min, v2 max) {
    min.x *= buffer->width;
    min.y *= buffer->height;

    max.x *= buffer->width;
    max.y *= buffer->height;

    DrawBitmapStretchInPixels(buffer, bitmap, min, max);
}

function void
DrawBitmapZoomLightnessInPixels(RenderBuffer *buffer, Bitmap *bitmap, v2 min, f32 zoom, f32 lightness) {
    Assert(bitmap);

    Assert(zoom != 0);
    f32 inv_scale = 1.0f / zoom;

    s32 min_x = RoundReal32ToInt32(min.x);
    s32 min_y = RoundReal32ToInt32(min.y);
    s32 max_x = RoundReal32ToInt32(min.x + zoom * bitmap->width);
    s32 max_y = RoundReal32ToInt32(min.y + zoom * bitmap->height);

    if (min_x >= buffer->width || min_y >= buffer->height || max_x < 0 || max_y < 0) {
        return;
    }

    if (max_y > buffer->height) {
        max_y = buffer->height;
    }

    if (max_x > buffer->width) {
        max_x = buffer->width;
    }

    s32 start_y = min_y;
    s32 start_x = min_x;
    min_x = 0;
    min_y = 0;

    if (start_y < 0) {
        min_y = -start_y;
        start_y = 0;
    }

    if (start_x < 0) {
        min_x = -start_x;
        start_x = 0;
    }

    u32 *src_row = (u32 *)bitmap->pixels + bitmap->width * (bitmap->height - 1);

    u32 *dest_row = (u32 *)buffer->memory + start_x + start_y * buffer->width;

    for (s32 y = start_y; y < max_y; ++y) {

        u32 *src_pixel = src_row - (s32)((y + min_y - start_y) * inv_scale) * bitmap->width;

        u32 *dest_pixel = dest_row;

        for (s32 x = start_x; x < max_x; ++x) {
            s32 src_x = (s32)((x + min_x - start_x) * inv_scale);

            u32 src_color = *(src_pixel + src_x);
            f32 a = (f32)((src_color >> 24) & 0xff) / 255.0f;

            f32 sr = (f32)((src_color >> 16) & 0xff);
            f32 sg = (f32)((src_color >> 8) & 0xff);
            f32 sb = (f32)((src_color >> 0) & 0xff);

            sr *= lightness;
            sg *= lightness;
            sb *= lightness;

            sr = Clamp(0, sr, 255);
            sg = Clamp(0, sg, 255);
            sb = Clamp(0, sb, 255);

            if (a == 1.0f) {
                *dest_pixel = (
                    (RoundReal32ToInt32(sr)) << 16 |
                    (RoundReal32ToInt32(sg) << 8) |
                    (RoundReal32ToInt32(sb) << 0));
            } else if (a != 0.0f) {
                f32 dr = (f32)((*dest_pixel >> 16) & 0xff);
                f32 dg = (f32)((*dest_pixel >> 8) & 0xff);
                f32 db = (f32)((*dest_pixel >> 0) & 0xff);

                f32 r = (1.0f - a) * dr + a * sr;
                f32 g = (1.0f - a) * dg + a * sg;
                f32 b = (1.0f - a) * db + a * sb;

                *dest_pixel =
                    (((u32)(r + 0.5f) << 16) |
                     ((u32)(g + 0.5f) << 8) |
                     ((u32)(b + 0.5f) << 0));
            }
            dest_pixel++;
        }
        dest_row += buffer->width;
    }
}

function void
DrawThickLineInPixels(RenderBuffer *buffer, v2 min, v2 max, s32 thickness, v4 color) {

    color = V4MulF32(color, 255.0f);

    s32 x0 = RoundReal32ToInt32(min.x);
    s32 y0 = RoundReal32ToInt32(min.y);
    s32 x1 = RoundReal32ToInt32(max.x);
    s32 y1 = RoundReal32ToInt32(max.y);

    s32 dx = Abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    s32 dy = Abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    s32 err = (dx > dy ? dx : -dy) / 2;

    s32 half_thickness = thickness / 2;

    while (1) {

        for (s32 i = -half_thickness; i <= half_thickness; i++) {
            for (s32 j = -half_thickness; j <= half_thickness; j++) {
                PlotPixelAlpha(buffer, x0 + i, y0 + j, color);
            }
        }

        if (x0 == x1 && y0 == y1) {
            break;
        }

        s32 e2 = err;
        if (e2 > -dx) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dy) {
            err += dx;
            y0 += sy;
        }
    }
}

function void
DrawBorderAroundRectInPixels(RenderBuffer *buffer, v2 min, v2 max, v4 color, u32 thickness) {
    v2 top_left = min;
    v2 top_right = V2(max.x, min.y);
    v2 bottom_left = V2(min.x, max.y);
    v2 bottom_right = max;

    DrawThickLineInPixels(buffer, top_left, top_right, thickness, color);
    DrawThickLineInPixels(buffer, top_left, bottom_left, thickness, color);
    DrawThickLineInPixels(buffer, bottom_left, bottom_right, thickness, color);
    DrawThickLineInPixels(buffer, top_right, bottom_right, thickness, color);
}

function void
DrawBorderAroundRect(RenderBuffer *buffer, v2 min, v2 max, v4 color, u32 thickness) {
    min.x *= buffer->width;
    min.y *= buffer->height;

    max.x *= buffer->width;
    max.y *= buffer->height;

    DrawBorderAroundRectInPixels(buffer, min, max, color, thickness);
}

function inline b32
IsNum(char c) {
    return c >= '0' && c <= '9';
}

function void
DrawTextInPixels(RenderBuffer *buffer, char *text, f32 text_size, v2 min, v4 color) {
    s32 x0 = RoundReal32ToInt32(min.x);
    s32 y0 = RoundReal32ToInt32(min.y);
    f32 curr_x = (f32)x0;
    f32 curr_y = (f32)y0;
    u32 text_length = StringLength(text);
    for (u32 i = 0; i < text_length; ++i) {
        char letter = text[i];
        b32 draw_lower = false;

        if (text[i] == ' ') {
            curr_x += text_size * 10;
            continue;
        } else {
            letter = text[i];
        }

        if (letter == 'p' || letter == 'q' || letter == 'y' || letter == 'g') {
            draw_lower = true;
        }

        if (draw_lower) {
            for (u32 y = 0; y < 8; ++y) {
                for (u32 x = 0; x < 7; ++x) {
                    if (ascii_table[letter][y * 7 + x] == '0') {
                        u32 old_y = y;
                        y += 3;
                        DrawRectAlphaInPixels(buffer,
                                              V2(curr_x + x * text_size,
                                                 curr_y + y * text_size),
                                              V2(curr_x + text_size + x * text_size,
                                                 curr_y + text_size + y * text_size),
                                              color);

                        DrawRectAlphaInPixels(buffer,
                                              V2(curr_x + (x + 1) * text_size,
                                                 curr_y + (y + 1) * text_size),
                                              V2(curr_x + text_size + (x + 1) * text_size,
                                                 curr_y + text_size + (y + 1) * text_size),
                                              V4(0, 0, 0, color.a));
                        y = old_y;
                    }
                }
            }
        } else {
            for (u32 y = 0; y < 8; ++y) {
                for (u32 x = 0; x < 7; ++x) {
                    if (ascii_table[letter][y * 7 + x] == '0') {
                        DrawRectAlphaInPixels(buffer,
                                              V2(curr_x + x * text_size,
                                                 curr_y + y * text_size),
                                              V2(curr_x + text_size + x * text_size,
                                                 curr_y + text_size + y * text_size),
                                              color);

                        DrawRectAlphaInPixels(buffer,
                                              V2(curr_x + (x + 1) * text_size,
                                                 curr_y + (y + 1) * text_size),
                                              V2(curr_x + text_size + (x + 1) * text_size,
                                                 curr_y + text_size + (y + 1) * text_size),
                                              V4(0, 0, 0, color.a));
                    }
                }
            }
        }


        curr_x += text_size * 10;
    }

}

function void
DrawText(RenderBuffer *buffer, char *text, f32 text_size, v2 min, v4 color) {
    min.x *= buffer->width;
    min.y *= buffer->height;
    DrawTextInPixels(buffer, text, text_size, min, color);
}