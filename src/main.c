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



static const char *TAG = "ION_REDLINE_CONTROLLER";


void app_main(void)
{
    

    init_led_strip_driver();
    init_gpio();
    int cnt = 0;

    while (1) {
        
        
        update_rainbow_chase();
        cnt++;
        gpio_set_level(SYSTEM_GPIO_OUTPUT, (cnt % 10) ? 0 : 1);
    }
}
