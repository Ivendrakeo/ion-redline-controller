/* hardware pinout defines */
#ifndef _HWDEFS_H
#define _HWDEFS_H

#include "driver/gpio.h"

// underglow
#define UNDERGLOW_RMT_GPIO         (GPIO_NUM_7)
#define UNDERGLOW_SW_GPIO             (GPIO_NUM_20) // deutsch (6pin) pin 5

// strobe
#define STROBE_RMT_GPIO            (GPIO_NUM_0)
#define STROBE_EN_GPIO             (GPIO_NUM_5)
#define STROBE_SW_GPIO             (GPIO_NUM_23) // deutsch (6pin) pin 2

// fog lights
#define FOG_EN_GPIO             (GPIO_NUM_6) 
#define FOG_SW_GPIO             (GPIO_NUM_19) // TBD? 

// rocker sw (for underglow only atm?)
#define ROCKER_SW_DN_GPIO             (GPIO_NUM_21) // deutsch (6pin) pin 4
#define ROCKER_SW_UP_GPIO             (GPIO_NUM_22) // deutsch (6pin) pin 3

#endif // _HWDEFS_H