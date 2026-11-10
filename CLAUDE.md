# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

ESP-IDF firmware (C) that drives an addressable LED strip using the RMT peripheral. The strip uses 3535 SMD LEDs driven by an external **FW1903** control IC (`LED_STRIP_TIMING_FW1903`), not integrated 5050 WS2812 LEDs. The project was created from Espressif's `rmt/led_strip` example and is currently still close to it (rainbow chase demo). Toolchain is **ESP-IDF v5.5.2**, configured target is **ESP32-C6** (`sdkconfig`, `.vscode/settings.json`). Local dev is on Windows with the ESP-IDF VS Code extension (serial port COM14, UART flashing, OpenOCD `board/esp32c6-builtin.cfg`); a `.devcontainer` exists for a Linux/QEMU ESP-IDF environment.

## Commands

Run from an ESP-IDF-exported shell (`export.ps1` / `export.sh` from the IDF install, or the VS Code ESP-IDF terminal). On this machine `export.ps1` picks up the system Python 3.13 and fails (no `idf5.5_py3.13_env`); put the IDF-bundled Python first:

```powershell
$env:PATH = "C:\Users\dschilling\.espressif\tools\idf-python\3.11.2;" + $env:PATH
. C:\Users\dschilling\esp\v5.5.2\esp-idf\export.ps1
```


- Build: `idf.py build`
- Flash + serial monitor: `idf.py -p COM14 flash monitor` (exit monitor with `Ctrl-]`)
- Change target (regenerates `sdkconfig`): `idf.py set-target esp32c6`
- Menuconfig: `idf.py menuconfig`
- Clean: `idf.py fullclean`

Test (on-hardware, requires `pytest-embedded` / `pytest-embedded-idf` and a connected board flashed from `build/`):

- `pytest pytest_led_strip.py --target esp32c6`

The test only asserts boot log lines (`example: Create RMT TX channel`, `example: Install led strip encoder`, `example: Enable RMT TX channel`, `example: Start LED rainbow chase`). Changing the `TAG` or those `ESP_LOGI` messages in `main/led_strip_example_main.c` will break it.

clangd uses `build/compile_commands.json` (`.clangd`, `.vscode/settings.json`), so run a build before expecting accurate IDE diagnostics.

## Architecture

- Top-level `CMakeLists.txt` sets `MINIMAL_BUILD ON`, so only `main` and its declared dependencies are built. Any new IDF component used by `main` (e.g. `esp_driver_gpio`, `esp_timer`) must be added to `PRIV_REQUIRES` in `main/CMakeLists.txt`, and any new `.c` file to its `SRCS`.
- `main/led_strip_encoder.c` implements a custom RMT encoder (`rmt_new_led_strip_encoder`) as a two-state machine composing an RMT **bytes encoder** (pixel data, MSB first) and a **copy encoder** (reset/latch symbol). Bit timings and reset length come from a `led_strip_timing_t` in the encoder config (presets `LED_STRIP_TIMING_FW1903` / `LED_STRIP_TIMING_WS2811` / `LED_STRIP_TIMING_WS2812` in `led_strip_encoder.h`), converted to ticks of `config->resolution`. The encode callback must handle `RMT_ENCODING_MEM_FULL` by yielding and resuming from `led_encoder->state`; it is marked `RMT_ENCODER_FUNC_ATTR` because it can run in ISR context.
- `main/led_strip_example_main.c` (`app_main`) creates the RMT TX channel (10 MHz resolution, GPIO 4), installs the encoder, then loops building a pixel buffer and calling `rmt_transmit` + `rmt_tx_wait_all_done`. The pixel buffer is a flat `uint8_t` array, `EXAMPLE_LED_NUMBERS` × 3 bytes, where `EXAMPLE_LED_NUMBERS` counts control ICs (addressable pixels), not physical LEDs. Wire color order is set by `LED_STRIP_R/G/B_OFFSET`. GPIO, timing preset, color order, pixel count, and chase speed are `#define`s at the top of that file.
