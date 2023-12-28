/*
 * Copyright (c) 2022-2023 by Willem Dijkstra <wpd@xs4all.nl>.
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
 * rgb easing (adjustments over time) functions
 *
 * When an event happens, we want to slowly move towards a color event
 * on the keyboard. This module implements the slowly moving towards a
 * color.
 *
 * The easing functions are updated every 1ms and keep track of where
 * they are using ``round`` (e.g. going toward a color, going away from a
 * color) and ``step`` (e.g. step 10 in the move to that color). The
 * functions supported are:
 *
 *   - NOP         : no operation, except if the led is part of the backlight
 *                   (2nd layer) leds, then adjust to backlight. Note
 *                   that all functions below that terminate end up as
 *                   NOP after termination.
 *
 *   - COLOR_FLASH : ease up towards a color in <step> increments, and
 *                   then fade away.
 *
 *   - COLOR_HOLD  : ease towards a color in <step> increments and hold.
 *
 *   - DIM         : ease away in <step>s from color, end in off.
 *
 *   - BRIGHTEN    : ease towards color in <step>s, end in off.
 *
 *   - RAINBOW     : use on <round> to walk all hues, set <round> to
 *                   number of times to repeat.
 *
 *   - BACKLIGHT   : use backlight color at backlight intensity.
 *
 *   - OVERRIDE    : do not ease this led
 */

#include <string.h>

#include "clock.h"
#include "config.h"
#include "elog.h"
#include "palette.h"
#include "rgbease.h"
#include "rotary.h"
#include "serial.h"

static rgbease_t leds[RGB_ALL_NUM];
static uint32_t rgbease_timer = 0;
fract8_t rgbintensity;

static uint8_t
scale8(uint8_t i, fract8_t scale)
{
    return ((uint16_t)i * (uint16_t)(scale)) >> 8;
}

static uint8_t
ease8_inoutquad(uint8_t i)
{
    uint8_t j = i;
    if (j & 0x80) {
        j = 0xff - j;
    }
    uint8_t jj  = scale8(j, j);
    uint8_t jj2 = jj << 1;
    if (i & 0x80) {
        jj2 = 0xff - jj2;
    }
    return jj2;
}

void
rgbease_init()
{
    memset(&leds, 0, sizeof(leds));
    rgbintensity = RGB_BACKLIGHT_INTENS;
}

void
rgbease_set(uint8_t id, hsv_t target, uint8_t f, uint8_t step, uint8_t round)
{
    rgbease_t *led = &leds[id];

    led->target = target;
    led->target.v = scale8(led->target.v, rgbintensity);
    led->f = f;
    led->step = step;
    led->round = round;
}

void
rgbease_rainbow(uint8_t times)
{
    uint8_t i;
    hsv_t color = HSV_RED;

    for (i = 0; i < RGB_ALL_NUM; i++) {
        rgbease_set(i, color, F_RAINBOW_THEN_DIM, 0, times);
        color.h += (HUE_MAX / RGB_ALL_NUM);
    }
}

void
rgbease_dim_all(void)
{
    uint8_t i;
    rgbease_t *led;

    for (i = 0; i < RGB_ALL_NUM; i++) {
        led = &leds[i];
        rgbease_set(i, led->target, F_DIM, 0, 0);
    }
}

uint32_t _rotate_timer = 0;
void
rgbease_rotate(uint8_t direction)
{
    uint8_t i;
    uint8_t offset;
    hsv_t color;

    /* Only set one led per ease * 2 interval */
    if (_rotate_timer == rgbease_timer) {
        return;
    } else {
        _rotate_timer = rgbease_timer + MS_EASE;
    }

    for (i = 0; i < RGB_BACKLIGHT_NUM; i++) {
        if (direction == ROTARY_FORWARD) {
            offset = RGB_BACKLIGHT_OFFSET + i;
            color = palette[COLOR_FORWARD];
        } else if (direction == ROTARY_BACKWARD) {
            offset = RGB_BACKLIGHT_OFFSET + RGB_BACKLIGHT_NUM - 1 - i;
            color = palette[COLOR_BACKWARD];
        }
        if (leds[offset].f != F_COLOR_FLASH) {
            rgbease_set(offset, color, F_COLOR_FLASH, 0, 0);
            return;
        }
    }
}

void
rgbease_advance() {
    uint8_t i;
    hsv_t color;
    uint8_t step;

    for (i = 0; i < RGB_ALL_NUM; i++) {
        color = leds[i].target;
        switch (leds[i].f) {
            case F_NOP:
                if (i >= RGB_BACKLIGHT_OFFSET) {
                    rgbease_set(i, palette[COLOR_BACKGROUND], F_BACKLIGHT, 0 , 0);
                } else {
                    continue;
                }
                break;

            case F_COLOR_FLASH:
                leds[i].step += SPEED_STEP_FLASH;
                step = ease8_inoutquad(leds[i].step);
                if (leds[i].step == STEP_LAST) {
                    leds[i].round++;
                    leds[i].step = 0;
                }
                switch (leds[i].round) {
                    case ROUND_FIRST:
                        /* move towards the color */
                        color.v = scale8(color.v, step);
                        break;
                    case ROUND_SECOND:
                        color.v = scale8(color.v, STEP_LAST - step);
                        break;
                    case ROUND_THIRD:
                        leds[i].f = F_NOP;
                        color.v = 0;
                        break;
                }
                break;

            case F_COLOR_FLASHING:
                leds[i].step += SPEED_STEP_FLASH;
                step = ease8_inoutquad(leds[i].step);
                if (leds[i].step == STEP_LAST) {
                    leds[i].round++;
                    leds[i].step = 0;
                }
                if (leds[i].round == ROUND_THIRD) {
                    leds[i].round = ROUND_FIRST;
                }
                switch (leds[i].round) {
                    case ROUND_FIRST:
                        /* move towards the color */
                        color.v = scale8(color.v, step);
                        break;
                    case ROUND_SECOND:
                        color.v = scale8(color.v, STEP_LAST - step);
                        break;
                }
                break;

            case F_COLOR_HOLD:
                /* move towards the color */
                step = ease8_inoutquad(++leds[i].step);
                color.v = scale8(color.v, step);
                if (leds[i].step == STEP_LAST) {
                    leds[i].f = F_NOP;
                }
                break;

            case F_DIM:
                step = ease8_inoutquad(++leds[i].step);
                color.v = scale8(color.v, STEP_LAST - step);
                if (leds[i].step == STEP_LAST) {
                    leds[i].f = F_NOP;
                    color.v = 0;
                }
                break;

            case F_BRIGHTEN:
                step = ease8_inoutquad(++leds[i].step);
                color.v = scale8(color.v, step);
                if (leds[i].step == STEP_LAST) {
                    leds[i].f = F_NOP;
                }
                break;

            case F_RAINBOW_THEN_DIM:
                ++leds[i].step;
                color.h += HUE_STEP_RAINBOW;
                color.h %= HUE_MAX;
                leds[i].target.h = color.h;
                if (leds[i].step == STEP_LAST) {
                    leds[i].round--;
                    if (leds[i].round == 0) {
                        leds[i].f = F_DIM;
                    }
                }
                break;

            case F_SLOW_RAINBOW:
                if ((rgbease_timer % TIMER_SLOW_RAINBOW) == 0) {
                    color.h += HUE_STEP_SLOW_RAINBOW;
                    color.h %= HUE_MAX;
                    leds[i].target.h = color.h;
                }
                break;

            case F_BACKLIGHT:
                color.v = scale8(color.v, rgbintensity);
                break;

            case F_OVERRIDE:
                continue;
        }

        hsv2rgb(&color, &frame[i]);
    }
    rgbpixel_render();
}

void
rgbease_process()
{
    if (timer_passed(rgbease_timer)) {
        rgbease_advance();
        rgbease_timer = timer_set(MS_EASE);
    }
}
