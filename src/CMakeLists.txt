idf_component_register(SRCS "main.c" "LedStripEncoder.c" "LedStripDriver.c" "System.c" "StrobeDriver.c"
                       PRIV_REQUIRES esp_driver_rmt esp_driver_gpio esp_timer
                       INCLUDE_DIRS "../include")
