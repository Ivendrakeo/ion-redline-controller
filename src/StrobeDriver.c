// ESP32 includes
#include "esp_log.h"
#include "driver/rmt_tx.h"
#include "soc/soc_caps.h"

#include "freertos/FreeRTOS.h"

// project includes
#include "StrobeDriver.h"
#include "HwDefs.h"

// Derived timing, in RMT ticks
#define STROBE_PERIOD_TICKS     (STROBE_RMT_RESOLUTION_HZ / STROBE_PULSE_FREQ_HZ)
#define STROBE_ON_HIGH_TICKS    (STROBE_PERIOD_TICKS * STROBE_DUTY_ON_PCT / 100)
#define STROBE_OFF_HIGH_TICKS   (STROBE_PERIOD_TICKS * STROBE_DUTY_OFF_PCT / 100)
#define STROBE_GAP_TICKS        ((uint32_t)STROBE_FRAME_GAP_US * (STROBE_RMT_RESOLUTION_HZ / 1000000))

// RMT symbol durations are 15 bits wide, and a zero duration ends the transmission
#define RMT_MAX_DURATION        (0x7FFF)

_Static_assert(STROBE_RMT_RESOLUTION_HZ % STROBE_PULSE_FREQ_HZ == 0, "period must be a whole number of ticks");
_Static_assert(STROBE_DUTY_ON_PCT  > 0 && STROBE_DUTY_ON_PCT  < 100, "on duty must be 1-99%");
_Static_assert(STROBE_DUTY_OFF_PCT > 0 && STROBE_DUTY_OFF_PCT < 100, "off duty must be 1-99%");
_Static_assert(STROBE_PERIOD_TICKS <= RMT_MAX_DURATION, "pulse period too long for RMT symbol");
_Static_assert(STROBE_GAP_TICKS >= 2 && STROBE_GAP_TICKS <= 2 * RMT_MAX_DURATION, "frame gap out of range");
_Static_assert(STROBE_PULSE_COUNT <= 8, "StrobeState only holds 8 bits");

static const char *TAG = "strobe_driver";

static rmt_channel_handle_t strobe_chan = NULL;
static rmt_encoder_handle_t strobe_encoder = NULL;
static const rmt_transmit_config_t strobe_tx_config = {
    .loop_count = 0,                // one-shot
    .flags.eot_level = 0,           // line idles low after the frame
};

// One symbol per pulse plus one for the trailing gap.
// The copy encoder reads this buffer during transmission, so it must not be
// rewritten until the previous frame is done (strobe_set waits for that).
static rmt_symbol_word_t strobe_frame[STROBE_PULSE_COUNT + 1];


void init_strobe(void)
{
    ESP_LOGI(TAG, "Create strobe RMT TX channel");
    rmt_tx_channel_config_t tx_chan_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .gpio_num = STROBE_RMT_GPIO,
        .mem_block_symbols = SOC_RMT_MEM_WORDS_PER_CHANNEL, // one block; frame is only 9 symbols
        .resolution_hz = STROBE_RMT_RESOLUTION_HZ,
        .trans_queue_depth = 2,
    };
    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &strobe_chan));

    rmt_copy_encoder_config_t copy_encoder_config = {};
    ESP_ERROR_CHECK(rmt_new_copy_encoder(&copy_encoder_config, &strobe_encoder));

    ESP_ERROR_CHECK(rmt_enable(strobe_chan));
}

void strobe_set(StrobeState strobeState)
{
    // Don't touch the frame buffer while the previous frame is still going out
    strobe_wait_done();

    // Unwired pulses (1-4) are always "off"
    uint8_t bits = strobeState.allStrobes & STROBE_USED_MASK;

    for (int i = 0; i < STROBE_PULSE_COUNT; i++) {
        uint32_t high = (bits & (1u << i)) ? STROBE_ON_HIGH_TICKS : STROBE_OFF_HIGH_TICKS;
        strobe_frame[i] = (rmt_symbol_word_t) {
            .level0 = 1,
            .duration0 = high,
            .level1 = 0,
            .duration1 = STROBE_PERIOD_TICKS - high,
        };
    }

    // Idle-low gap, split across both halves of the symbol
    strobe_frame[STROBE_PULSE_COUNT] = (rmt_symbol_word_t) {
        .level0 = 0,
        .duration0 = STROBE_GAP_TICKS / 2,
        .level1 = 0,
        .duration1 = STROBE_GAP_TICKS - STROBE_GAP_TICKS / 2,
    };

    ESP_ERROR_CHECK(rmt_transmit(strobe_chan, strobe_encoder, strobe_frame, sizeof(strobe_frame), &strobe_tx_config));
}

void strobe_wait_done(void)
{
    ESP_ERROR_CHECK(rmt_tx_wait_all_done(strobe_chan, portMAX_DELAY));
}
