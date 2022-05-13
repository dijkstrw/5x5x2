#ifndef _RGBMAP_H
#define _RGBMAP_H

#include "rgbpixel.h"

typedef struct {
    uint16_t h;
    uint8_t s;
    uint8_t v;
} __attribute__ ((packed)) hsv_t;

#define HUE_SEXTANT                0x100
#define SEXTANT_MAX                6
#define HUE_MAX                    ((SEXTANT_MAX * HUE_SEXTANT) - 1)

/* Temporary effects to troubleshoot code */
void rainbow_init(void);
void rainbow_advance(void);
void hsv2rgb(hsv_t *h, rgbpixel_t *p);

#endif
