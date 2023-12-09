/*
 * Copyright (c) 2023 by Willem Dijkstra <wpd@xs4all.nl>.
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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _LIGHT_H
#define _LIGHT_H

#include <stdint.h>

#include "config.h"

/* The keyboard has 5x5 + 8 leds and can show a variety of statusses.
 *
 * These "system events" can reach us from either:
 * - a key (up/down) event
 * - a system event signalled to us via serial
 *
 * Classes of "system events" we want to display are:
 *
 * - desktop number changed -- display the current desktop number on a
 *   key or keygroup using a color from the palette.
 *
 * - sound mute enabled/disabled -- could be done by using red on the
 *   lower lights to indicate that sound is muted.
 *
 * - microphone mute enabled/disabled -- could be done by using purple
 *   on the lower lights to indicate that the microphone is muted.
 *
 * - layer number changed -- display the current layer number on a key
 *   or keygroup using a color from the palette.
 *
 * - autokey running -- autokey will wreak havoc on any other
 *   keypresses, so clearly displaying that the keyboard is spamming
 *   away is crucial.
 *
 * - automouse running -- automouse will still allow keypresses, but
 *   can be in the way of normal mousework, so this too must be
 *   clearly visible.
 *
 * - nkro status
 *
 * - screen locked
 *
 * - power up / down
 *
 * - vumeter -- color information coming from serial that maps to
 *   audio played by the main system.
 *
 * - custom -- effects you want to trigger the keyboard for.
 *
 * We need to be able to communicate these items via serial, so what is the
 * protocol:
 *
 * D T SS DD
 * | |  | |
 * | |  | ╰ Desktop 01-10
 * | |  ╰-- Screen 00 = all, 01-04 a specific one
 * | ╰----- 'D' for desktop
 * ╰------- 'D' fixed, to indicate 'D'isplay event / 'D'esktop event
 *
 * D T MM
 * | |  |
 * | |  ╰-- Mute status 00 = not muted, 01 = muted
 * | ╰----- 'M' for sound output, 'R' for mic input
 * ╰------- 'D' fixed, to indicate 'D'isplay event / 'D'esktop event
 *
 * D T VVVV
 * | |  |
 * | |  ╰-- Volume 0000 - ffff
 * | ╰----- 'V' for volume
 * ╰------- 'D' fixed, to indicate 'D'isplay event / 'D'esktop event
 *
 */

enum {
    LIGHT_AUTOMOUSE       = 'A',
    LIGHT_BACKLIGHT       = 'B',
    LIGHT_DESKTOP         = 'D',
    LIGHT_LAYER           = 'L',
    LIGHT_MACRO           = 'm',
    LIGHT_MIC_MUTE        = 'R',
    LIGHT_MUTE            = 'M',
    LIGHT_NKRO            = 'N',
    LIGHT_VOLUME          = 'V',
};

typedef struct {
    uint8_t desktop[SCREENS_NUM];
    uint8_t mic_mute;
    uint8_t mute;
    uint16_t volume;
} lightstate_t;

void light_init(void);
void light_apply_state(uint8_t only_type);
void light_set_desktop(uint8_t screen, uint8_t display);
void light_set_mic_mute(uint8_t state);
void light_set_mute(uint8_t state);
void light_set_volume(uint16_t volume);

#endif /* _LIGHT_H */
