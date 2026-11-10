/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <stdint.h>
#include "driver/rmt_encoder.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Bit timing of the LED control IC's one-wire protocol
 */
typedef struct {
    uint32_t t0h_ns;   /*!< High time of a 0 bit, in ns */
    uint32_t t0l_ns;   /*!< Low time of a 0 bit, in ns */
    uint32_t t1h_ns;   /*!< High time of a 1 bit, in ns */
    uint32_t t1l_ns;   /*!< Low time of a 1 bit, in ns */
    uint32_t reset_us; /*!< Low time that latches the data, in us */
} led_strip_timing_t;

/**
 * @brief WS2811 external control IC, 800 kHz (high-speed) mode.
 *        Reset is 280us to cover newer WS2811 revisions (older ones need only 50us).
 */
#define LED_STRIP_TIMING_WS2811 ((led_strip_timing_t) { \
    .t0h_ns = 250, .t0l_ns = 1000, .t1h_ns = 600, .t1l_ns = 650, .reset_us = 280, \
})

/**
 * @brief FW1903 external control IC, 800 kHz (1.25us bit period).
 *        Datasheet: T0H 320-400ns, T1H 640-800ns, reset >= 200us; 250us used for margin.
 */
#define LED_STRIP_TIMING_FW1903 ((led_strip_timing_t) { \
    .t0h_ns = 360, .t0l_ns = 890, .t1h_ns = 720, .t1l_ns = 530, .reset_us = 250, \
})

/**
 * @brief WS2812/WS2812B integrated 5050 LED
 */
#define LED_STRIP_TIMING_WS2812 ((led_strip_timing_t) { \
    .t0h_ns = 300, .t0l_ns = 900, .t1h_ns = 900, .t1l_ns = 300, .reset_us = 50, \
})

/**
 * @brief Type of led strip encoder configuration
 */
typedef struct {
    uint32_t resolution;       /*!< Encoder resolution, in Hz */
    led_strip_timing_t timing; /*!< Bit timing of the LED control IC */
} led_strip_encoder_config_t;

/**
 * @brief Create RMT encoder for encoding LED strip pixels into RMT symbols
 *
 * @param[in] config Encoder configuration
 * @param[out] ret_encoder Returned encoder handle
 * @return
 *      - ESP_ERR_INVALID_ARG for any invalid arguments
 *      - ESP_ERR_NO_MEM out of memory when creating led strip encoder
 *      - ESP_OK if creating encoder successfully
 */
esp_err_t rmt_new_led_strip_encoder(const led_strip_encoder_config_t *config, rmt_encoder_handle_t *ret_encoder);

#ifdef __cplusplus
}
#endif
