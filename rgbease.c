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
#include "rotary.h"

static rgbease leds[RGB_ALL_NUM];
static uint32_t ease_timer = 0;

static uint8_t group[ROWS_NUM][COLS_NUM] =
{
    { 1,  1,  1,  1,  2 },
    { 1,  1,  1,  1,  1 },
    { 2,  2,  2,  2,  2 },
    { 3,  3,  3,  3,  3 },
    { 3,  3,  3,  3,  3 },
};

/*
 * Actions are stored per key release and press event
 */
#define PRESSED_NUM                2

static rgbaction action[PRESSED_NUM][ROWS_NUM][COLS_NUM] =
{
    {
        {
            EASE(HSV_PURPLE_HALF,  F_COLOR_HOLD, 0, 0, 1),
            EASE(HSV_RED_HALF,     F_COLOR_HOLD, 0, 0, 1),
            EASE(HSV_ORANGE_HALF,  F_COLOR_HOLD, 0, 0, 1),
            EASE(HSV_YELLOW_HALF,  F_COLOR_HOLD, 0, 0, 1),
            EASE(HSV_GREEN_HALF,   F_COLOR_HOLD, 0, 0, 1)
        },
        {
            EASE(HSV_CYAN_HALF,    F_COLOR_HOLD, 0, 0, 1),
            EASE(HSV_BLUE_HALF,    F_COLOR_HOLD, 0, 0, 1),
            EASE(HSV_PURPLE_HALF,  F_COLOR_HOLD, 0, 0, 1),
            EASE(HSV_MAGENTA_HALF, F_COLOR_HOLD, 0, 0, 1),
            EASE(HSV_WHITE_HALF,   F_COLOR_HOLD, 0, 0, 2)
        },
        {
            EASE(HSV_BLACK,        F_NOP,        0, 0, 0),
            EASE(HSV_BLACK,        F_NOP,        0, 0, 0),
            EASE(HSV_BLACK,        F_NOP,        0, 0, 0),
            EASE(HSV_BLACK,        F_NOP,        0, 0, 0),
            EASE(HSV_BLACK,        F_NOP,        0, 0, 0)
        },
        {
            EASE(HSV_BLACK,        F_NOP,        0, 0, 0),
            EASE(HSV_BLACK,        F_NOP,        0, 0, 0),
            EASE(HSV_BLACK,        F_NOP,        0, 0, 0),
            EASE(HSV_BLACK,        F_NOP,        0, 0, 0),
            EASE(HSV_BLACK,        F_NOP,        0, 0, 0)
        },
        {
            EASE(HSV_BLACK,        F_NOP,        0, 0, 0),
            EASE(HSV_BLACK,        F_NOP,        0, 0, 0),
            EASE(HSV_BLACK,        F_NOP,        0, 0, 0),
            EASE(HSV_BLACK,        F_NOP,        0, 0, 0),
            EASE(HSV_BLACK,        F_NOP,        0, 0, 0)
        },
    },
    {
        {
            EASE(HSV_PURPLE,       F_COLOR_HOLD, 0, 0, 0),
            EASE(HSV_RED,          F_COLOR_HOLD, 0, 0, 0),
            EASE(HSV_ORANGE,       F_COLOR_HOLD, 0, 0, 0),
            EASE(HSV_YELLOW,       F_COLOR_HOLD, 0, 0, 0),
            EASE(HSV_GREEN,        F_COLOR_HOLD, 0, 0, 2)
        },
        {
            EASE(HSV_CYAN,         F_COLOR_HOLD, 0, 0, 0),
            EASE(HSV_BLUE,         F_COLOR_HOLD, 0, 0, 0),
            EASE(HSV_PURPLE,       F_COLOR_HOLD, 0, 0, 0),
            EASE(HSV_MAGENTA,      F_COLOR_HOLD, 0, 0, 0),
            EASE(HSV_WHITE,        F_COLOR_HOLD, 0, 0, 0)
        },
        {
            EASE(HSV_BLACK,        F_NOP,        0, 0, 0),
            EASE(HSV_BLACK,        F_NOP,        0, 0, 0),
            EASE(HSV_BLACK,        F_NOP,        0, 0, 0),
            EASE(HSV_BLACK,        F_NOP,        0, 0, 0),
            EASE(HSV_BLACK,        F_NOP,        0, 0, 0)
        },
        {
            EASE(HSV_BLACK,        F_NOP,        0, 0, 0),
            EASE(HSV_BLACK,        F_NOP,        0, 0, 0),
            EASE(HSV_BLACK,        F_NOP,        0, 0, 0),
            EASE(HSV_BLACK,        F_NOP,        0, 0, 0),
            EASE(HSV_BLACK,        F_NOP,        0, 0, 0)
        },
        {
            EASE(HSV_BLACK,        F_NOP,        0, 0, 0),
            EASE(HSV_BLACK,        F_NOP,        0, 0, 0),
            EASE(HSV_BLACK,        F_NOP,        0, 0, 0),
            EASE(HSV_BLACK,        F_NOP,        0, 0, 0),
            EASE(HSV_BLACK,        F_NOP,        0, 0, 0)
        },
    }
};

static uint8_t
scale8(uint8_t i, fract8 scale)
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
ease_event(uint8_t row, uint8_t column, bool pressed)
{
    uint8_t r, c, id;
    rgbaction *act = &action[pressed][row][column];

    if (act->f == F_NOP) {
        return;
    } else {
        if (act->group) {
            for (r = 0; r < ROWS_NUM; r++) {
                for (c = 0; c < COLS_NUM; c++) {
                    if (act->group == group[r][c]) {
                        id = key2led(r, c);
                        ease_set_direct(id, act->target, act->f, act->step, act->round);
                    }
                }
            }
        } else {
            id = key2led(row, column);
            ease_set_direct(id, act->target, act->f, act->step, act->round);
        }
    }
}

void
ease_set(uint8_t row, uint8_t column, hsv_t target, uint8_t f)
{
    uint8_t id = key2led(row, column);

    ease_set_direct(id, target, f, 0, 0);
}

void
ease_set_direct(uint8_t id, hsv_t target, uint8_t f, uint8_t step, uint8_t round)
{
    rgbease *led = &leds[id];

    led->target = target;
    led->f = f;
    led->step = step;
    led->round = round;
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

uint32_t _rotate_timer = 0;
void
ease_rotate(uint8_t direction)
{
    uint8_t i;
    uint8_t offset;
    hsv_t color_green = HSV_GREEN;
    hsv_t color_red = HSV_RED;
    hsv_t color;

    /* Only set one led per ease * 2 interval */
    if (_rotate_timer == ease_timer) {
        return;
    } else {
        _rotate_timer = ease_timer + MS_EASE;
    }

    for (i = 0; i < RGB_BACKLIGHT_NUM; i++) {
        if (direction == ROTARY_FORWARD) {
            offset = RGB_BACKLIGHT_OFFSET + i;
            color = color_green;
        } else if (direction == ROTARY_BACKWARD) {
            offset = RGB_BACKLIGHT_OFFSET + RGB_BACKLIGHT_NUM - 1 - i;
            color = color_red;
        }
        if (leds[offset].f != F_COLOR_FLASH) {
            ease_set_direct(offset, color, F_COLOR_FLASH, 0, 0);
            return;
        }
    }
}

void
ease_advance() {
    uint8_t i;
    hsv_t color;
    hsv_t color_backlight = HSV_ORANGE;
    uint8_t step;

    for (i = 0; i < RGB_ALL_NUM; i++) {
        color = leds[i].target;
        switch (leds[i].f) {
            case F_NOP:
                if (i > RGB_BACKLIGHT_OFFSET) {
                    color = color_backlight;
                    ease_set_direct(i, color, F_BACKLIGHT, 0 , 0);
                } else {
                    continue;
                }
                break;

            case F_COLOR_FLASH:
                leds[i].step += 3;
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

            case F_RAINBOW:
                ++leds[i].step;
                color.h += STEP_RAINBOW;
                color.h %= HUE_MAX;
                leds[i].target.h = color.h;
                if (leds[i].step == STEP_LAST) {
                    leds[i].round--;
                    if (leds[i].round == 0) {
                        leds[i].f = F_DIM;
                    }
                }
                break;

            case F_BACKLIGHT:
                color.v = scale8(color.v, 0x20);
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
