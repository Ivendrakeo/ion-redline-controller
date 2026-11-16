// std lib include
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// project includes
#include "AppDefs.h"
#include "HwDefs.h"
#include "System.h"
#include "LedStripEncoder.h"
#include "LedStripDriver.h"
#include "StrobeDriver.h"



static const char *TAG = "ION_REDLINE_CONTROLLER";


void app_main(void)
{
    

    init_led_strip_driver();
    init_gpio();
    init_strobe();
    int cnt = 0;

    while (1) {
        
        
        update_rainbow_chase();
        cnt++;

        // test pattern: walk one strobe on at a time
        StrobeState strobes = { .allStrobes = (uint8_t)(0x10 << (cnt % 4)) };
        strobe_set(strobes);
        gpio_set_level(SYSTEM_GPIO_OUTPUT, (cnt % 10) ? 0 : 1);
    }
}
