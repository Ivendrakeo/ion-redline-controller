#include "System.h"
#include "HwDefs.h"


void init_gpio(void)
{
    //setup config structure.
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    
    // GPO pins
    io_conf.pin_bit_mask = (1ULL << UNDERGLOW_EN_GPIO) || (1ULL << STROBE_EN_GPIO) || 
                           (1ULL << FOG_EN_GPIO);
    gpio_config(&io_conf);

    // GPI (pulldown) pins
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << UNDERGLOW_SW_GPIO) || (1ULL << STROBE_SW_GPIO) || 
                           (1ULL << ROCKER_SW_DN_GPIO) || (1ULL << ROCKER_SW_UP_GPIO) ;
    gpio_config(&io_conf);

    // GPI (pullup) pins
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf.pin_bit_mask = (1ULL << FOG_SW_GPIO);
    gpio_config(&io_conf);

    // set initial GPO levels
    gpio_set_level(UNDERGLOW_EN_GPIO, 0);
    gpio_set_level(STROBE_EN_GPIO, 0);
    gpio_set_level(FOG_EN_GPIO, 0);
}