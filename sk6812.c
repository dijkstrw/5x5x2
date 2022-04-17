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
 * Drive a string of sk6812 leds using spi
 */

/*
 *
 * SK6812 need a string of "bits" clocked out every 1.25us ± 600ns
 * - a 0 is signalled as "▇▁▁"
 * - a 1 is signalled as "▇▇▁"
 * - each sk6812 needs 8 of these words for R, G, B
 * - a reset pulse clears the light we are addressing, and needs to be
 *   longer than 80us
 *
 * Looking for SPI divisor; main clock is 72MHz. If we divide the
 * led-word into three bits we need 3 times the data transmission speed
 * required by the leds.
 *
 * - Data transmission time is 1.25µs ± 600ns = [  650ns,  1.85µs]
 * - Divide by 3 to get our per spi bit rate =  [  217ns,   617ns]
 * - To Hz                                   =  [4.61Mhz, 1.62Mhz]
 * - So divisor must be 16 or 32
 *
 */

#include <limits.h>
#include <stdint.h>
#include <string.h>

#include <libopencm3/cm3/common.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/spi.h>

#include "config.h"
#include "sk6812.h"

#define RESET_PULSE_US        80

void
sk6812_init()
{
    rcc_periph_clock_enable(SPI_RCC);
    rcc_periph_clock_enable(SPI_GPIO_RCC);
    gpio_set_mode(SPI_GPIO,
                  GPIO_MODE_OUTPUT_10_MHZ,
                  GPIO_CNF_OUTPUT_PUSHPULL,
                  SPI_BV);

    spi_reset(SPI_IF);
    spi_init_master(SPI_IF, SPI_CR1_BAUDRATE_FPCLK_DIV_32,
                    SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
                    SPI_CR1_CPHA_CLK_TRANSITION_1,
                    SPI_CR1_DFF_8BIT,
                    SPI_CR1_MSBFIRST);
    spi_enable_software_slave_management(SPI_IF);
    spi_set_nss_high(SPI_IF);
}

typedef struct {
    uint8_t g;
    uint8_t r;
    uint8_t b;
} __attribute__ ((packed)) rgbpixel_t;

typedef struct {
    uint8_t g[3];
    uint8_t r[3];
    uint8_t b[3];
} __attribute__ ((packed)) spipixel_t;

static rgbpixel_t frame[BACKLIGHT_LEDS_NUM];
static uint8_t active_buffer = 0;
static spipixel_t spi[2][BACKLIGHT_LEDS_NUM];

void
sk6812_reset()
{
    uint8_t i, j;

    memset(&frame, 0, sizeof(frame));
    memset(&spi, 0, sizeof(spi));
    active_buffer = 0;

    /*
     * Setup the bits that are always on, i.e. the start of a new signalling frame.
     */
    for (i = 0; i < 2; i++) {
        for (j = 0; j < BACKLIGHT_LEDS_NUM; j++) {
            spi[i][j].g[0] = spi[i][j].r[0] = spi[i][j].b[0] = 0b10010010;
            spi[i][j].g[1] = spi[i][j].r[1] = spi[i][j].b[1] = 0b01001001;
            spi[i][j].g[2] = spi[i][j].r[2] = spi[i][j].b[2] = 0b00100100;
        }
    }
}

/*
 * ARM Cortex M3/M4 can bitband; map individual bits of an byte into a
 * separate address range, where the bits are blown up to words to
 * make them easily accessible.
 */

#define SRAM_BASE_BITBAND	(0x22000000U)

/*
 * BBADDR(addr)
 *
 * Given a normal SRAM address, map to the bitband equivalent.
 *
 * Note that BBADDR returns the BB address of addr at bit 0.
 */
#define BBADDR(addr)                                                    \
    (uint8_t *)(((((uint32_t)addr) & 0x0FFFFF) << 5) | SRAM_BASE_BITBAND)

/*
 * BBOFFSET(byte, bit)
 *
 * Calculate the offset to add to a start bitbanded address when we
 * want to access a bit in a byte further on.
 *
 */
#define BBOFFSET(byte, bit)  (byte << 5 | bit << 2)

void
sk6812_render()
{
    uint8_t inactive_buffer = (active_buffer ^ 1);
    uint8_t *out = BBADDR(&spi[inactive_buffer][0]);
    uint8_t *in = BBADDR(&frame[0]);
    uint8_t i, j;

    /*
     * Our goal is to get the bits with light info from the
     * framebuffer into the inactive spi buffer.
     *
     * framebuffer[0].g = bits 76543210
     * spibuffer[0].g = bits x7xx6xx5 xx4xx3xx 2xx1xx0x
     */

    for (i = 0; i < BACKLIGHT_LEDS_NUM; i++) {
        for (j = 0; j < sizeof(rgbpixel_t); j++) {
            MMIO8(out + BBOFFSET(2, 1)) = MMIO8(in + BBOFFSET(0, 0));
            MMIO8(out + BBOFFSET(2, 4)) = MMIO8(in + BBOFFSET(0, 1));
            MMIO8(out + BBOFFSET(2, 7)) = MMIO8(in + BBOFFSET(0, 2));
            MMIO8(out + BBOFFSET(1, 2)) = MMIO8(in + BBOFFSET(0, 3));
            MMIO8(out + BBOFFSET(1, 5)) = MMIO8(in + BBOFFSET(0, 4));
            MMIO8(out + BBOFFSET(0, 0)) = MMIO8(in + BBOFFSET(0, 5));
            MMIO8(out + BBOFFSET(0, 3)) = MMIO8(in + BBOFFSET(0, 6));
            MMIO8(out + BBOFFSET(0 ,6)) = MMIO8(in + BBOFFSET(0, 7));

            /* Jump to next color, 1 byte further for in, 3 for out  */
            in += BBOFFSET(1, 0);
            out += BBOFFSET(3, 0);
        }
    }
}

void
sk6812_set(uint8_t n, uint8_t r, uint8_t g, uint8_t b)
{
    if (n < BACKLIGHT_LEDS_NUM) {
        frame[n].g = g;
        frame[n].r = r;
        frame[n].b = b;
    }
}
