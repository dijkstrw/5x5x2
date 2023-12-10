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
 * Layer operations
 *
 * The different ways to switch layers are:
 * - MOVE    - move to a particular layer using a keyboard switch
 * - UP      - move to higher layer
 * - DOWN    - move to lower layer
 * - NEXTKEY - go to new layer for only next keypress
 * - HOLD    - while held goto layer, return on release
 */

#include "config.h"
#include "elog.h"
#include "layer.h"
#include "light.h"

uint8_t layer = 0;

typedef struct {
    uint16_t row;
    uint16_t col;
    uint8_t previous;
    uint8_t active;
    uint8_t nextkey;
} __attribute__ ((packed)) layer_context_t;

static layer_context_t context = {0, 0, 0, 0, 0};

static void
layer_advance(uint8_t next)
{
    context.previous = layer;
    layer = next % LAYERS_NUM;
    light_set_layer(layer);
    elog("layer %02x active", layer);
}

static void
layer_return()
{
    layer = context.previous;
    light_set_layer(layer);
    elog("layer %02x active", layer);
}

event_t *
layer_get_event(uint16_t row, uint16_t col, bool pressed)
{
    event_t *result = &keymap[layer][row][col];

    if ((! pressed) &&
        context.active)
    {
        if ((context.row == row) &&
            (context.col == col)) {
            /*
             * At keydown we can change to a new layer; so considering at
             * keyup only; are we executing some kind of layer action? If so,
             * take the layer event structure from the previous layer as our
             * event
             */
            elog("previous layer keyup detected");
            result = &keymap[context.previous][row][col];
        } else if (context.nextkey) {
            /*
             * If we are only in the layer for the nextkey, then
             * return to the previous layer after that key was seen.
             */
            context.nextkey = false;
            context.active = false;
            layer_return();
        }
    }

    return result;
}

void
layer_event(uint16_t row, uint16_t col, event_t *event, bool pressed)
{
    uint8_t action = event->layer.action;
    uint8_t number = event->layer.number;

    elog("layer %02x %02x %d", event->layer.action, event->layer.number, pressed);

    if (pressed) {
        if (context.active) {
            elog("ignoring layer event: already active");
            return;
        } else {
            context.active = true;
            context.row = row;
            context.col = col;
        }

        switch (action) {
            case LAYER_MOVE:
                layer_advance(number);
                break;
            case LAYER_UP:
                layer_advance(layer + 1);
                break;
            case LAYER_DOWN:
                layer_advance(layer - 1);
                break;
            case LAYER_NEXTKEY:
                context.nextkey = true;
                layer_advance(number);
                break;
            case LAYER_HOLD:
                layer_advance(number);
                break;
        }
    } else {
        if (context.active &&
            ((context.row != row) ||
             (context.col != col))) {
            elog("ignoring layer event: already active");
            return;
        }

        switch (action) {
            case LAYER_MOVE:
            case LAYER_UP:
            case LAYER_DOWN:
                context.active = false;
                break;

            case LAYER_NEXTKEY:
                break;

            case LAYER_HOLD:
                layer_return();
                context.active = false;
                break;
        }
    }
}
