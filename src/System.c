#include "System.h"
#include "HwDefs.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_timer.h"


// ignore further fog switch edges for this long after a toggle (switch bounce)
#define FOG_SW_DEBOUNCE_US            (50 * 1000)


// pairs each switch input with the enable output it mirrors
typedef struct {
    gpio_num_t sw;
    gpio_num_t en;
} sw_en_pair_t;

static const sw_en_pair_t s_sw_en_pairs[] = {
    { UNDERGLOW_SW_GPIO, UNDERGLOW_EN_GPIO },
    { STROBE_SW_GPIO,    STROBE_EN_GPIO    },
};

// fog output state, toggled by the fog switch
static volatile uint32_t s_fog_en_level = 0;
static volatile int64_t s_fog_last_toggle_us = 0;

// any edge on a switch input: drive the paired enable output to match
// (rising -> high, falling -> low). reading the level instead of tracking
// edges keeps the output correct even if the switch bounces.
static void IRAM_ATTR sw_isr_handler(void *arg)
{
    const sw_en_pair_t *pair = (const sw_en_pair_t *)arg;
    gpio_set_level(pair->en, gpio_get_level(pair->sw));
}

// falling edge on the fog switch: toggle the fog enable output.
// edges within FOG_SW_DEBOUNCE_US of the last toggle are treated as bounce.
static void IRAM_ATTR fog_sw_isr_handler(void *arg)
{
    int64_t now = esp_timer_get_time();
    if (now - s_fog_last_toggle_us < FOG_SW_DEBOUNCE_US) {
        return;
    }
    s_fog_last_toggle_us = now;
    s_fog_en_level = !s_fog_en_level;
    gpio_set_level(FOG_EN_GPIO, s_fog_en_level);
}


void init_gpio(void)
{
    //setup config structure.
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    
    // GPO pins
    io_conf.pin_bit_mask = (1ULL << UNDERGLOW_EN_GPIO) | (1ULL << STROBE_EN_GPIO) | 
                           (1ULL << FOG_EN_GPIO);
    gpio_config(&io_conf);

    // set initial GPO levels
    gpio_set_level(UNDERGLOW_EN_GPIO, 0);
    gpio_set_level(STROBE_EN_GPIO, 0);
    gpio_set_level(FOG_EN_GPIO, 0);

    // GPI (pulldown) pins, no interrupt
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << ROCKER_SW_DN_GPIO) | (1ULL << ROCKER_SW_UP_GPIO);
    gpio_config(&io_conf);

    // GPI (pulldown) pins, interrupt on both edges
    io_conf.intr_type = GPIO_INTR_ANYEDGE;
    io_conf.pin_bit_mask = (1ULL << UNDERGLOW_SW_GPIO) | (1ULL << STROBE_SW_GPIO);
    gpio_config(&io_conf);

    // GPI (pullup) pins, interrupt on falling edge
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf.pin_bit_mask = (1ULL << FOG_SW_GPIO);
    gpio_config(&io_conf);

    // hook up switch -> enable mirroring
    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    for (size_t i = 0; i < sizeof(s_sw_en_pairs) / sizeof(s_sw_en_pairs[0]); i++) {
        const sw_en_pair_t *pair = &s_sw_en_pairs[i];
        ESP_ERROR_CHECK(gpio_isr_handler_add(pair->sw, sw_isr_handler, (void *)pair));
        // sync output to the switch's current state, since no edge fires for it
        gpio_set_level(pair->en, gpio_get_level(pair->sw));
    }

    // fog switch toggles fog enable (starts off)
    ESP_ERROR_CHECK(gpio_isr_handler_add(FOG_SW_GPIO, fog_sw_isr_handler, NULL));
}
