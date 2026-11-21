# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

ESP-IDF firmware (C) that drives an addressable LED strip using the RMT peripheral. The strip uses 3535 SMD LEDs driven by an external **FW1903** control IC, not integrated 5050 WS2812 LEDs. The code started from Espressif's `rmt/led_strip` example and still runs its rainbow chase demo. `README.md` is the unmodified example README, and its paths (`main/...`) and WS2812 wiring notes are out of date. Toolchain is **ESP-IDF v5.5.2** and the target is **ESP32-C6**. Local development is on Windows with the ESP-IDF VS Code extension (serial port COM14, UART flashing, OpenOCD `board/esp32c6-builtin.cfg`).

## Commands

Run these from an ESP-IDF-exported shell. On this machine, `export.ps1` picks up the system Python 3.13 and fails because there is no `idf5.5_py3.13_env`, so put the IDF-bundled Python first:

```powershell
$env:PATH = "C:\Users\dschilling\.espressif\tools\idf-python\3.11.2;" + $env:PATH
. C:\Users\dschilling\esp\v5.5.2\esp-idf\export.ps1
```

- Build: `idf.py build`
- Flash + serial monitor: `idf.py -p COM14 flash monitor` (exit the monitor with `Ctrl-]`)
- Menuconfig: `idf.py menuconfig`
- Clean: `idf.py fullclean`. Run this after changing the component layout in either `CMakeLists.txt`, because a stale CMake cache can hide the change.

`sdkconfig` is in `.gitignore`. On a fresh clone, run `idf.py set-target esp32c6` before the first build, or it defaults to ESP32.

There are no tests or linters. To verify a change, build it and check it on hardware. clangd reads `build/compile_commands.json` (`.clangd`, `.vscode/settings.json`), so run a build before trusting IDE diagnostics.

## Build layout (non-standard)

App code lives in `src/` and headers in `include/`. There is **no `main/` directory**, so ESP-IDF won't discover the app component on its own:

- The top-level `CMakeLists.txt` sets `EXTRA_COMPONENT_DIRS src` and `COMPONENTS src` **before** `include(project.cmake)`. The component is named `src`. Setting `COMPONENTS` keeps the build minimal, so only `src` and its dependencies are built. Don't re-add `idf_build_set_property(MINIMAL_BUILD ON)`: it forces `COMPONENTS main`, nothing from the app gets linked, and the only symptom is a linker warning, `cannot find entry symbol _start`.
- `src/CMakeLists.txt` registers the component with `INCLUDE_DIRS "../include"`. Add every new `.c` file to `SRCS`, and every IDF component it uses (for example `esp_timer`, `nvs_flash`) to `PRIV_REQUIRES`. Because the build is minimal, a component you don't list isn't built, and its headers aren't found.

## Architecture

- **`include/HwDefs.h`** holds the board pinout: LED strip data on GPIO 7, strobe pulse train (`StrobeDriver`) on GPIO 0, and a system output on GPIO 11. **`include/AppDefs.h`** is reserved for application-level defines and is currently empty. Put pin assignments in `HwDefs.h`, not in the modules that use them.
- **`LedStripEncoder.c/.h`** is a custom RMT encoder (`rmt_new_led_strip_encoder`). It is a two-state machine that composes an RMT **bytes encoder** (pixel data, MSB first) and a **copy encoder** (the reset/latch symbol). Bit timings and reset length come from a `led_strip_timing_t` in the config. They are given in ns/µs and converted to ticks of `config->resolution`. `LED_STRIP_TIMING_FW1903` is the only preset; to support another IC, add a preset next to it. The encode callback must handle `RMT_ENCODING_MEM_FULL` by yielding and resuming from `led_encoder->state`. It is marked `RMT_ENCODER_FUNC_ATTR` because it can run in ISR context.
- **`LedStripDriver.c/.h`** owns the RMT channel, the encoder handle, and a static pixel buffer in file-scope statics. `init_led_strip_driver()` creates the TX channel (10 MHz resolution), installs the encoder, and enables the channel. The pattern functions (`update_rainbow_chase`, `led_strip_solid`) fill the buffer, then call `rmt_transmit` and `rmt_tx_wait_all_done`. `update_rainbow_chase` blocks for 3 × `EXAMPLE_CHASE_SPEED_MS` per call. Strip parameters are `#define`s in `LedStripDriver.h`:
  - The buffer is a flat `uint8_t` array of `EXAMPLE_LED_NUMBERS × 3` bytes. `EXAMPLE_LED_NUMBERS` counts control ICs (addressable pixels), not physical LEDs: on 12 V strips one IC drives about 3 LEDs.
  - `LED_STRIP_R/G/B_OFFSET` set the color byte order on the wire, which depends on how the strip vendor wired the LEDs. If colors come out wrong, swap these offsets; don't change the timing.
- **`System.c/.h`** handles general GPIO setup (`init_gpio` configures `UNDERGLOW_SW_GPIO`).
- **`main.c`** contains `app_main`, which calls the module `init_*` functions and then loops over a pattern function while pulsing GPIO 11.
