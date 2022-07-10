/*
 * Copyright (c) 2022 by Willem Dijkstra <wpd@xs4all.nl>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *    * Neither the name of the auhor nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Map key/rows to rgbpixels & map between colorspaces.
 */
#include "elog.h"
#include "config.h"
#include "rgbmap.h"

/* Map location of a rgb led to a key */

static uint8_t key2rgb[ROWS_NUM][COLS_NUM] =
{
    {  0,  1,  2,  3,  4 },
    {  9,  8,  7,  6,  5 },
    { 10, 11, 12, 13, 14 },
    { 19, 18, 17, 16, 15 },
    { 20, 21, 22, 23, 24 }
};

uint8_t
key2led(uint8_t row, uint8_t column)
{
    row %= ROWS_NUM;
    column %= COLS_NUM;
    return key2rgb[row][column];
}

/* hsv2rgb using integer arithmetic
 *
 * See https://www.vagrearg.org/content/hsvrgb for an excellent
 * explanation by B. Stultiens.
 */
void
hsv2rgb(hsv_t *h, rgbpixel_t *p)
{
    uint8_t sextant;
    uint8_t *r = &p->r;
    uint8_t *g = &p->g;
    uint8_t *b = &p->b;
    uint8_t *t;
    uint16_t bottom_level;
    uint8_t h_fraction;
    uint32_t slope;

#define swap(a, b) do {t = a; a = b; b = t; } while (0)

    /* monochromatic test */
    if (!h->s) {
        *r = *g = *b = h->v;
        return;
    }

    sextant = h->h >> 8;
    sextant %= SEXTANT_MAX;

    /* Looking at the hsv to rgb picture:
     *
     *      1|
     *       |
     *      V|-r-.-g-.-g-.-b-.-b-.-r-.
     *       |  / \     / \     / \
     *       | g   r   b   g   r   b
     *       |/     \ /     \ /     \
     * V(1-S)|-b-.-b-.-r-.-r-.-g-.-g-.
     *       |
     *      0|_________________________
     *        0 60  120 180 240 300 360
     *
     * - one of the rgb colors is at h->v
     * - one of the rgb colors is at h->v * (1 - h->s)
     * - one of the rgb colors is enroute up or down
     *
     * Sextant one is our standard, and when in other sextants we
     * rearrange the pointers so that we can calculate like we were in
     * sextant one.
     */
    switch (sextant) {
        case 0:
            swap(r, g);
            break;
        case 1:
            break;
        case 2:
            swap(r, b);
            break;
        case 3:
            swap(r, g);
            swap(g, b);
            break;
        case 4:
            swap(g, b);
            break;
        case 5:
            swap(r, g);
            swap(r, b);
            break;
    }

    *g = h->v;

    bottom_level = h->v * (0xff - h->s);
    bottom_level += 1;
    bottom_level += bottom_level >> 8;

    *b = bottom_level >> 8;

    h_fraction = h->h & 0xff;

    if (sextant & 1) {
        /* slope down */
        slope = h->v * (uint32_t)((0xff << 8) - (uint16_t)(h->s * h_fraction));
    } else {
        /* slope up */
        slope = h->v * (uint32_t)((0xff << 8) - (uint16_t)(h->s * (0x100 - h_fraction)));
    }

    slope += slope >> 8;
    slope += h->v;
    *r = slope >> 16;
}
