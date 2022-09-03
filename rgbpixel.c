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
 * Drive a string of rgbpixels (sk68xx, sk98xx, ws28xx) leds using spi
 */

/*
 *
 * sk68xx need a string of "bits" clocked out every 1.25µs ± 600ns
 * - a 0 is signalled as "▇▁▁"
 * - a 1 is signalled as "▇▇▁"
 * - each sk6812 needs 8 of these words for R, G, B
 * - a reset pulse clears the light we are addressing, and needs to be
 *   longer than 80µs
 *
 * Looking for SPI divisor; main clock is 72MHz. If we divide the
 * led-word into three bits we need 3 times the data transmission speed
 * required by the leds.
 *
 * - Data transmission time is 1.25µs ± 600ns = [  650ns,  1.85µs]
 * - Divide by 3 to get our per spi bit rate =  [  217ns,   617ns]
 * - To Hz                                   =  [4.61Mhz, 1.62Mhz]
 * - SPI CLK divisor must be 32
 * - then one spi bit takes 1/2.25MHz = 444ns, and 3 take = 1.33µs
 */

#include <limits.h>
#include <stdint.h>
#include <string.h>

#include <libopencm3/cm3/common.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/spi.h>

#include "config.h"
#include "rgbpixel.h"

#define RESET_PULSE_NS        80000
#define RESET_SPI_RATE_NS       444
#define RESET_SPI_COUNT       (RESET_PULSE_NS / RESET_SPI_RATE_NS)

typedef struct {
    uint8_t g[3];
    uint8_t r[3];
    uint8_t b[3];
} __attribute__ ((packed)) spipixel_t;

typedef enum {
    TX_RESET_PULSE = 0,
    TX_PIXEL_ARRAY,
} spistatus_t;

rgbpixel_t frame[RGB_ALL_NUM];
static spipixel_t spi[2][RGB_ALL_NUM];
static volatile uint8_t active_buffer = 0;
static volatile spistatus_t status = TX_RESET_PULSE;
static uint32_t resetdata = 0;

static void
rgbpixel_reset()
{
    uint8_t i, j;

    memset(&frame, 0, sizeof(frame));
    memset(&spi, 0, sizeof(spi));
    active_buffer = 0;

    /*
     * Setup the bits that are always on, i.e. the start of a new signalling frame.
     */
    for (i = 0; i < 2; i++) {
        for (j = 0; j < RGB_ALL_NUM; j++) {
            spi[i][j].g[0] = spi[i][j].r[0] = spi[i][j].b[0] = 0b10010010;
            spi[i][j].g[1] = spi[i][j].r[1] = spi[i][j].b[1] = 0b01001001;
            spi[i][j].g[2] = spi[i][j].r[2] = spi[i][j].b[2] = 0b00100100;
        }
    }
}

static void
rgbpixel_send_pixel_array()
{
    dma_channel_reset(DMA_IF, DMA_CHANNEL);

    dma_set_peripheral_address(DMA_IF, DMA_CHANNEL, (uint32_t)&SPI_DATA);
    dma_set_memory_address(DMA_IF, DMA_CHANNEL, (uint32_t)&spi[active_buffer][0]);
    dma_set_number_of_data(DMA_IF, DMA_CHANNEL, sizeof(spi[0]));
    dma_set_read_from_memory(DMA_IF, DMA_CHANNEL);
    dma_enable_memory_increment_mode(DMA_IF, DMA_CHANNEL);
    dma_set_peripheral_size(DMA_IF, DMA_CHANNEL, DMA_CCR_PSIZE_8BIT);
    dma_set_memory_size(DMA_IF, DMA_CHANNEL, DMA_CCR_MSIZE_8BIT);
    dma_set_priority(DMA_IF, DMA_CHANNEL, DMA_CCR_PL_LOW);

    dma_enable_transfer_complete_interrupt(DMA_IF, DMA_CHANNEL);
    dma_enable_channel(DMA_IF, DMA_CHANNEL);

    spi_enable(SPI_IF);
    spi_enable_tx_dma(SPI_IF);
}

static void
rgbpixel_send_reset_pulse()
{
    dma_channel_reset(DMA_IF, DMA_CHANNEL);

    dma_set_peripheral_address(DMA_IF, DMA_CHANNEL, (uint32_t)&SPI_DATA);
    dma_set_memory_address(DMA_IF, DMA_CHANNEL, (uint32_t)&resetdata);
    dma_set_number_of_data(DMA_IF, DMA_CHANNEL, RESET_SPI_COUNT);
    dma_set_read_from_memory(DMA_IF, DMA_CHANNEL);
    dma_set_peripheral_size(DMA_IF, DMA_CHANNEL, DMA_CCR_PSIZE_8BIT);
    dma_set_memory_size(DMA_IF, DMA_CHANNEL, DMA_CCR_MSIZE_8BIT);
    dma_set_priority(DMA_IF, DMA_CHANNEL, DMA_CCR_PL_LOW);

    dma_enable_transfer_complete_interrupt(DMA_IF, DMA_CHANNEL);
    dma_enable_channel(DMA_IF, DMA_CHANNEL);

    spi_enable(SPI_IF);
    spi_enable_tx_dma(SPI_IF);
}

void
rgbpixel_init()
{
    rcc_periph_clock_enable(SPI_RCC);
    rcc_periph_clock_enable(SPI_GPIO_RCC);
    rcc_periph_clock_enable(DMA_RCC);
    rcc_periph_clock_enable(RCC_AFIO);

    /* Setup SPI */
    gpio_set_mode(SPI_GPIO,
                  GPIO_MODE_OUTPUT_50_MHZ,
                  GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
                  SPI_BV);

    spi_reset(SPI_IF);
    spi_init_master(SPI_IF, SPI_CR1_BAUDRATE_FPCLK_DIV_32,
                    SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
                    SPI_CR1_CPHA_CLK_TRANSITION_2,
                    SPI_CR1_DFF_8BIT,
                    SPI_CR1_MSBFIRST);
    spi_enable_software_slave_management(SPI_IF);
    spi_set_nss_high(SPI_IF);

    /* Setup DMA */
    nvic_set_priority(DMA_IRQ, 10);
    nvic_enable_irq(DMA_IRQ);

    /* Start sending data */
    rgbpixel_reset();
    rgbpixel_send_reset_pulse();
}

void
DMA_ISR_FUNCTION()
{
    spi_disable_tx_dma(SPI_IF);

        /* Wait to transmit last data */
    while (!(SPI_SR(SPI_IF) & SPI_SR_TXE));
    while ((SPI_SR(SPI_IF) & SPI_SR_BSY));

    spi_disable(SPI_IF);

    dma_disable_transfer_complete_interrupt(DMA_IF, DMA_CHANNEL);
    dma_disable_channel(DMA_IF, DMA_CHANNEL);

    if (status == TX_RESET_PULSE) {
        status = TX_PIXEL_ARRAY;
        rgbpixel_send_pixel_array();
    } else {
        status = TX_RESET_PULSE;
        rgbpixel_send_reset_pulse();
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
 */
#define BBOFFSET(byte, bit)  (byte << 5 | bit << 2)

void
rgbpixel_render()
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
     * spibuffer[0].g = bits ₁7₀₁6₀₁5 ₀₁4₀₁3₀₁ 2₀₁1₀₁0₀
     */

    for (i = 0; i < RGB_ALL_NUM; i++) {
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

    /* Ensure SPI takes rendered buffer for next transmission */
    active_buffer = inactive_buffer;
}

void
rgbpixel_set(uint8_t n, uint8_t r, uint8_t g, uint8_t b)
{
    if (n < RGB_ALL_NUM) {
        frame[n].g = g;
        frame[n].r = r;
        frame[n].b = b;
    }
}
