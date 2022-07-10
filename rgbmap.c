#include "elog.h"
#include "config.h"
#include "rgbmap.h"

/* Map location of a rgb led to a key */

uint8_t key2rgb[ROWS_NUM][COLS_NUM] =
{
    {  0,  1,  2,  3,  4 },
    {  5,  6,  7,  8,  9 },
    { 10, 11, 12, 13, 14 },
    { 15, 16, 17, 18, 19 },
    { 20, 21, 22, 23, 24 }
};

/*
 * Light movements come in a number of forms:
 * - set the backlight to a particular color (can be per key, per layer)
 * - move from current color to a new color in a number of steps
 */

typedef struct {
    rgbpixel_t backlight;
} a_background;

typedef struct {
    rgbpixel_t source;
    rgbpixel_t destination;
    uint8_t step;
} a_ease;

static hsv_t rainbow[BACKLIGHT_LEDS_NUM];

void
rainbow_init() {
    uint8_t i;
    for (i = 0; i < BACKLIGHT_LEDS_NUM; i++) {
        rainbow[i].h = 0;
        rainbow[i].s = 0xff;
        rainbow[i].v = 0xff;
    }
}

void
rainbow_advance() {
    uint8_t i;
    for (i = 0; i < BACKLIGHT_LEDS_NUM; i++) {
        rainbow[i].h += 3;
        rainbow[i].h %= HUE_MAX;

        hsv2rgb(&rainbow[i], &frame[i]);
    }
    rgbpixel_render();
}

/* hsv2rgb using integer arithmetic
 *
 * See https://www.vagrearg.org/content/hsvrgb for an excellent
 * explanation by B. Stultiens.
 */
void
hsv2rgb(hsv_t *h, rgbpixel_t *p)
{
    uint8_t sextant;
    uint8_t *r = &p->r;
    uint8_t *g = &p->g;
    uint8_t *b = &p->b;
    uint8_t *t;
    uint16_t bottom_level;
    uint8_t h_fraction;
    uint32_t slope;

#define swap(a, b) do {t = a; a = b; b = t; } while (0)

    /* monochromatic test */
    if (!h->s) {
        *r = *g = *b = h->v;
        return;
    }

    sextant = h->h >> 8;
    sextant %= SEXTANT_MAX;

    /* Looking at the hsv to rgb picture:
     *
     *      1|
     *       |
     *      V|-r-.-g-.-g-.-b-.-b-.-r-.
     *       |  / \     / \     / \
     *       | g   r   b   g   r   b
     *       |/     \ /     \ /     \
     * V(1-S)|-b-.-b-.-r-.-r-.-g-.-g-.
     *       |
     *      0|_________________________
     *        0 60  120 180 240 300 360
     *
     * - one of the rgb colors is at h->v
     * - one of the rgb colors is at h->v * (1 - h->s)
     * - one of the rgb colors is enroute up or down
     *
     * Sextant one is our standard, and when in other sextants we
     * rearrange the pointers so that we can calculate like we were in
     * sextant one.
     */
    switch (sextant) {
        case 0:
            swap(r, g);
            break;
        case 1:
            break;
        case 2:
            swap(r, b);
            break;
        case 3:
            swap(r, g);
            swap(g, b);
            break;
        case 4:
            swap(g, b);
            break;
        case 5:
            swap(r, g);
            swap(r, b);
            break;
    }

    *g = h->v;

    bottom_level = h->v * (0xff - h->s);
    bottom_level += 1;
    bottom_level += bottom_level >> 8;

    *b = bottom_level >> 8;

    h_fraction = h->h & 0xff;

    if (sextant & 1) {
        /* slope down */
        slope = h->v * (uint32_t)((0xff << 8) - (uint16_t)(h->s * h_fraction));
    } else {
        /* slope up */
        slope = h->v * (uint32_t)((0xff << 8) - (uint16_t)(h->s * (0x100 - h_fraction)));
    }

    slope += slope >> 8;
    slope += h->v;
    *r = slope >> 16;
}
