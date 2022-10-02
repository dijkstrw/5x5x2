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

#ifndef _RGBMAP_H
#define _RGBMAP_H

#include "rgbpixel.h"

typedef struct {
    union {
        struct {
            uint16_t h;
            uint8_t s;
            uint8_t v;
        };
        uint32_t raw;
    };
} __attribute__ ((packed)) hsv_t;

#define HSV(Hue, Saturation, Value) { .h = Hue * (HUE_SEXTANT/60), .s = Saturation, .v = Value }

#define HSV_WHITE                   HSV(  0,    0, 0xff)
#define HSV_WHITE_HALF              HSV(  0,    0, 0x80)
#define HSV_BLACK                   HSV(  0,    0, 0)

#define HSV_RED                     HSV(  0, 0xff, 0xff)
#define HSV_RED_HALF                HSV(  0, 0xff, 0x80)
#define HSV_ORANGE                  HSV( 30, 0xff, 0xff)
#define HSV_ORANGE_HALF             HSV( 30, 0xff, 0x80)
#define HSV_YELLOW                  HSV( 60, 0xff, 0xff)
#define HSV_YELLOW_HALF             HSV( 60, 0xff, 0x80)
#define HSV_GREEN                   HSV(120, 0xff, 0xff)
#define HSV_GREEN_HALF              HSV(120, 0xff, 0x80)
#define HSV_CYAN                    HSV(180, 0xff, 0xff)
#define HSV_CYAN_HALF               HSV(180, 0xff, 0x80)
#define HSV_BLUE                    HSV(240, 0xff, 0xff)
#define HSV_BLUE_HALF               HSV(240, 0xff, 0x80)
#define HSV_PURPLE                  HSV(270, 0xff, 0xff)
#define HSV_PURPLE_HALF             HSV(270, 0xff, 0x80)
#define HSV_MAGENTA                 HSV(300, 0xff, 0xff)
#define HSV_MAGENTA_HALF            HSV(300, 0xff, 0x80)

#define HUE_SEXTANT                 0x100
#define SEXTANT_MAX                 6
#define HUE_MAX                     ((SEXTANT_MAX * HUE_SEXTANT) - 1)

uint8_t key2led(uint8_t row, uint8_t column);
void hsv2rgb(hsv_t *h, rgbpixel_t *p);

#endif
