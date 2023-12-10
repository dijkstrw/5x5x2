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

#include "elog.h"
#include "palette.h"
#include "serial.h"

hsv_t palette[PALETTE_NUM] = {
    HSV_BLACK,
    HSV_WHITE,

    HSV_LIME,
    HSV_VERMILION,

    HSV_AMBER,

    HSV_CHARTREUSE,
    HSV_YELLOW,
    HSV_ROSE,
    HSV_VIOLET,
    HSV_AZURE,

    HSV_MAGENTA,
    HSV_VOLTA,
    HSV_CYAN,
    HSV_SPRING,
    HSV_CRIMSON,
};

/*
 * Define all colors as consts for easy use as a starting point in
 * color operations
 */

const hsv_t hsv_white         = HSV_WHITE;
const hsv_t hsv_silver        = HSV_SILVER;
const hsv_t hsv_gray          = HSV_GRAY;
const hsv_t hsv_black         = HSV_BLACK;
const hsv_t hsv_red           = HSV_RED;
const hsv_t hsv_maroon        = HSV_MAROON;
const hsv_t hsv_yellow        = HSV_YELLOW;
const hsv_t hsv_olive         = HSV_OLIVE;
const hsv_t hsv_blime         = HSV_BLIME;
const hsv_t hsv_green         = HSV_GREEN;
const hsv_t hsv_aqua          = HSV_AQUA;
const hsv_t hsv_teal          = HSV_TEAL;
const hsv_t hsv_blue          = HSV_BLUE;
const hsv_t hsv_navy          = HSV_NAVY;
const hsv_t hsv_fuchsia       = HSV_FUCHSIA;
const hsv_t hsv_vermilion     = HSV_VERMILION;
const hsv_t hsv_amber         = HSV_AMBER;
const hsv_t hsv_lime          = HSV_LIME;
const hsv_t hsv_orange        = HSV_ORANGE;
const hsv_t hsv_chartreuse    = HSV_CHARTREUSE;
const hsv_t hsv_ddahal        = HSV_DDAHAL;
const hsv_t hsv_erin          = HSV_ERIN;
const hsv_t hsv_spring        = HSV_SPRING;
const hsv_t hsv_gashyanta     = HSV_GASHYANTA;
const hsv_t hsv_cyan          = HSV_CYAN;
const hsv_t hsv_capri         = HSV_CAPRI;
const hsv_t hsv_azure         = HSV_AZURE;
const hsv_t hsv_cerulean      = HSV_CERULEAN;
const hsv_t hsv_volta         = HSV_VOLTA;
const hsv_t hsv_violet        = HSV_VIOLET;
const hsv_t hsv_magenta       = HSV_MAGENTA;
const hsv_t hsv_cerise        = HSV_CERISE;
const hsv_t hsv_rose          = HSV_ROSE;
const hsv_t hsv_crimson       = HSV_CRIMSON;

void
palette_dump()
{
    uint8_t i;

    printfnl("palette:");
    for (i = 0; i < PALETTE_NUM; i++) {
        printfnl("hsv %02x: %04x,%02x,%02x",
               i,
               palette[i].h,
               palette[i].s,
               palette[i].v);
    }
}

hsv_t
palette_get(uint8_t color)
{
    color = color % PALETTE_NUM;
    return palette[color];
}

void
palette_set(uint8_t color, hsv_t hsv)
{
    if (color >= PALETTE_NUM) {
        elog("palette color out of bounds");
        return;
    }
    palette[color] = hsv;
}
