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

uint8_t layer = 0;

static uint8_t layer_active = 0;
static uint8_t layer_nextkey = 0;
static uint8_t layer_previous = 0;

static void
layer_advance(uint8_t next)
{
    layer_previous = layer;
    layer = next % LAYERS_NUM;
}

static void
layer_return()
{
    layer = layer_previous;
}

void
layer_event(event_t *event, bool pressed)
{
	uint8_t action = event->layer.action;
	uint8_t number = event->layer.number;

    elog("layer %02x %02x %d", event->layer.action, event->layer.number, pressed);

    if (layer_active) {
        elog("ignoring layer event: already active");
        return;
    }

    if (pressed) {
        layer_active = 1;
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
                layer_active = 0;
                break;
            case LAYER_HOLD:
                layer_advance(number);
                break;
        }
    } else {
        switch (action) {
            case LAYER_MOVE:
            case LAYER_UP:
            case LAYER_DOWN:
                layer_active = 0;
                break;

            case LAYER_NEXTKEY:
                layer_advance(number);
                layer_nextkey = 1;
                layer_active = 1;
                break;

            case LAYER_HOLD:
                layer_return();
                layer_active = 0;
                break;
        }
    }
}

void
layer_use_event(event_t *event, bool pressed)
{
    if ((! pressed) && layer_nextkey) {
        layer_nextkey = 0;
        layer_active = 0;
        layer_return();
    }
}
