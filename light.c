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

#include <stdint.h>
#include <string.h>

#include "config.h"
#include "elog.h"
#include "layer.h"
#include "light.h"
#include "palette.h"
#include "rgbease.h"
#include "rgbmap.h"

lightmap_t lightmap =
{
    .data[0] = {
        { LIGHT_DESKTOP, LIGHT_DESKTOP, LIGHT_DESKTOP, LIGHT_DESKTOP, LIGHT_MUTE       },
        { LIGHT_DESKTOP, LIGHT_DESKTOP, LIGHT_DESKTOP, LIGHT_DESKTOP, LIGHT_DESKTOP    },
        { LIGHT_VOLUME,  LIGHT_VOLUME,  LIGHT_VOLUME,  LIGHT_VOLUME,  LIGHT_VOLUME     },
        { LIGHT_MACRO,   LIGHT_MACRO,   LIGHT_MACRO,   LIGHT_MACRO,   LIGHT_MACRO      },
        { LIGHT_LAYER,   LIGHT_LAYER,   LIGHT_DESKTOP, LIGHT_DESKTOP, LIGHT_BACKLIGHT  },
    },
    {
        { LIGHT_DESKTOP, LIGHT_DESKTOP, LIGHT_DESKTOP, LIGHT_DESKTOP, LIGHT_MUTE       },
        { LIGHT_DESKTOP, LIGHT_DESKTOP, LIGHT_DESKTOP, LIGHT_DESKTOP, LIGHT_DESKTOP    },
        { LIGHT_VOLUME,  LIGHT_VOLUME,  LIGHT_VOLUME,  LIGHT_VOLUME,  LIGHT_VOLUME     },
        { LIGHT_MACRO,   LIGHT_MACRO,   LIGHT_MACRO,   LIGHT_MACRO,   LIGHT_MACRO      },
        { LIGHT_LAYER,   LIGHT_LAYER,   LIGHT_DESKTOP, LIGHT_DESKTOP, LIGHT_BACKLIGHT  },
    },
    {
        { LIGHT_DESKTOP, LIGHT_DESKTOP, LIGHT_DESKTOP, LIGHT_DESKTOP, LIGHT_MUTE       },
        { LIGHT_DESKTOP, LIGHT_DESKTOP, LIGHT_DESKTOP, LIGHT_DESKTOP, LIGHT_DESKTOP    },
        { LIGHT_VOLUME,  LIGHT_VOLUME,  LIGHT_VOLUME,  LIGHT_VOLUME,  LIGHT_VOLUME     },
        { LIGHT_MACRO,   LIGHT_MACRO,   LIGHT_MACRO,   LIGHT_MACRO,   LIGHT_MACRO      },
        { LIGHT_LAYER,   LIGHT_LAYER,   LIGHT_DESKTOP, LIGHT_DESKTOP, LIGHT_BACKLIGHT  },
    },
};

lightstate_t light_state;

void
light_dump()
{
    uint8_t l, r, c;

    for (l = 0; l < LAYERS_NUM; l++) {
        printfnl("layer %02x", l);
        for (r = 0; r < ROWS_NUM; r++) {
            printf("row %02x: ", r);
            for (c = 0; c < COLS_NUM; c++) {
                printf("%c ", lightmap.data[l][r][c]);
            }
            printf("\n\r");
        }
    }
}

void
light_init()
{
    memset(&light_state, 0, sizeof(light_state));
}

void
light_set(uint8_t l, uint8_t r, uint8_t c, uint8_t v)
{
    if ((l >= LAYERS_NUM) ||
        (r >= ROWS_NUM) ||
        (c >= COLS_NUM)) {
        elog("light position out of bounds");
        return;
    }

    lightmap.data[l][r][c] = v;
}

void
light_set_desktop(uint8_t ascreen, uint8_t adisplay)
{
    uint8_t s = ascreen % SCREENS_NUM;

    if (ascreen == 0) {
        for (s = 0; s < SCREENS_NUM; s++) {
            light_state.desktop[s] = adisplay;
        }
    } else {
        light_state.desktop[s - 1] = adisplay;
    }
    light_apply_state(LIGHT_DESKTOP);
}

void
light_set_layer(uint8_t alayer)
{
    light_apply_state(0);
}

void
light_set_mic_mute(uint8_t astate)
{
    light_state.mic_mute = astate;
    light_apply_state(LIGHT_MUTE);
}

void
light_set_mute(uint8_t astate)
{
    light_state.mute = astate;
    light_apply_state(LIGHT_MUTE);
}

void
light_set_volume(uint16_t avolume)
{
    light_state.volume = avolume;
    light_apply_state(LIGHT_VOLUME);
}

void
light_apply_state(uint8_t only_type)
{
    uint8_t r, c;
    uint8_t id, typ;
    uint8_t screen = 0;
    hsv_t color;

    for (r = 0; r < ROWS_NUM; r++) {
        for (c = 0; c < COLS_NUM; c++) {
            typ = lightmap.data[layer][r][c];
            if (only_type && (typ != only_type)) {
                continue;
            }
            id = key2led(r, c);

            switch (typ) {
            case LIGHT_DESKTOP:
                color = palette_get(COLOR_1 + light_state.desktop[screen]);
                rgbease_set(id, color, F_COLOR_HOLD, 0, 0);
                screen = (screen + 1) % SCREENS_NUM;
                break;

            case LIGHT_LAYER:
                color = palette_get(COLOR_6 + layer);
                rgbease_set(id, color, F_COLOR_HOLD, 0, 0);
                break;

            case LIGHT_MUTE:
               if (light_state.mic_mute) {
                   if (light_state.mute) {
                       color = hsv_red;
                   } else {
                       color = hsv_magenta;
                   }
               } else {
                   if (light_state.mute) {
                       color = hsv_yellow;
                   } else {
                       color = hsv_green;
                   }
               }
               rgbease_set(id, color, F_COLOR_HOLD, STEP_FAST, 0);
               break;

            case LIGHT_VOLUME:
                color = hsv_yellow;
                color.h = LIGHT_VOLUME_TO_HUE(light_state.volume);
                rgbease_set(id, color, F_COLOR_HOLD, STEP_FAST, 0);
                break;
            }
        }
    }
}
