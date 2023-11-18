function s32
TextLength(char *text) {
    BEGIN_TIMED_FUNCTION();
    s32 length = 0;
    if (text) {
        char ch = *text;
        while (ch) {
            if (*text == ' ') {
                length += 10;
            } else {
                length += g_font->glyphs[*text].bitmap.width + 1;
            }
            text++;
            ch = *text;
        }
    }

    END_TIMED_FUNCTION();

    return length;
}

#if 0
function v2
AlignTextInRect(char *text, f32 text_size, Rect dest, UI_AlignmentFlag align) {
    v2 result = { 0 };
    f32 src_width = (f32)TextLength(text);
    f32 src_height = text_size * 8.0f;

    f32 dest_width = dest.max.x - dest.min.x;
    f32 dest_height = dest.max.y - dest.min.y;

    f32 offset_x = 0;
    f32 offset_y = 0;

    if (align & UI_AlignmentFlag_Center) {

        offset_x = (dest_width - src_width) / 2.0f;
        offset_y = (dest_height - src_height) / 2.0f;

    } else if (align & UI_AlignmentFlag_Left) {
        offset_x = 0;
        offset_y = (dest_height - src_height) / 2.0f;
    } else if (align & UI_AlignmentFlag_Right) {
        // TODO(hampus): This should actually offset the end of the text 
        // to the right, not the beginning
        offset_x = dest_width;
        offset_y = (dest_height - src_height) / 2.0f;
    }

    result = V2(offset_x, offset_y);
    result = V2AddV2(result, dest.min);

    return result;
}
#endif

function v2 GetCenterAlignedPosOfText(char *text, f32 text_size, Rect dest) {
    v2 result = { 0 };
    f32 src_width = (f32)TextLength(text);
    f32 src_height = text_size * 8.0f;

    f32 dest_width = dest.max.x - dest.min.x;
    f32 dest_height = dest.max.y - dest.min.y;

    f32 offset_x = (dest_width - src_width) / 2.0f;
    f32 offset_y = (dest_height - src_height) / 2.0f;

    result = V2(offset_x, offset_y);
    result = V2AddV2(result, dest.min);

    return result;
}

function inline u32 Unpack4x8(v4 a) {
    u32 result;

    result =
        ((RoundReal32ToInt32(a.a) << 24) |
         (RoundReal32ToInt32(a.r) << 16) |
         (RoundReal32ToInt32(a.g) << 8) |
         (RoundReal32ToInt32(a.b)));

    return result;
}

// @Speed: SIMD    
function inline v4 Pack4x8(u32 value) {
    v4 result;

    result.a = (f32)((value >> (8 * 3)) & 0xff);
    result.r = (f32)((value >> (8 * 2)) & 0xff);
    result.g = (f32)((value >> (8 * 1)) & 0xff);
    result.b = (f32)((value >> (8 * 0)) & 0xff);

    return result;

}

function void ClearScreen(RenderBuffer *buffer, v4 color) {
    u32 color_unpacked = Unpack4x8(V4MulF32(color, 255.0f));
    u32 *pixels = (u32 *)buffer->memory;
    for (s32 i = 0; i < buffer->height * buffer->width; ++i) {
        *pixels++ = color_unpacked;
    }
}

// @Speed: SIMD
function void DrawRectAlpha(RenderBuffer *buffer, v2 min, v2 max, v4 color) {

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

global Rect g_clip_rect;
// @Speed: SIMD
function void DrawRectAlphaClipped(RenderBuffer *buffer, v2 min, v2 max, v4 color) {
    BEGIN_TIMED_FUNCTION();
    s32 min_x = RoundReal32ToInt32(g_clip_rect.min.x);
    s32 min_y = RoundReal32ToInt32(g_clip_rect.min.y);
    s32 max_x = RoundReal32ToInt32(g_clip_rect.max.x);
    s32 max_y = RoundReal32ToInt32(g_clip_rect.max.y);

    s32 x0 = RoundReal32ToInt32(min.x);
    s32 y0 = RoundReal32ToInt32(min.y);
    s32 x1 = RoundReal32ToInt32(max.x);
    s32 y1 = RoundReal32ToInt32(max.y);

    if (x0 < min_x) x0 = min_x;
    if (y0 < min_y) y0 = min_y;
    if (x1 > max_x) x1 = max_x;
    if (y1 > max_y) y1 = max_y;

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
    END_TIMED_FUNCTION();
}

function void DrawText(RenderBuffer *buffer, char *text, f32 text_size, v2 min, v4 color) {
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
                        DrawRectAlpha(buffer,
                                      V2(curr_x + x * text_size,
                                         curr_y + y * text_size),
                                      V2(curr_x + text_size + x * text_size,
                                         curr_y + text_size + y * text_size),
                                      color);

                        y = old_y;
                    }
                }
            }
        } else {
            for (u32 y = 0; y < 8; ++y) {
                for (u32 x = 0; x < 7; ++x) {
                    if (ascii_table[letter][y * 7 + x] == '0') {
                        DrawRectAlpha(buffer,
                                      V2(curr_x + x * text_size,
                                         curr_y + y * text_size),
                                      V2(curr_x + text_size + x * text_size,
                                         curr_y + text_size + y * text_size),
                                      color);

                    }
                }
            }
        }


        curr_x += text_size * 8.0f;
    }

}

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
            __m256 rrf = _mm256_mul_ps(saf, srf);
            rrf = _mm256_add_ps(rrf, _mm256_mul_ps(_mm256_sub_ps(_mm256_set1_ps(1.0), saf), drf));

            __m256 rgf = _mm256_mul_ps(saf, sgf);
            rgf = _mm256_add_ps(rgf, _mm256_mul_ps(_mm256_sub_ps(_mm256_set1_ps(1.0), saf), dgf));

            __m256 rbf = _mm256_mul_ps(saf, sbf);
            rbf = _mm256_add_ps(rbf, _mm256_mul_ps(_mm256_sub_ps(_mm256_set1_ps(1.0), saf), dbf));

            // store back to 0-255 srgb
            __m256i rr = _mm256_slli_epi32(_mm256_cvtps_epi32(rrf), 16);
            __m256i rg = _mm256_slli_epi32(_mm256_cvtps_epi32(rgf), 8);
            __m256i rb = _mm256_cvtps_epi32(rbf);

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

function void
DrawBitmapInPixelsClipped(RenderBuffer *buffer, Bitmap *bitmap, v2 min) {

    Assert(bitmap);
    s32 min_x = RoundReal32ToInt32(min.x);
    s32 min_y = RoundReal32ToInt32(min.y);
    s32 max_x = RoundReal32ToInt32(min.x + (f32)bitmap->width);
    s32 max_y = RoundReal32ToInt32(min.y + (f32)bitmap->height);

    if (min_x >= g_clip_rect.max.x ||
        min_y >= g_clip_rect.max.y ||
        max_x < g_clip_rect.min.x ||
        max_y < g_clip_rect.min.y) {
        return;
    }

    s32 src_offset_x = 0;
    if (min_x < (g_clip_rect.min.x - min_x)) {
        src_offset_x = -(s32)(g_clip_rect.min.x - min_x);
        min_x = (s32)g_clip_rect.min.x - min_x;
    }

    s32 src_offset_y = 0;
    if (min_y < (g_clip_rect.min.y - min_y)) {
        src_offset_y = -(s32)(g_clip_rect.min.y - min_y);
        min_y = (s32)g_clip_rect.min.y - min_y;
    }

    if (max_x > g_clip_rect.max.x) {
        max_x = (s32)g_clip_rect.max.x;
    }

    if (max_y > g_clip_rect.max.y) {
        max_y = (s32)g_clip_rect.max.y;
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
            __m256 rrf = _mm256_mul_ps(saf, srf);
            rrf = _mm256_add_ps(rrf, _mm256_mul_ps(_mm256_sub_ps(_mm256_set1_ps(1.0), saf), drf));

            __m256 rgf = _mm256_mul_ps(saf, sgf);
            rgf = _mm256_add_ps(rgf, _mm256_mul_ps(_mm256_sub_ps(_mm256_set1_ps(1.0), saf), dgf));

            __m256 rbf = _mm256_mul_ps(saf, sbf);
            rbf = _mm256_add_ps(rbf, _mm256_mul_ps(_mm256_sub_ps(_mm256_set1_ps(1.0), saf), dbf));

            // store back to 0-255 srgb
            __m256i rr = _mm256_slli_epi32(_mm256_cvtps_epi32(rrf), 16);
            __m256i rg = _mm256_slli_epi32(_mm256_cvtps_epi32(rgf), 8);
            __m256i rb = _mm256_cvtps_epi32(rbf);

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

function void DrawTextClipped(RenderBuffer *buffer, char *text, f32 text_size, v2 min, v4 color) {
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
            DrawBitmapInPixelsClipped(buffer, &g_font->glyphs[letter].bitmap, V2(curr_x, curr_y + 3));

        } else {
            DrawBitmapInPixelsClipped(buffer, &g_font->glyphs[letter].bitmap, V2(curr_x, curr_y));
        }

        s32 spacing = 1;
        curr_x += text_size * g_font->glyphs[letter].bitmap.width + spacing * text_size;
    }

}

function inline void PlotPixelAlpha(RenderBuffer *buffer, s32 x, s32 y, v4 color) {
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

function void DrawThickLineInPixels(RenderBuffer *buffer, v2 min, v2 max, s32 thickness, v4 color) {

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

function void DrawBorderAroundRectInPixels(RenderBuffer *buffer, v2 min, v2 max, v4 color, u32 thickness) {
    v2 top_left = min;
    v2 top_right = V2(max.x, min.y);
    v2 bottom_left = V2(min.x, max.y);
    v2 bottom_right = max;

    DrawThickLineInPixels(buffer, top_left, top_right, thickness, color);
    DrawThickLineInPixels(buffer, top_left, bottom_left, thickness, color);
    DrawThickLineInPixels(buffer, bottom_left, bottom_right, thickness, color);
    DrawThickLineInPixels(buffer, top_right, bottom_right, thickness, color);
}

function f32 Length(v2 v) {
    f32 result;
    result = sqrtf(Square(v.x) + Square(v.y));
    return result;
}

function v2 abs_v2(v2 v) {
    v2 result;

    result.x = Abs(v.x);
    result.y = Abs(v.y);

    return result;
}

function v2 max_v2(v2 a, v2 b) {
    v2 result;

    result.x = Max(a.x, b.x);
    result.y = Max(a.y, b.y);

    return result;
}

function f32 sd_circle(v2 p, f32 r) {
    f32 result;
    result = Length(p) - r;
    return result;
}

function f32 sd_NewRect(v2 p, v2 b, v4 r) {

    if (p.x <= 0.0f) {
        r.x = r.z;
        r.y = r.w;
    }

    if (p.y <= 0.0f) {
        r.x = r.y;
    }

    v2 q = V2SubV2(abs_v2(p), b);
    q.x += r.x;
    q.y += r.x;
    f32 result = Min(Max(q.x, q.y), 0.0f);
    return result + Length(max_v2(q, V2(0.0, 0.0))) - r.x;
}

float smoothstep(float edge0, float edge1, float x) {
    if (x < edge0)
        return 0;

    if (x >= edge1)
        return 1;

    // Scale/bias into [0..1] range
    x = (x - edge0) / (edge1 - edge0);

    return x * x * (3 - 2 * x);
}

v4 smoothstep2(v4 edge0, v4 edge1, float x) {
    v4 result;

    result.r = smoothstep(edge0.r, edge1.r, x);
    result.g = smoothstep(edge0.g, edge1.g, x);
    result.b = smoothstep(edge0.b, edge1.b, x);
    result.a = smoothstep(edge0.a, edge1.a, x);

    return result;
}

inline __m128 clamp_sse(__m128 a, __m128 val, __m128 b) {
    return _mm_max_ps(a, _mm_min_ps(b, val));
}

inline __m128 smoothstep_sse(__m128 edge0, __m128 edge1, __m128 x) {

    x = clamp_sse(edge0, x, edge1);

    // x = (x - edge0) / (edge1 - edge0);
    x = _mm_div_ps(_mm_sub_ps(x, edge0), _mm_sub_ps(edge1, edge0));

    // x =  x * x;
    x = _mm_mul_ps(x, x);

    // x = x * (3 - 2 * x)
    x = _mm_mul_ps(x, _mm_sub_ps(_mm_set_ps1(3), _mm_mul_ps(_mm_set_ps1(2), x)));

    return x;
}

inline __m128 lerp_sse(__m128 edge0, __m128 edge1, __m128 x) {

    x = clamp_sse(_mm_set_ps1(0.0f), x, _mm_set_ps1(1.0f));
    __m128 result;

    __m128 l = _mm_mul_ps(edge0, _mm_sub_ps(_mm_set_ps1(1.0f), x));
    __m128 r = _mm_mul_ps(edge1, x);

    result = _mm_add_ps(l, r);

    return result;
}

# define M_LN2          0.69314718055994530942
inline __m128 exp_sse(__m128 x) {
    x.m128_f32[0] = expf(x.m128_f32[0]);
    x.m128_f32[1] = expf(x.m128_f32[1]);
    x.m128_f32[2] = expf(x.m128_f32[2]);
    x.m128_f32[3] = expf(x.m128_f32[3]);
    return x;
}

inline __m256 clamp_avx(__m256 a, __m256 val, __m256 b) {
    return _mm256_max_ps(a, _mm256_min_ps(b, val));
}

inline __m256 smoothstep_avx(__m256 edge0, __m256 edge1, __m256 x) {

    x = clamp_avx(edge0, x, edge1);

    // x = (x - edge0) / (edge1 - edge0);
    x = _mm256_div_ps(_mm256_sub_ps(x, edge0), _mm256_sub_ps(edge1, edge0));

    // x =  x * x;
    x = _mm256_mul_ps(x, x);

    // x = x * (3 - 2 * x)
    x = _mm256_mul_ps(x, _mm256_sub_ps(_mm256_set1_ps(3), _mm256_mul_ps(_mm256_set1_ps(2), x)));

    return x;
}

inline __m256 lerp_avx(__m256 edge0, __m256 edge1, __m256 x) {

    x = clamp_avx(_mm256_set1_ps(0.0f), x, _mm256_set1_ps(1.0f));
    __m256 result;

    __m256 l = _mm256_mul_ps(edge0, _mm256_sub_ps(_mm256_set1_ps(1.0f), x));
    __m256 r = _mm256_mul_ps(edge1, x);

    result = _mm256_add_ps(l, r);

    return result;
}

function void DrawRoundedRect2InPixels(RenderBuffer *buffer, v2 min, v2 max, f32 r, v4 color) {
    s32 min_x = (s32)roundf(min.x);
    s32 min_y = (s32)roundf(min.y);
    s32 max_x = (s32)roundf(max.x);
    s32 max_y = (s32)roundf(max.y);

    u32 *dest_row = (u32 *)buffer->memory + min_x + min_y * buffer->width;

    v2 half_size = V2SubV2(max, min);
    half_size.x /= 2;
    half_size.y /= 2;

    s32 dx = max_x - min_x;
    s32 dy = max_y - min_y;

    s32 start_y = -dy / 2;
    s32 end_y = dy / 2;

    s32 start_x = -dx / 2;
    s32 end_x = dx / 2;

    u32 color_unpacked = Unpack4x8(V4MulF32(color, 255));

    for (s32 y = start_y; y < end_y; ++y) {
        u32 *dest_pixel = dest_row;
        for (s32 x = start_x; x < end_x; ++x) {
            v2 p = V2((f32)x, (f32)y);

            f32 d = sd_NewRect(p, half_size, V4(r, r, r, r));

            if (d < 0.0f) {
                *dest_pixel = color_unpacked;
            }

            dest_pixel++;
        }
        dest_row += buffer->width;
    }
}