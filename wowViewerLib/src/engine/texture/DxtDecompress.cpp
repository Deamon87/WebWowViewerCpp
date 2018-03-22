//
// Created by Deamon on 3/22/2018.
//

#include "DxtDecompress.h"

#include <stdint.h>
#include <stddef.h>
#include <assert.h>

/*
DXT1/DXT3/DXT5 texture decompression

The original code is from Benjamin Dobell, see below for details. Compared to
the original the code is now valid C89, has support for 64-bit architectures
and has been refactored. It also has support for additional formats and uses
a different PackRGBA order.

---

Copyright (c) 2012 - 2015 Matthäus G. "Anteru" Chajdas (http://anteru.net)

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

---

Copyright (C) 2009 Benjamin Dobell, Glass Echidna

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

---
*/
static uint32_t PackRGBA (uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    return r | (g << 8) | (b << 16) | (a << 24);
}

static float Int8ToFloat_SNORM (const uint8_t input)
{
    return (float)((int8_t)input) / 127.0f;
}

static float Int8ToFloat_UNORM (const uint8_t input)
{
    return (float)input / 255.0f;
}

/**
Decompress a BC 16x3 index block stored as
h g f e
d c b a
p o n m
l k j i

Bits packed as

| h | g | f | e | d | c | b | a | // Entry
|765 432 107 654 321 076 543 210| // Bit
|0000000000111111111112222222222| // Byte

into 16 8-bit indices.
*/
static void Decompress16x3bitIndices (const uint8_t* packed, uint8_t* unpacked)
{
    uint32_t tmp, block, i;

    for (block = 0; block < 2; ++block) {
        tmp = 0;

        // Read three bytes
        for (i = 0; i < 3; ++i) {
            tmp |= ((uint32_t)packed [i]) << (i * 8);
        }

        // Unpack 8x3 bit from last 3 byte block
        for (i = 0; i < 8; ++i) {
            unpacked [i] = (tmp >> (i*3)) & 0x7;
        }

        packed += 3;
        unpacked += 8;
    }
}

static void DecompressBlockBC1Internal (const uint8_t* block,
                                        unsigned char* output, uint32_t outputStride, const uint8_t* alphaValues)
{
    uint32_t temp, code;

    uint16_t color0, color1;
    uint8_t r0, g0, b0, r1, g1, b1;

    int i, j;

    color0 = *(const uint16_t*)(block);
    color1 = *(const uint16_t*)(block + 2);

    temp = (color0 >> 11) * 255 + 16;
    r0 = (uint8_t)((temp/32 + temp)/32);
    temp = ((color0 & 0x07E0) >> 5) * 255 + 32;
    g0 = (uint8_t)((temp/64 + temp)/64);
    temp = (color0 & 0x001F) * 255 + 16;
    b0 = (uint8_t)((temp/32 + temp)/32);

    temp = (color1 >> 11) * 255 + 16;
    r1 = (uint8_t)((temp/32 + temp)/32);
    temp = ((color1 & 0x07E0) >> 5) * 255 + 32;
    g1 = (uint8_t)((temp/64 + temp)/64);
    temp = (color1 & 0x001F) * 255 + 16;
    b1 = (uint8_t)((temp/32 + temp)/32);

    code = *(const uint32_t*)(block + 4);

    if (color0 > color1) {
        for (j = 0; j < 4; ++j) {
            for (i = 0; i < 4; ++i) {
                uint32_t finalColor, positionCode;
                uint8_t alpha;

                alpha = alphaValues [j*4+i];

                finalColor = 0;
                positionCode = (code >>  2*(4*j+i)) & 0x03;

                switch (positionCode) {
                    case 0:
                        finalColor = PackRGBA(r0, g0, b0, alpha);
                        break;
                    case 1:
                        finalColor = PackRGBA(r1, g1, b1, alpha);
                        break;
                    case 2:
                        finalColor = PackRGBA((2*r0+r1)/3, (2*g0+g1)/3, (2*b0+b1)/3, alpha);
                        break;
                    case 3:
                        finalColor = PackRGBA((r0+2*r1)/3, (g0+2*g1)/3, (b0+2*b1)/3, alpha);
                        break;
                }

                *(uint32_t*)(output + j*outputStride + i * sizeof (uint32_t)) = finalColor;
            }
        }
    } else {
        for (j = 0; j < 4; ++j) {
            for (i = 0; i < 4; ++i) {
                uint32_t finalColor, positionCode;
                uint8_t alpha;

                alpha = alphaValues [j*4+i];

                finalColor = 0;
                positionCode = (code >>  2*(4*j+i)) & 0x03;

                switch (positionCode) {
                    case 0:
                        finalColor = PackRGBA(r0, g0, b0, alpha);
                        break;
                    case 1:
                        finalColor = PackRGBA(r1, g1, b1, alpha);
                        break;
                    case 2:
                        finalColor = PackRGBA((r0+r1)/2, (g0+g1)/2, (b0+b1)/2, alpha);
                        break;
                    case 3:
                        finalColor = PackRGBA(0, 0, 0, alpha);
                        break;
                }

                *(uint32_t*)(output + j*outputStride + i * sizeof (uint32_t)) = finalColor;
            }
        }
    }
}

/*
Decompresses one block of a BC1 (DXT1) texture and stores the resulting pixels at the appropriate offset in 'image'.

uint32_t x:						x-coordinate of the first pixel in the block.
uint32_t y:						y-coordinate of the first pixel in the block.
uint32_t stride:				stride of a scanline in bytes.
const uint8_t* blockStorage:	pointer to the block to decompress.
uint32_t* image:				pointer to image where the decompressed pixel data should be stored.
*/
void DecompressBlockBC1 (uint32_t x, uint32_t y, uint32_t stride,
                         const uint8_t* blockStorage, unsigned char* image)
{
    static const uint8_t const_alpha [] = {
        255, 255, 255, 255,
        255, 255, 255, 255,
        255, 255, 255, 255,
        255, 255, 255, 255
    };

    DecompressBlockBC1Internal (blockStorage,
                                image + x * sizeof (uint32_t) + (y * stride), stride, const_alpha);
}

/*
Decompresses one block of a BC3 (DXT5) texture and stores the resulting pixels at the appropriate offset in 'image'.

uint32_t x:						x-coordinate of the first pixel in the block.
uint32_t y:						y-coordinate of the first pixel in the block.
uint32_t stride:				stride of a scanline in bytes.
const uint8_t *blockStorage:	pointer to the block to decompress.
uint32_t *image:				pointer to image where the decompressed pixel data should be stored.
*/
void DecompressBlockBC3 (uint32_t x, uint32_t y, uint32_t stride,
                         const uint8_t* blockStorage, unsigned char* image)
{
    uint8_t alpha0, alpha1;
    uint8_t alphaIndices [16];

    uint16_t color0, color1;
    uint8_t r0, g0, b0, r1, g1, b1;

    int i, j;

    uint32_t temp, code;

    alpha0 = *(blockStorage);
    alpha1 = *(blockStorage + 1);

    Decompress16x3bitIndices (blockStorage + 2, alphaIndices);

    color0 = *(const uint16_t*)(blockStorage + 8);
    color1 = *(const uint16_t*)(blockStorage + 10);

    temp = (color0 >> 11) * 255 + 16;
    r0 = (uint8_t)((temp / 32 + temp) / 32);
    temp = ((color0 & 0x07E0) >> 5) * 255 + 32;
    g0 = (uint8_t)((temp / 64 + temp) / 64);
    temp = (color0 & 0x001F) * 255 + 16;
    b0 = (uint8_t)((temp / 32 + temp) / 32);

    temp = (color1 >> 11) * 255 + 16;
    r1 = (uint8_t)((temp / 32 + temp) / 32);
    temp = ((color1 & 0x07E0) >> 5) * 255 + 32;
    g1 = (uint8_t)((temp / 64 + temp) / 64);
    temp = (color1 & 0x001F) * 255 + 16;
    b1 = (uint8_t)((temp / 32 + temp) / 32);

    code = *(const uint32_t*)(blockStorage + 12);

    for (j = 0; j < 4; j++) {
        for (i = 0; i < 4; i++) {
            uint8_t finalAlpha;
            int alphaCode;
            uint8_t colorCode;
            uint32_t finalColor;

            alphaCode = alphaIndices [4 * j + i];

            if (alphaCode == 0) {
                finalAlpha = alpha0;
            } else if (alphaCode == 1) {
                finalAlpha = alpha1;
            } else {
                if (alpha0 > alpha1) {
                    finalAlpha = (uint8_t)(((8 - alphaCode)*alpha0 + (alphaCode - 1)*alpha1) / 7);
                } else {
                    if (alphaCode == 6) {
                        finalAlpha = 0;
                    } else if (alphaCode == 7) {
                        finalAlpha = 255;
                    } else {
                        finalAlpha = (uint8_t)(((6 - alphaCode)*alpha0 + (alphaCode - 1)*alpha1) / 5);
                    }
                }
            }

            colorCode = (code >> 2 * (4 * j + i)) & 0x03;
            finalColor = 0;

            switch (colorCode) {
                case 0:
                    finalColor = PackRGBA (r0, g0, b0, finalAlpha);
                    break;
                case 1:
                    finalColor = PackRGBA (r1, g1, b1, finalAlpha);
                    break;
                case 2:
                    finalColor = PackRGBA ((2 * r0 + r1) / 3, (2 * g0 + g1) / 3, (2 * b0 + b1) / 3, finalAlpha);
                    break;
                case 3:
                    finalColor = PackRGBA ((r0 + 2 * r1) / 3, (g0 + 2 * g1) / 3, (b0 + 2 * b1) / 3, finalAlpha);
                    break;
            }


            *(uint32_t*)(image + sizeof (uint32_t) * (i + x) + (stride * (y + j))) = finalColor;
        }
    }
}

/*
Decompresses one block of a BC2 (DXT3) texture and stores the resulting pixels at the appropriate offset in 'image'.

uint32_t x:						x-coordinate of the first pixel in the block.
uint32_t y:						y-coordinate of the first pixel in the block.
uint32_t stride:				stride of a scanline in bytes.
const uint8_t *blockStorage:	pointer to the block to decompress.
uint32_t *image:				pointer to image where the decompressed pixel data should be stored.
*/
void DecompressBlockBC2 (uint32_t x, uint32_t y, uint32_t stride,
                         const uint8_t* blockStorage, unsigned char* image)
{
    int i;

    uint8_t alphaValues [16] = { 0 };

    for (i = 0; i < 4; ++i) {
        const uint16_t* alphaData = (const uint16_t*)(blockStorage);

        alphaValues [i * 4 + 0] = (((*alphaData) >> 0) & 0xF) * 17;
        alphaValues [i * 4 + 1] = (((*alphaData) >> 4) & 0xF) * 17;
        alphaValues [i * 4 + 2] = (((*alphaData) >> 8) & 0xF) * 17;
        alphaValues [i * 4 + 3] = (((*alphaData) >> 12) & 0xF) * 17;

        blockStorage += 2;
    }

    DecompressBlockBC1Internal (blockStorage,
                                image + x * sizeof (uint32_t) + (y * stride), stride, alphaValues);
}



static void DecompressBlockBC4Internal (
    const uint8_t* block, unsigned char* output,
    uint32_t outputStride, const float* colorTable)
{
    uint8_t indices [16];
    int x, y;

    Decompress16x3bitIndices (block + 2, indices);

    for (y = 0; y < 4; ++y) {
        for (x = 0; x < 4; ++x) {
            *(float*)(output + x * sizeof (float)) = colorTable [indices [y*4 + x]];
        }

        output += outputStride;
    }
}

/*
Decompresses one block of a BC4 texture and stores the resulting pixels at the appropriate offset in 'image'.

uint32_t x:						x-coordinate of the first pixel in the block.
uint32_t y:						y-coordinate of the first pixel in the block.
uint32_t stride:				stride of a scanline in bytes.
const uint8_t* blockStorage:	pointer to the block to decompress.
float* image:					pointer to image where the decompressed pixel data should be stored.
*/
void DecompressBlockBC4 (uint32_t x, uint32_t y, uint32_t stride, enum BC4Mode mode,
                         const uint8_t* blockStorage, unsigned char* image)
{
    float colorTable [8];
    float r0, r1;

    if (mode == BC4_UNORM) {
        r0 = Int8ToFloat_UNORM (blockStorage [0]);
        r1 = Int8ToFloat_UNORM (blockStorage [1]);

        colorTable [0] = r0;
        colorTable [1] = r1;

        if (r0 > r1) {
            // 6 interpolated color values
            colorTable [2] = (6*r0 + 1*r1)/7.0f; // bit code 010
            colorTable [3] = (5*r0 + 2*r1)/7.0f; // bit code 011
            colorTable [4] = (4*r0 + 3*r1)/7.0f; // bit code 100
            colorTable [5] = (3*r0 + 4*r1)/7.0f; // bit code 101
            colorTable [6] = (2*r0 + 5*r1)/7.0f; // bit code 110
            colorTable [7] = (1*r0 + 6*r1)/7.0f; // bit code 111
        } else {
            // 4 interpolated color values
            colorTable [2] = (4*r0 + 1*r1)/5.0f; // bit code 010
            colorTable [3] = (3*r0 + 2*r1)/5.0f; // bit code 011
            colorTable [4] = (2*r0 + 3*r1)/5.0f; // bit code 100
            colorTable [5] = (1*r0 + 4*r1)/5.0f; // bit code 101
            colorTable [6] = 0.0f;               // bit code 110
            colorTable [7] = 1.0f;               // bit code 111
        }
    } else if (mode == BC4_SNORM) {
        r0 = Int8ToFloat_SNORM (blockStorage [0]);
        r1 = Int8ToFloat_SNORM (blockStorage [1]);

        colorTable [0] = r0;
        colorTable [1] = r1;

        if (r0 > r1) {
            // 6 interpolated color values
            colorTable [2] = (6*r0 + 1*r1)/7.0f; // bit code 010
            colorTable [3] = (5*r0 + 2*r1)/7.0f; // bit code 011
            colorTable [4] = (4*r0 + 3*r1)/7.0f; // bit code 100
            colorTable [5] = (3*r0 + 4*r1)/7.0f; // bit code 101
            colorTable [6] = (2*r0 + 5*r1)/7.0f; // bit code 110
            colorTable [7] = (1*r0 + 6*r1)/7.0f; // bit code 111
        } else {
            // 4 interpolated color values
            colorTable [2] = (4*r0 + 1*r1)/5.0f; // bit code 010
            colorTable [3] = (3*r0 + 2*r1)/5.0f; // bit code 011
            colorTable [4] = (2*r0 + 3*r1)/5.0f; // bit code 100
            colorTable [5] = (1*r0 + 4*r1)/5.0f; // bit code 101
            colorTable [6] = -1.0f;              // bit code 110
            colorTable [7] =  1.0f;              // bit code 111
        }
    }

    DecompressBlockBC4Internal (blockStorage,
                                image + x * sizeof (float) + (y * stride), stride, colorTable);
}


/*
Decompresses one block of a BC5 texture and stores the resulting pixels at the appropriate offset in 'image'.

uint32_t x:						x-coordinate of the first pixel in the block.
uint32_t y:						y-coordinate of the first pixel in the block.
uint32_t stride:				stride of a scanline in bytes.
const uint8_t* blockStorage:	pointer to the block to decompress.
float* image:					pointer to image where the decompressed pixel data should be stored.
*/
void DecompressBlockBC5 (uint32_t x, uint32_t y, uint32_t stride, enum BC5Mode mode,
                         const uint8_t* blockStorage, unsigned char* image)
{
    // We decompress the two channels separately and interleave them when
    // writing to the output
    float c0 [16];
    float c1 [16];

    int dx, dy;

    DecompressBlockBC4 (0, 0, 4 * sizeof (float), (enum BC4Mode)mode,
                        blockStorage, (unsigned char*)c0);
    DecompressBlockBC4 (0, 0, 4 * sizeof (float), (enum BC4Mode)mode,
                        blockStorage + 8, (unsigned char*)c1);

    for (dy = 0; dy < 4; ++dy) {
        for (dx = 0; dx < 4; ++dx) {
            *(float*)(image + stride * y + (x + dx + 0) * sizeof (float) * 2) = c0 [dy * 4 + dx];
            *(float*)(image + stride * y + (x + dx + 1) * sizeof (float) * 2) = c1 [dy * 4 + dx];
        }
    }
}


void DecompressBC1(uint32_t width, uint32_t height, const uint8_t* blockStorage, unsigned char* image) {
    const uint32_t stride = width * 4;

    for( uint32_t y = 0; y < height; y += 4 )
    {
        for( uint32_t x = 0; x < width; x += 4 )
        {
            DecompressBlockBC1( x, y, stride, blockStorage, image );
            blockStorage += 8;
        }
    }
}

void DecompressBC2(uint32_t width, uint32_t height, const uint8_t* blockStorage, unsigned char* image) {
    const uint32_t stride = width * 4;

    for( uint32_t y = 0; y < height; y += 4 )
    {
        for( uint32_t x = 0; x < width; x += 4 )
        {
            DecompressBlockBC2( x, y, stride, blockStorage, image );
            blockStorage += 16;
        }
    }
}
void DecompressBC3(uint32_t width, uint32_t height, const uint8_t* blockStorage, unsigned char* image) {
    const uint32_t stride = width * 4;

    for( uint32_t y = 0; y < height; y += 4 )
    {
        for( uint32_t x = 0; x < width; x += 4 )
        {
            DecompressBlockBC3( x, y, stride, blockStorage, image );
            blockStorage += 16;
        }
    }
}