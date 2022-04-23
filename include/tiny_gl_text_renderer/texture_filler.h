#pragma once

#include <cstring>

#include "font_table.h"

namespace tiny_gl_text_renderer {

constexpr int CHAR_WIDTH = 16;
constexpr int CHAR_HEIGHT = 18;
constexpr int OFFSET = 2;

/**
    The target 'texture' must be of the following structure:
    pixels are organized line by line, from top to bottom, from left to right,
    each line is 'texture_w' pixels wide,
    each pixel is 'texture_stride' floats wide.
    The coordinates (x,y) define the position of the top-left corner of the
    character in the texture. These coordinates can be negative - in this case
    not fitting parts will be cropped. The function will return 1.
    The source 'color' pointer must point to an array of at least
    'n_components' floats which are used to change the values of
    the pixels of the target texture.
    In a normal case 'texture_stride' >= 'n_components'
    In a very normal case you migh be using RGBA texture and
    'texture_stride' = 4
    'n_components' = 4 (or maybe 3 if you want to ignore the alpha channel)
    'texture_h' used only to check that the texture possesses enough
    area to draw the character.
    Return 0 is everything went well.
    Returns 1 is something went wrong. Should not break however,
    if something can be printed, it will be printed.
    If the line does not fit, it will be cropped and 1 will be returned.
*/
static
int FillCharacter(
    const char character,
    float* texture,
    const size_t texture_stride,
    const size_t texture_w,
    const size_t texture_h,
    const signed long long x,
    const signed long long y,
    const float* const color,
    const size_t n_components)
{
    int ret_val = 0;
    const signed long long top_left_idx = y * texture_w + x;
    const unsigned short int* lut = GetLUT(character);
    if (lut == nullptr) return 1; // Character not supported
    for (signed long long yl = 0; yl < CHAR_HEIGHT; yl++) {
        if ((y + yl) < 0 || (y + yl) >= (signed long long)texture_h) { ret_val = 1; continue; }
        const signed long long cur_line_start_idx = top_left_idx + yl * texture_w;
        const unsigned short int& sublut = lut[yl + OFFSET];
        for (signed long long xl = 0; xl < CHAR_WIDTH; xl++) {
            if ((x + xl) < 0 || (x + xl) >= (signed long long)texture_w) { ret_val = 1; continue; }
            const signed long long i = cur_line_start_idx + xl;
            const unsigned short mask = (0x1 << (CHAR_WIDTH - 1 - xl));
            const unsigned short int f = ((sublut & mask) >> (CHAR_WIDTH - 1 - xl));
            for (size_t comp = 0; comp < n_components; comp++) {
                texture[i * texture_stride + comp] = f * color[comp];
            }
        }
    }
    return ret_val;
}

static
int FillString(
    const char* line,
    float* texture,
    const size_t texture_stride,
    const size_t texture_w,
    const size_t texture_h,
    const size_t x,
    const size_t y,
    const float* const color,
    const size_t n_components)
{
    int ret_val = 0;
    const size_t nch = strlen(line);
    size_t xl = x;
    size_t yl = y;
    for (size_t ch = 0; ch < nch; ch++) {
        if (line[ch] == '\n') {
            xl = x;
            yl += CHAR_HEIGHT;
            continue;
        }
        if (FillCharacter(line[ch], texture, texture_stride,
            texture_w, texture_h, xl, yl, color, n_components) == 1) {
            ret_val = 1;
        }
        xl += CHAR_WIDTH;
        if (xl > texture_w) return 1;
    }
    return ret_val;
}

/**
    Return the number of pixels required for the given string.
    After that the returned value should be multiplied by the
    number of color components (or more precisely, texture_stride),
    which is commonly 4, and depending on the way you allocate memory
    for the texture, also multiplied by sizeof(datatype), i.e. sizeof(float).
    The two arguments 'o_w' and 'o_h' are the references which will be
    filled with the required width and height of the texture.
    In fact, the returned value is o_w*o_h;
*/
static
size_t GetRequiredTextureSize(const char* line, size_t& o_w, size_t& o_h)
{
    o_h = 0;
    o_w = 0;
    int cursor = 0;
    const size_t nch = strlen(line);
    for (size_t i = 0; i < nch; i++) {
        o_w = (cursor > o_w) ? cursor : o_w;
        cursor++;
        if (line[i] == '\n') {
            o_h++;
            cursor = 0;
        }
    }
    o_w++; //TODO check
    if (line[nch - 1] != '\n') o_h++;
    o_w *= CHAR_WIDTH;
    o_h *= CHAR_HEIGHT;
    return o_w * o_h;
}

} // end of namespace tiny_gl_text_renderer
