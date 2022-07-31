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

#include "clock.h"
#include "config.h"
#include "rgbease.h"

static rgbease leds[RGB_ALL_NUM];
static uint32_t ease_timer = 0;

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
    memset(&leds, 0, sizeof(leds));
}

void
ease_set(uint8_t row, uint8_t column, hsv_t target, uint8_t f)
{
    uint8_t id = key2led(row, column);

    ease_set_direct(id, target, f, 0, 0);
}

void
ease_set_direct(uint8_t id, hsv_t target, uint8_t f, uint8_t step, uint8_t phase)
{
    rgbease *led = &leds[id];

    led->target = target;
    led->f = f;
    led->step = step;
    led->phase = phase;
}

void
ease_rainbow(uint8_t times)
{
    uint8_t i;
    hsv_t color = HSV(0, 0xff, 0xff);

    for (i = 0; i < RGB_ALL_NUM; i++) {
        ease_set_direct(i, color, F_RAINBOW, 0, times);
        color.h += (HUE_MAX / RGB_ALL_NUM);
    }
}

void
ease_dim_all(void)
{
    uint8_t i;
    rgbease *led;

    for (i = 0; i < RGB_ALL_NUM; i++) {
        led = &leds[i];
        ease_set_direct(i, led->target, F_DIM, 0, 0);
    }
}

void
ease_rotate(uint8_t direction)
{
    uint8_t i;
    hsv_t color = HSV(0xff, 0x80, 0x80);
    uint8_t phase = (LAST_STEP / RGB_BACKLIGHT_NUM) * 2;

    for (i = 0; i < RGB_BACKLIGHT_NUM; i++) {
        ease_set_direct(RGB_BACKLIGHT_OFFSET + i, color, F_ROTATE_FORWARD, 0, phase);
        if (color.h < phase) {
            color.h = 0;
        } else {
            color.h -= phase;
        }
    }
}

void
ease_advance() {
    uint8_t i;
    hsv_t color;
    uint8_t step;

    for (i = 0; i < RGB_ALL_NUM; i++) {
        color = leds[i].target;
        switch (leds[i].f) {
            case F_NOP:
                continue;
            case F_EASEQUADIN:
                step = ease8_inoutquad(++leds[i].step);
                color.s = scale8(color.s, step);
                if (leds[i].step == LAST_STEP) {
                    leds[i].f = F_EASEQUADOUT;
                }
                break;
            case F_EASEQUADOUT:
                step = ease8_inoutquad(++leds[i].step);
                color.s = scale8(color.s, 0xff - step);
                if (leds[i].step == LAST_STEP) {
                    leds[i].f = F_EASEQUADIN;
                }
                break;
            case F_DIM:
                step = ease8_inoutquad(++leds[i].step);
                color.v = scale8(color.v, LAST_STEP - step);
                if (leds[i].step == LAST_STEP) {
                    leds[i].f = F_NOP;
                    color.v = 0;
                }
                break;
            case F_BRIGHTEN:
                step = ease8_inoutquad(++leds[i].step);
                color.v = scale8(color.v, step);
                if (leds[i].step == LAST_STEP) {
                    leds[i].f = F_NOP;
                }
                break;
            case F_RAINBOW:
                ++leds[i].step;
                color.h += 3;
                color.h %= HUE_MAX;
                leds[i].target.h = color.h;
                if (leds[i].step == LAST_STEP) {
                    leds[i].phase--;
                    if (leds[i].phase == 0) {
                        leds[i].f = F_DIM;
                    }
                }
                break;
            case F_BLIP:
                leds[i].step += 3;
                step = ease8_inoutquad(leds[i].step);
                if (leds[i].step == LAST_STEP) {
                    leds[i].phase++;
                    leds[i].step = 0;
                }
                switch (leds[i].phase) {
                    case 0:
                        color.v = scale8(color.v, step);
                        break;
                    case 1:
                        color.v = scale8(color.v, LAST_STEP - step);
                        break;
                    case 2:
                        leds[i].f = F_NOP;
                        color.v = 0;
                        break;
                }
                break;
            case F_ROTATE_FORWARD:
                leds[i].step += 1;
                if (leds[i].step == 60) {
                    leds[i].step = 0;
                    color.h += leds[i].phase;
                    color.h %= HUE_MAX;
                    leds[i].target = color;
                }
                break;
        }

        hsv2rgb(&color, &frame[i]);
    }
    rgbpixel_render();
}

void
ease_process() {
    if (timer_passed(ease_timer)) {
        ease_advance();
        ease_timer = timer_set(MS_EASE);
    }
}
