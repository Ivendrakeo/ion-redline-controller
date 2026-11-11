/* Implementation for using LED strip */
#ifndef _LEDSTRIPDRIVER_H
#define _LEDSTRIPDRIVER_H

// project includes
#include "LedStripEncoder.h"

#define RMT_LED_STRIP_RESOLUTION_HZ (10000000) // 10MHz resolution, 1 tick = 0.1us (led strip needs a high resolution)

#define RMT_LED_STRIP_TIMING        LED_STRIP_TIMING_FW1903

// Byte position of each color within a pixel as sent on the wire.
// FW1903 sends OUTR/OUTG/OUTB data in that order (RGB), but strip vendors often wire the 3535 LEDs differently (e.g. BRG, GRB):
// if colors come out wrong, swap these.
#define LED_STRIP_R_OFFSET          (1)
#define LED_STRIP_G_OFFSET          (2)
#define LED_STRIP_B_OFFSET          (0)

// Number of addressable pixels = number of control ICs, not LEDs.
// On 12V strips one external IC typically drives a group of 3 LEDs; on 5V strips usually 1 LED.
#define EXAMPLE_LED_NUMBERS         (160)
#define EXAMPLE_CHASE_SPEED_MS      (100)

void init_led_strip_driver(void);

void led_strip_solid(void);
void update_rainbow_chase(void);

#endif // _LEDSTRIPDRIVER_H