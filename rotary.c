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

#include <stddef.h>
#include <string.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>

#include "config.h"
#include "elog.h"
#include "keymap.h"
#include "rotary.h"
#include "usb_keycode.h"

static event_t *last_event = NULL;
volatile uint16_t rotary_value = 0;

event_t rotary[LAYERS_NUM][ROTARY_DIRECTIONS] =
{
    { _C(VOLUMEDEC), _C(VOLUMEINC) },
};

void
rotary_init(void)
{
    rcc_periph_clock_enable(ROT_GPIO_RCC);
    rcc_periph_clock_enable(ROT_TIM_RCC);

    timer_set_period(ROT_TIM, ROT_PERIOD);
    timer_slave_set_mode(ROT_TIM, TIM_SMCR_SMS_EM3);
    timer_ic_set_input(ROT_TIM, TIM_IC1, TIM_IC_IN_TI1);
    timer_ic_set_input(ROT_TIM, TIM_IC2, TIM_IC_IN_TI2);
    timer_enable_counter(ROT_TIM);

    rotary_value = timer_get_counter(ROT_TIM);
}

void
rotary_process(void)
{
    uint16_t current = timer_get_counter(ROT_TIM);
    uint8_t direction = ROTARY_NONE;
    event_t *event;

    if (last_event) {
        if (send_event_if_idle(last_event, 0)) {
            last_event = NULL;
        }
    } else {
        if (current > rotary_value) {
            direction = ROTARY_FORWARD;
        } else if (current < rotary_value) {
            direction = ROTARY_BACKWARD;
        }

        if (direction != ROTARY_NONE) {
            elog("count = %d", current);
            event = &rotary[layer][direction];
            if (send_event_if_idle(event, 1)) {
                last_event = event;
            }
        }
    }

    rotary_value = current;
}

void
rotary_set(uint8_t l, uint8_t d, event_t *event)
{
    if ((l > LAYERS_NUM) ||
        (d >= ROTARY_NONE)) {
        elog("rotary position out of bounds");
        return;
    }

    memcpy(&rotary[l][d], event, sizeof(event_t));
}
