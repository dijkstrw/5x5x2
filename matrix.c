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

/*
 * Matrix
 *
 * Tasks:
 * - scan the hardware matrix, taking into account transmission lines
 *   and debouncing the result
 * - determine if there is a keydown or keyup event
 * - if so, trigger keyboard, mouse, extrakey events
 */

#include <stdlib.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#include "clock.h"
#include "config.h"
#include "serial.h"
#include "matrix.h"
#include "keymap.h"

static uint32_t debounce[ROWS_NUM];
static bool update[ROWS_NUM];
matrix_t matrix;
static matrix_t matrix_debounce;
static matrix_t matrix_previous;
static uint8_t current;

/*
 * row_select
 *
 * Pull a row high
 */
static void
row_select(uint8_t r)
{
    GPIO_BSRR(ROWS_GPIO) = (1 << r);
}

/*
 * row_clear
 *
 * Pull a row low
 */
static void
row_clear(void)
{
    GPIO_BSRR(ROWS_GPIO) = (ROWS_BV << 16);
}

/*
 * col_read
 *
 * Read a column bits and return them in ascending order. Note that this
 * function hides the physical column wiring.
 */
static uint16_t
col_read(void)
{
    uint16_t c = (uint16_t)(GPIO_IDR(COLS_GPIO) & COLS_BV);

    return COLS_DECODE(c);
}

/*
 * matrix_init
 *
 * Initialize the matrix module. Must be called before any other function in
 * this module is called. Initializes the hardware and module local variables
 * we depend on.
 */
void
matrix_init(void)
{
    uint8_t i;

    rcc_periph_clock_enable(ROWS_RCC);
    rcc_periph_clock_enable(COLS_RCC);
    gpio_set_mode(ROWS_GPIO, GPIO_MODE_OUTPUT_10_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, ROWS_BV);
    gpio_set_mode(COLS_GPIO, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, COLS_BV);

    /* Enable pulldowns on column pins */
    gpio_clear(COLS_GPIO, COLS_BV);

    row_clear();

    for (i = 0; i < ROWS_NUM; i++) {
        matrix.row[i] = 0;
        matrix_debounce.row[i] = 0;
        matrix_previous.row[i] = 0;
        debounce[i] = 0;
    }

    current = 0;
    row_select(current);
}

/*
 * matrix_row_scan
 *
 * Scan the matrix a row at a time.
 *
 * A row was selected and set high in the previous call to matrix
 * scan, and the next call will scan the columns. This time between
 * row set and column read allows for the signal to stabilize
 * (necessary for long tracks or bad termination). Debounce by making
 * sure that a column (set) is stable for at least MS_DEBOUNCE ms.
 */
void
matrix_row_scan()
{
    uint16_t col;

    col = col_read();
    if (matrix_debounce.row[current] != col) {
        matrix_debounce.row[current] = col;
        update[current] = true;
        debounce[current] = timer_set(MS_DEBOUNCE);
    }

    if (update[current] && timer_passed(debounce[current])) {
        update[current] = false;
        matrix.row[current] = matrix_debounce.row[current];
    }

    row_clear();
    current +=1;
    current %= ROWS_NUM;
    row_select(current);
}

/*
 * matrix_process
 *
 * Generate key up/down events depending on the current and previous scan
 * state.
 */
void
matrix_row_process()
{
    uint8_t r, c;
    uint16_t col, colbit;

    /* Make sure that we pick up new scan events */
    r = current;
    matrix_row_scan();

    /*
     * Check for scan events, even if the previous matrix scan returned
     * nothing. We might still have some unprocessed events in our previous
     * matrix.
     */

    col = matrix.row[r] ^ matrix_previous.row[r];
    if (col) {
        for (c = 0; c < COLS_NUM; c++) {
            colbit = (1 << c);
            if (col & colbit) {
                keymap_event(r, c, matrix.row[r] & colbit);
                matrix_previous.row[r] ^= colbit;
            }
        }
    }
}
