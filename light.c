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
#include "layer.h"
#include "light.h"
#include "palette.h"
#include "rgbease.h"

uint8_t lightmap[LAYERS_NUM][ROWS_NUM][COLS_NUM] =
{
    {
        { LIGHT_DESKTOP, LIGHT_DESKTOP, LIGHT_DESKTOP, LIGHT_DESKTOP, LIGHT_MICROPHONE },
        { LIGHT_DESKTOP, LIGHT_DESKTOP, LIGHT_DESKTOP, LIGHT_DESKTOP, LIGHT_DESKTOP    },
        { LIGHT_SOUND,   LIGHT_SOUND,   LIGHT_SOUND,   LIGHT_SOUND,   LIGHT_SOUND      },
        { LIGHT_MACRO,   LIGHT_MACRO,   LIGHT_MACRO,   LIGHT_MACRO,   LIGHT_MACRO      },
        { LIGHT_LAYER,   LIGHT_LAYER,   LIGHT_DESKTOP, LIGHT_DESKTOP, LIGHT_BACKLIGHT  },
    },
    {
        { LIGHT_DESKTOP, LIGHT_DESKTOP, LIGHT_DESKTOP, LIGHT_DESKTOP, LIGHT_MICROPHONE },
        { LIGHT_DESKTOP, LIGHT_DESKTOP, LIGHT_DESKTOP, LIGHT_DESKTOP, LIGHT_DESKTOP    },
        { LIGHT_SOUND,   LIGHT_SOUND,   LIGHT_SOUND,   LIGHT_SOUND,   LIGHT_SOUND      },
        { LIGHT_MACRO,   LIGHT_MACRO,   LIGHT_MACRO,   LIGHT_MACRO,   LIGHT_MACRO      },
        { LIGHT_LAYER,   LIGHT_LAYER,   LIGHT_DESKTOP, LIGHT_DESKTOP, LIGHT_BACKLIGHT  },
    },
    {
        { LIGHT_DESKTOP, LIGHT_DESKTOP, LIGHT_DESKTOP, LIGHT_DESKTOP, LIGHT_MICROPHONE },
        { LIGHT_DESKTOP, LIGHT_DESKTOP, LIGHT_DESKTOP, LIGHT_DESKTOP, LIGHT_DESKTOP    },
        { LIGHT_SOUND,   LIGHT_SOUND,   LIGHT_SOUND,   LIGHT_SOUND,   LIGHT_SOUND      },
        { LIGHT_MACRO,   LIGHT_MACRO,   LIGHT_MACRO,   LIGHT_MACRO,   LIGHT_MACRO      },
        { LIGHT_LAYER,   LIGHT_LAYER,   LIGHT_DESKTOP, LIGHT_DESKTOP, LIGHT_BACKLIGHT  },
    },
};

lightstate_t light_state;

void light_init()
{
    memset(&light_state, 0, sizeof(light_state));
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
light_apply_state(uint8_t only_type)
{
    uint8_t r, c;
    uint8_t id, typ;
    uint8_t screen = 0;
    hsv_t color;

    for (r = 0; r < ROWS_NUM; r++) {
        for (c = 0; c < COLS_NUM; c++) {
            typ = lightmap[layer][r][c];
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
            }
        }
    }
}