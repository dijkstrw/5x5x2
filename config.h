/*
 * Copyright (c) 2015-2022 by Willem Dijkstra <wpd@xs4all.nl>.
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

#ifndef _CONFIG_H
#define _CONFIG_H

#include "usb.h"

#define USB_GPIO              GPIOA
#define USB_RCC               RCC_GPIOA
#define USB_BV                (GPIO12)
#define SERIAL_BUF_SIZEIN     160
#define SERIAL_BUF_SIZEOUT    1024

/*
 * Matrix pinout definition:
 *
 * PA0-A4 = row driver
 * PB0-B2, B6-B7 = column reader
 *
 * COLS_DECODE is used after column GPIO reading to get a consecutive bitfield
 */

#define ROWS_NUM              5
#define ROWS_GPIO             GPIOA
#define ROWS_RCC              RCC_GPIOA
#define ROWS_BV               0b11111

#define COLS_NUM              5
#define COLS_GPIO             GPIOB
#define COLS_RCC              RCC_GPIOB
#define COLS_BV               0b11000111
#define COLS_DECODE(x)        (((x >> 3) & 0b11000) | (x & 0b111))

/*
 * Timeouts:
 *
 * Debounce, how long does a key need to be down to be pressed
 * Enumerate, how long may enumeration take before reset
 * Ease, how often are the rgbleds updated
 */
#define MS_DEBOUNCE           10
#define MS_ENUMERATE          5000
#define MS_EASE               1

/*
 * Number of layers possible in keymap definition
 */
#define LAYERS_NUM            3

/*
 * Number of macro keys, and max len of a macro sequence
 */
#define MACRO_MAXKEYS         12
#define MACRO_MAXLEN          32

/*
 * Amount of userflash to be used to store the configuration.
 *
 * STM32 flash page size depends on the device:
 * stm32f103xx, with < 128k flash = 1k
 */
#define FLASH_PAGE_NUM        4
#define FLASH_PAGE_SIZE       0x400

#define LEDS_GPIO             GPIOB
#define LEDS_RCC              RCC_GPIOB
#define LEDS_BV               (GPIO12 | GPIO13 | GPIO14 | GPIO15)
#define LED1IO                GPIO12
#define LED2IO                GPIO13
#define LED3IO                GPIO14
#define LED4IO                GPIO15

#define AUTOMOUSE_LED_ACTIVE  (1<<2)
#define AUTOMOUSE_LED_PRESS   (1<<1)
#define MACRO_LED_ACTIVE      (1<<2)

/*
 * Backlight
 */
#define SPI_RCC               RCC_SPI1
#define SPI_GPIO_RCC          RCC_GPIOA
#define SPI_GPIO              GPIOA
#define SPI_BV                GPIO7
#define SPI_IF                SPI1
#define SPI_DATA              SPI1_DR

#define DMA_RCC               RCC_DMA1
#define DMA_IF                DMA1
#define DMA_CHANNEL           DMA_CHANNEL3
#define DMA_IRQ               NVIC_DMA1_CHANNEL3_IRQ
#define DMA_ISR_FUNCTION      dma1_channel3_isr

#define RGB_KEYS_NUM          (COLS_NUM * ROWS_NUM)
#define RGB_BACKLIGHT_NUM     8
#define RGB_ALL_NUM           (RGB_KEYS_NUM + RGB_BACKLIGHT_NUM)
#define RGB_BACKLIGHT_OFFSET  RGB_KEYS_NUM

/*
 * Rotary encoder
 */
#define ROT_GPIO_RCC          RCC_GPIOA
#define ROT_GPIO              GPIOA
#define ROT_TIM_RCC           RCC_TIM1
#define ROT_TIM               TIM1
#define ROT_BV                (GPIO8 | GPIO9)
#define ROT_PERIOD            65535


#endif /* _CONFIG_H */
