/*
 * Copyright (c) 2015-2021 by Willem Dijkstra <wpd@xs4all.nl>.
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
 * Keymap
 *
 * Holds the keymap and code to take on press and release events from the
 * matrix. These can then be passed on to the keyboard, mouse or extrakey
 * modules.
 */
#include <stdint.h>
#include <string.h>

#include "automouse.h"
#include "config.h"
#include "elog.h"
#include "extrakey.h"
#include "keyboard.h"
#include "keymap.h"
#include "layer.h"
#include "macro.h"
#include "mouse.h"
#include "rgbease.h"
#include "serial.h"
#include "usb_keycode.h"

event_t keymap[LAYERS_NUM][ROWS_NUM][COLS_NUM] =
{
    {
        { _K(F13), _K(F14), _K(F15), _K(F16), _K(MUTE) },
        { _K(F17), _K(F18), _K(F19), _K(F20), _K(F21) },
        { _KM(RCTRL, LEFT), _K(LEFT), _C(PLAY), _K(RIGHT), _KM(RCTRL, RIGHT) },
        { _K(APP), _K(INTER2), _K(INTER4), _K(INTER5), _K(LANG1) },
        { _L(MOVE, 1), _L(NEXTKEY, 2), _K(F22), _K(F23), _MA(0) }
    },
    {
        { _K(PAD_MINUS), _K(NUM_LOCK), _K(PAD_SLASH), _K(PAD_ASTERISK), _K(MUTE) },
        { _C(AL_FILEBROWSER), _K(PAD_7), _K(PAD_8), _K(PAD_9), _K(PAD_PLUS) },
        { _C(AL_INTERNETBROWSER), _K(PAD_4), _K(PAD_5), _K(PAD_6), _K(PAD_PLUS) },
        { _C(AL_EMAILREADER), _K(PAD_1), _K(PAD_2), _K(PAD_3), _K(PAD_ENTER) },
        { _L(MOVE, 2), _K(PAD_0), _K(PAD_0), _K(PAD_PERIOD), _K(PAD_ENTER) },
    },
    {
        { _MA(1), _MA(2), _MA(3), _MA(4), _K(MUTE) },
        { _MA(5), _MA(6), _MA(7), _MA(8), _MA(9) },
        { _AM(MOUSE_BUTTON1, 20, 1), _AM(0, 1, 5),  _C(VOLUMEINC), _C(VOLUMEDEC), _C(AC_BOOKMARKS) },
        { _C(AL_CALCULATOR), _C(AL_TERMINALLOCK), _C(EJECT), _C(SCANNEXTTRACK), _C(SCANPREVIOUSTRACK) },
        { _L(MOVE, 0), _C(AL_CHAT), _K(LANG2), _K(LANG3), _K(LANG4) },
    },
};

void
keymap_dump()
{
    uint8_t l, r, c;
    event_t *e;

    for (l = 0; l < LAYERS_NUM; l++) {
        printfnl("layer %02x", l);
        for (r = 0; r < ROWS_NUM; r++) {
            printf("row %02x: ", r);
            for (c = 0; c < COLS_NUM; c++) {
                e = keymap_get(l, r, c);
                printf("%01x,%02x,%02x,%02x ",
                       e->type,
                       e->args.num1,
                       e->args.num2,
                       e->args.num3);
            }
            printf("\n\r");
        }
    }
}

event_t *
keymap_get(uint8_t l, uint8_t r, uint8_t c)
{
    if ((l > LAYERS_NUM) ||
        (r > ROWS_NUM) ||
        (c > COLS_NUM)) {
        elog("keymap position out of bounds");
        return 0;
    }

    return &keymap[l][r][c];
}

void
keymap_set(uint8_t l, uint8_t r, uint8_t c, event_t *event)
{
    if ((l >= LAYERS_NUM) ||
        (r >= ROWS_NUM) ||
        (c >= COLS_NUM)) {
        elog("keymap position out of bounds");
        return;
    }

    memcpy(&keymap[l][r][c], event, sizeof(event_t));
}

void
keymap_event(uint16_t row, uint16_t col, bool pressed)
{
    event_t *event = layer_get_event(row, col, pressed);

    rgbease_event(row, col, pressed);

    switch (event->type) {
        case KMT_KEY:
            keyboard_event(event, pressed);
            break;

        case KMT_MOUSE:
            mouse_event(event, pressed);
            break;

        case KMT_AUTOMOUSE:
            automouse_event(event, pressed);
            break;

        case KMT_WHEEL:
            wheel_event(event, pressed);
            break;

        case KMT_CONSUMER:
            extrakey_consumer_event(event, pressed);
            break;

        case KMT_SYSTEM:
            extrakey_system_event(event, pressed);
            break;

        case KMT_LAYER:
            layer_event(row, col, event, pressed);
            break;

        case KMT_MACRO:
            macro_event(event, pressed);
            break;
    }
}

uint8_t
send_event_if_idle(event_t *event, uint8_t press)
{
    switch (event->type) {
        case KMT_KEY:
            if (usb_ep_keyboard_idle && usb_ep_nkro_idle) {
                keyboard_event(event, press);
            } else {
                return 0;
            }
            break;

        case KMT_MOUSE:
            if (usb_ep_mouse_idle) {
                mouse_event(event, press);
            } else {
                return 0;
            }
            break;

        case KMT_AUTOMOUSE:
            if (usb_ep_mouse_idle) {
                automouse_event(event, press);
            } else {
                return 0;
            }
            break;

        case KMT_WHEEL:
            if (usb_ep_mouse_idle) {
                wheel_event(event, press);
            } else {
                return 0;
            }
            break;

        case KMT_CONSUMER:
            if (usb_ep_extrakey_idle) {
                extrakey_consumer_event(event, press);
            } else {
                return 0;
            }
            break;

        case KMT_SYSTEM:
            if (usb_ep_extrakey_idle) {
                extrakey_system_event(event, 1);
            } else {
                return 0;
            }
            break;
    }
    return 1;
}
