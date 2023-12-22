/*
 * Copyright (c) 2021-2023 by Willem Dijkstra <wpd@xs4all.nl>.
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

#include "command.h"
#include "config.h"
#include "elog.h"
#include "flash.h"
#include "keyboard.h"
#include "keymap.h"
#include "light.h"
#include "macro.h"
#include "palette.h"
#include "ring.h"
#include "rgbease.h"
#include "rotary.h"
#include "serial.h"
#include "usb.h"

static uint8_t
hex_digit(uint8_t in)
{
    if (('0' <= in) && (in <= '9')) {
        return (in - '0');
    }
    if (('A' <= in) && (in <= 'F')) {
        return 10 + (in - 'A');
    }
    if (('a' <= in) && (in <= 'f')) {
        return 10 + (in - 'a');
    }
    return 0;
}

static uint8_t
read_hex_8(struct ring *input_ring)
{
    uint8_t c;
    uint16_t result = 0;

    if (ring_read_ch(input_ring, &c) != -1) {
        result = hex_digit(c) << 4;
        if (ring_read_ch(input_ring, &c) != -1) {
            result |= hex_digit(c);
        }
    }
    return result;
}

static void
command_identify(void)
{
    uint8_t i;

    for (i = 0; i < STRI_MAX; i++) {
        printfnl("%02x: %s", i, usb_strings[i]);
    }
}

static void
command_set_backcolor(struct ring *input_ring)
{
    hsv_t dummy = HSV_WHITE;
    uint8_t n;
    uint8_t red, green, blue;

    for (n = RGB_BACKLIGHT_OFFSET; n < RGB_ALL_NUM; n++) {
        rgbease_set(n, dummy, F_OVERRIDE, 0, 0);
        red = read_hex_8(input_ring);
        green = read_hex_8(input_ring);
        blue = read_hex_8(input_ring);
        rgbpixel_set(n, red, green, blue);
    }
}

static void
command_set_color(struct ring *input_ring)
{
    hsv_t dummy = HSV_WHITE;
    uint8_t r, c, n;
    uint8_t red, green, blue;

    for (r = 0; r < ROWS_NUM; r++) {
        for (c = 0; c < COLS_NUM; c++) {
            n = key2led(r, c);

            rgbease_set(n, dummy, F_OVERRIDE, 0, 0);
            red = read_hex_8(input_ring);
            green = read_hex_8(input_ring);
            blue = read_hex_8(input_ring);
            rgbpixel_set(n, red, green, blue);
        }
    }
}

static void
command_set_light_desktop(struct ring* input_ring)
{
    uint8_t adisplay, ascreen;

    ascreen  = read_hex_8(input_ring);
    adisplay = read_hex_8(input_ring);

    light_set_desktop(ascreen, adisplay);
}

static void
command_set_light_mic_mute(struct ring* input_ring)
{
    uint8_t astate = read_hex_8(input_ring);

    light_set_mic_mute(astate);
}

static void
command_set_light_mute(struct ring* input_ring)
{
    uint8_t astate = read_hex_8(input_ring);

    light_set_mute(astate);
}

static void
command_set_light_volume(struct ring* input_ring)
{
    uint16_t avolume = ((read_hex_8(input_ring) << 8) |
                        read_hex_8(input_ring));

    light_set_volume(avolume);
}

static void
command_set_intensity(struct ring *input_ring)
{
    uint8_t aintensity;

    aintensity = read_hex_8(input_ring);

    rgbintensity = (fract8_t)aintensity;
    light_apply_state(LIGHT_ALL);
}

static void
command_set_keymap(struct ring *input_ring)
{
    uint8_t alayer, arow, acolumn;
    event_t event;

    alayer = read_hex_8(input_ring);
    arow = read_hex_8(input_ring);
    acolumn = read_hex_8(input_ring);

    event.type = read_hex_8(input_ring);
    event.args.num1 = read_hex_8(input_ring);
    event.args.num2 = read_hex_8(input_ring);
    event.args.num3 = read_hex_8(input_ring);

    keymap_set(alayer, arow, acolumn, &event);
}

static void
command_set_light(struct ring *input_ring)
{
    uint8_t alayer, arow, acolumn, avalue;

    alayer = read_hex_8(input_ring);
    arow = read_hex_8(input_ring);
    acolumn = read_hex_8(input_ring);
    avalue = read_hex_8(input_ring);

    light_set(alayer, arow, acolumn, avalue);
}

static void
command_set_macro(struct ring *input_ring)
{
    uint8_t number = read_hex_8(input_ring);
    uint8_t buffer[SERIAL_BUF_SIZEIN];
    uint8_t len = 0;
    uint8_t c;

    while (ring_read_ch(input_ring, &c) != -1) {
        if ((c == '\n') ||
            (c == '\r')) {
            if (len) {
                macro_set_phrase(number, (uint8_t *)&buffer, len);
                return;
            } else {
                elog("macro with empty phrase");
            }
       }

        buffer[len++] = c;

        if (len >= sizeof(buffer)) {
            elog("macro len exceeds buffer");
            return;
        }
    }

    elog("macro not closed of with eol");
}

static void
command_set_nkro(struct ring *input_ring)
{
    uint8_t aenable = read_hex_8(input_ring);

    nkro_active = (aenable & 1);
    printfnl("nkro %d", nkro_active);
}

static void
command_set_palette(struct ring *input_ring)
{
    hsv_t color;
    uint8_t anumber;

    anumber = read_hex_8(input_ring);
    color.h = (read_hex_8(input_ring) << 8) | read_hex_8(input_ring);
    color.s = read_hex_8(input_ring);
    color.v = read_hex_8(input_ring);

    palette_set(anumber, color);
}

static void
command_set_rotary(struct ring *input_ring)
{
    uint8_t alayer, adirection;
    event_t event;

    alayer = read_hex_8(input_ring);
    adirection = read_hex_8(input_ring);

    event.type = read_hex_8(input_ring);
    event.args.num1 = read_hex_8(input_ring);
    event.args.num2 = read_hex_8(input_ring);
    event.args.num3 = read_hex_8(input_ring);

    rotary_set(alayer, adirection, &event);
}

void
command_process(struct ring *input_ring)
{
    uint8_t c;

    while (ring_read_ch(input_ring, &c) != -1) {
        switch (c) {
            case CMD_FLASH_CLEAR:
                flash_clear_config();
                break;

            case CMD_FLASH_LOAD:
                flash_read_config();
                break;

            case CMD_FLASH_SAVE:
                flash_write_config();
                break;

            case CMD_IDENTIFY:
                command_identify();
                break;

            case CMD_BACKCOLOR_SET:
                command_set_backcolor(input_ring);
                break;

            case CMD_COLOR_SET:
                command_set_color(input_ring);
                break;

            case CMD_DUMP:
                if (ring_read_ch(input_ring, &c) != -1) {
                    switch (c) {
                        case DUMP_KEYMAP:
                            keymap_dump();
                            break;

                        case DUMP_LIGHT:
                            light_dump();
                            break;

                        case DUMP_ROTARY:
                            rotary_dump();
                            break;

                        case DUMP_PALETTE:
                            palette_dump();
                            break;
                    }
                }
                break;

            case CMD_DISPLAY_SET:
                if (ring_read_ch(input_ring, &c) != -1) {
                    switch (c) {
                        case LIGHT_DESKTOP:
                            command_set_light_desktop(input_ring);
                            break;
                        case LIGHT_MIC_MUTE:
                            command_set_light_mic_mute(input_ring);
                            break;
                        case LIGHT_MUTE:
                            command_set_light_mute(input_ring);
                            break;
                        case LIGHT_VOLUME:
                            command_set_light_volume(input_ring);
                            break;
                    }
                }
                break;

            case CMD_INTENSITY_SET:
                command_set_intensity(input_ring);
                break;

            case CMD_KEYMAP_SET:
                command_set_keymap(input_ring);
                break;

            case CMD_LIGHT_SET:
                command_set_light(input_ring);
                break;

            case CMD_MACRO_CLEAR:
                macro_init();
                break;

            case CMD_MACRO_SET:
                command_set_macro(input_ring);
                break;

            case CMD_NKRO_SET:
                command_set_nkro(input_ring);
                break;

            case CMD_PALETTE_SET:
                command_set_palette(input_ring);
                break;

            case CMD_ROTARY_SET:
                command_set_rotary(input_ring);
                break;

            case '?':
                printfnl("commands:");
                printfnl("i                 - identify");
                printfnl("dt                - dump type: [l]ight, [k]eymap, [r]otary, [p]alette");
                printfnl("B[rrggbb]*8       - set color rgb of bottom layer");
                printfnl("C[rrggbb]*25      - set color rgb of top layer");
                printfnl("Grrccgg           - set light: layer, row, column, group");
                printfnl("Iii               - set backlight / bottom layer intensity");
                printfnl("Kllrrcctta1a2a3   - set keymap layer, row, column, type, arg1-3");
                printfnl("m                 - clear all macro keys");
                printfnl("Mnnstring         - set macro nn with string");
                printfnl("n                 - clear nkro");
                printfnl("N                 - set nkro");
                printfnl("Pnnhhhhssvv       - set palette: number, hue, saturation, value");
                printfnl("Rllddtta1a2a3     - set rotary layer, direction, type, arg1-3");
                printfnl("L                 - load configuration from flash");
                printfnl("S                 - write configuration to flash");
                printfnl("Z                 - erase configuration flash");
                break;

            case '\n':
            case '\r':
                /* remove eols */
                break;

            default:
                /* lost sync; process until newline */
                ring_skip_line(input_ring);
                break;
        }
    }
}
