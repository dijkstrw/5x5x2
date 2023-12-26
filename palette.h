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
#ifndef _PALETTE_H
#define _PALETTE_H

#include "rgbmap.h"

/*
 * Basic hues, 15° increments
 */

#define HUE_RED                       0
#define HUE_VERMILION                15
#define HUE_ORANGE                   30
#define HUE_AMBER                    45
#define HUE_YELLOW                   60
#define HUE_LIME                     75
#define HUE_CHARTREUSE               90
#define HUE_DDAHAL                  105
#define HUE_GREEN                   120
#define HUE_ERIN                    135
#define HUE_SPRING                  150
#define HUE_GASHYANTA               165
#define HUE_CYAN                    180
#define HUE_CAPRI                   195
#define HUE_AZURE                   210
#define HUE_CERULEAN                225
#define HUE_BLUE                    240
#define HUE_VOLTA                   255
#define HUE_VIOLET                  270
#define HUE_PURPLE                  285
#define HUE_MAGENTA                 300
#define HUE_CERISE                  315
#define HUE_ROSE                    330
#define HUE_CRIMSON                 345

/*
 * Basic colors, CSS 1-2.0, HTML 3.2-4, VGA color names
 */
#define HSV_WHITE                   HSV(HUE_RED,           0, 0xff) // #ffffff
#define HSV_SILVER                  HSV(HUE_RED,           0, 0xc0) // #c0c0c0
#define HSV_GRAY                    HSV(HUE_RED,           0, 0x80) // #808080
#define HSV_BLACK                   HSV(HUE_RED,           0, 0)    // #000000
#define HSV_RED                     HSV(HUE_RED,        0xff, 0xff) // #ff0000
#define HSV_MAROON                  HSV(HUE_RED,        0xff, 0x80) // #800000
#define HSV_YELLOW                  HSV(HUE_YELLOW,     0xff, 0xff) // #ffff00
#define HSV_OLIVE                   HSV(HUE_YELLOW,     0xff, 0x80) // #808000
#define HSV_BLIME                   HSV(HUE_GREEN,      0xff, 0xff) // #00ff00
#define HSV_GREEN                   HSV(HUE_GREEN,      0xff, 0x80) // #008000
#define HSV_AQUA                    HSV(HUE_CYAN,       0xff, 0xff) // #00ffff
#define HSV_TEAL                    HSV(HUE_CYAN,       0xff, 0x80) // #008080
#define HSV_BLUE                    HSV(HUE_BLUE,       0xff, 0xff) // #0000ff
#define HSV_NAVY                    HSV(HUE_BLUE,       0xff, 0x80) // #000080
#define HSV_FUCHSIA                 HSV(HUE_MAGENTA,    0xff, 0xff) // #ff00ff
#define HSV_PURPLE                  HSV(HUE_MAGENTA,    0xff, 0x80) // #800080

/*
 * Add 15° increments
 */
#define HSV_VERMILION               HSV(HUE_VERMILION,  0xff, 0xff) // #ff4000
#define HSV_AMBER                   HSV(HUE_AMBER,      0xff, 0xff) // #ffc000
#define HSV_LIME                    HSV(HUE_LIME,       0xff, 0xff) // #c0ff00
#define HSV_ORANGE                  HSV(HUE_ORANGE,     0xff, 0xff) // #ff8000
#define HSV_CHARTREUSE              HSV(HUE_CHARTREUSE, 0xff, 0xff) // #80ff00
#define HSV_DDAHAL                  HSV(HUE_DDAHAL,     0xff, 0xff) // #40ff00
#define HSV_ERIN                    HSV(HUE_ERIN,       0xff, 0xff) // #00ff40
#define HSV_SPRING                  HSV(HUE_SPRING,     0xff, 0xff) // #00ff80
#define HSV_GASHYANTA               HSV(HUE_GASHYANTA,  0xff, 0xff) // #00ffc0
#define HSV_CYAN                    HSV(HUE_CYAN,       0xff, 0xff) // #00ffff
#define HSV_CAPRI                   HSV(HUE_CAPRI,      0xff, 0xff) // #00c0ff
#define HSV_AZURE                   HSV(HUE_AZURE,      0xff, 0xff) // #0080ff
#define HSV_CERULEAN                HSV(HUE_CERULEAN,   0xff, 0xff) // #0040ff
#define HSV_VOLTA                   HSV(HUE_VOLTA,      0xff, 0xff) // #4000ff
#define HSV_VIOLET                  HSV(HUE_VIOLET,     0xff, 0xff) // #8000ff
#define HSV_MAGENTA                 HSV(HUE_MAGENTA,    0xff, 0xff) // #ff00ff
#define HSV_CERISE                  HSV(HUE_CERISE,     0xff, 0xff) // #ff00c0
#define HSV_ROSE                    HSV(HUE_ROSE,       0xff, 0xff) // #ff0080
#define HSV_CRIMSON                 HSV(HUE_CRIMSON,    0xff, 0xff) // #ff0040

/*
 * Explicit const hsv_t for each color
 */
extern const hsv_t hsv_white;
extern const hsv_t hsv_silver;
extern const hsv_t hsv_gray;
extern const hsv_t hsv_black;
extern const hsv_t hsv_red;
extern const hsv_t hsv_maroon;
extern const hsv_t hsv_yellow;
extern const hsv_t hsv_olive;
extern const hsv_t hsv_blime;
extern const hsv_t hsv_green;
extern const hsv_t hsv_aqua;
extern const hsv_t hsv_teal;
extern const hsv_t hsv_blue;
extern const hsv_t hsv_navy;
extern const hsv_t hsv_fuchsia;
extern const hsv_t hsv_vermilion;
extern const hsv_t hsv_amber;
extern const hsv_t hsv_lime;
extern const hsv_t hsv_orange;
extern const hsv_t hsv_chartreuse;
extern const hsv_t hsv_ddahal;
extern const hsv_t hsv_erin;
extern const hsv_t hsv_spring;
extern const hsv_t hsv_gashyanta;
extern const hsv_t hsv_cyan;
extern const hsv_t hsv_capri;
extern const hsv_t hsv_azure;
extern const hsv_t hsv_cerulean;
extern const hsv_t hsv_volta;
extern const hsv_t hsv_violet;
extern const hsv_t hsv_magenta;
extern const hsv_t hsv_cerise;
extern const hsv_t hsv_rose;
extern const hsv_t hsv_crimson;

enum {
    COLOR_OFF,
    COLOR_ON,

    COLOR_FORWARD,
    COLOR_BACKWARD,

    COLOR_BACKGROUND,

    COLOR_1,
    COLOR_DESKTOP = COLOR_1,
    COLOR_2,
    COLOR_3,
    COLOR_4,
    COLOR_5,

    COLOR_6,
    COLOR_LAYER = COLOR_6,
    COLOR_7,
    COLOR_8,
    COLOR_9,
    COLOR_10,
    COLOR_MACRO = COLOR_10,
};

extern hsv_t palette[PALETTE_NUM];

void palette_dump(void);
hsv_t palette_get(uint8_t color);
void palette_set(uint8_t color, hsv_t hsv);
#endif
