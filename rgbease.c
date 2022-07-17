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
 * Light adjustments over time
 */

#include <string.h>

#include "config.h"
#include "rgbease.h"

static rgbease leds[BACKLIGHT_LEDS_NUM];

static uint8_t scale8(uint8_t i, fract8 scale)
{
    return ((uint16_t)i * (uint16_t)(scale)) >> 8;
}

uint8_t ease8_inoutquad(uint8_t i)
{
    uint8_t j = i;
    if (j & 0x80) {
        j = 0xff - j;
    }
    uint8_t jj  = scale8(j, j);
    uint8_t jj2 = jj << 1;
    if( i & 0x80 ) {
        jj2 = 0xff - jj2;
    }
    return jj2;
}

void
ease_init() {
    memset(&leds, 0, sizeof(rgbease));
}

void
ease_set(uint8_t row, uint8_t column, hsv_t target, uint8_t f)
{
    uint8_t id = key2led(row, column);

    ease_set_direct(id, target, f, 0);
}

void
ease_set_direct(uint8_t id, hsv_t target, uint8_t f, uint8_t step)
{
    rgbease *led = &leds[id];

    led->target = target;
    led->f = f;
    led->step = step;
}

void
ease_rainbow(void)
{
    uint8_t i;
    hsv_t color = HSV_WHITE;

    for (i = 0; i < BACKLIGHT_LEDS_NUM; i++) {
        ease_set_direct(i, color, F_RAINBOW, 0);
        color.h += (HUE_MAX / BACKLIGHT_LEDS_NUM);
    }
}

void
ease_advance() {
    uint8_t i;
    hsv_t color;
    uint8_t step;

    for (i = 0; i < BACKLIGHT_LEDS_NUM; i++) {
        color = leds[i].target;
        switch (leds[i].f) {
            case F_NOP:
                continue;
            case F_EASEQUADIN:
                step = ease8_inoutquad(++leds[i].step);
                color.s = scale8(color.s, step);
                if (leds[i].step == 0xff) {
                    leds[i].f = F_EASEQUADOUT;
                }
                break;
            case F_EASEQUADOUT:
                step = ease8_inoutquad(++leds[i].step);
                color.s = scale8(color.s, 0xff - step);
                if (leds[i].step == 0xff) {
                    leds[i].f = F_EASEQUADIN;
                }
                break;
            case F_RAINBOW:
                color.h += 3;
                color.h %= HUE_MAX;
                leds[i].target.h = color.h;
                break;
        }
        hsv2rgb(&color, &frame[i]);
    }
    rgbpixel_render();
}
