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
#ifndef _RGBEASE_H
#define _RGBEASE_H

#include "rgbmap.h"

enum {
    F_NOP = 0,
    F_COLOR_FLASH,
    F_COLOR_HOLD,
    F_BRIGHTEN,
    F_DIM,
    F_RAINBOW,
    F_BACKLIGHT,
    F_OVERRIDE
};

typedef uint8_t fract8_t;

typedef struct {
    hsv_t target;
    uint8_t f;
    uint8_t step;
    uint8_t round;
} rgbease_t;

typedef struct {
    uint8_t color;
    uint8_t f;
    uint8_t step;
    uint8_t round;
    uint8_t group;
} rgbaction_t;


extern uint32_t rgbgroup[ROWS_NUM][COLS_NUM];
extern rgbaction_t rgbaction[PRESSED_NUM][ROWS_NUM][COLS_NUM];
extern fract8_t rgbintensity;

#define STEP_LAST             0xff
#define STEP_RAINBOW          0x03

#define ROUND_LAST            0xff
#define ROUND_FIRST           0x00
#define ROUND_SECOND          0x01
#define ROUND_THIRD           0x02

#define GROUP_LAST            0xff

#define EASE(Color, Func, Step, Round, Group) { .color = Color, .f = Func, .step = Step, .round = Round, .group = Group }

void rgbease_init(void);
void rgbease_advance(void);
void rgbease_dim_all(void);
void rgbease_dump(void);
void rgbease_event(uint8_t row, uint8_t column, bool pressed);
void rgbease_layer(uint8_t layer);
void rgbease_process(void);
void rgbease_rainbow(uint8_t times);
void rgbease_rotate(uint8_t direction);
void rgbease_set(uint8_t pressed, uint8_t row, uint8_t column, rgbaction_t *action);
void rgbease_set_direct(uint8_t id, hsv_t target, uint8_t f, uint8_t step, uint8_t round);

void rgbgroup_dump(void);
void rgbgroup_set(uint8_t row, uint8_t column, uint8_t group);
#endif
