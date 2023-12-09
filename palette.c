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
