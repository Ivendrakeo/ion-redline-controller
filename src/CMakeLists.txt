idf_component_register(SRCS "main.c" "LedStripEncoder.c" "LedStripDriver.c" "System.c"
                       PRIV_REQUIRES esp_driver_rmt esp_driver_gpio
                       INCLUDE_DIRS "../include")
