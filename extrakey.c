/*
 * Copyright (c) 2015-2016 by Willem Dijkstra <wpd@xs4all.nl>.
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
 * Extrakey
 *
 * Receive extrakey events and process them into usb communication with the
 * host.
 */

#include "elog.h"
#include "extrakey.h"

uint8_t extrakey_idle = 0;

static report_extrakey_t state;

report_extrakey_t *
extrakey_report()
{
    return &state;
}

void
extrakey_consumer_event(event_t *event, bool pressed)
{
    state.id = REPORTID_CONSUMER;

    elog("extrakey consumer %04x %d", event->extra.code, pressed);

    if (pressed) {
        state.codel = event->extra.code & 0xff;
        state.codeh = event->extra.code >> 8;
    } else {
        state.codel = state.codeh = 0;
    }

    usb_update_extrakey(&state);
}

void
extrakey_system_event(event_t *event, bool pressed)
{
    state.id = REPORTID_SYSTEM;

    elog("extrakey system %04x %d", event->extra.code, pressed);

    if (pressed) {
        state.codel = event->extra.code & 0xff;
        state.codeh = event->extra.code >> 8;
    } else {
        state.codel = state.codeh = 0;
    }

    usb_update_extrakey(&state);
}
