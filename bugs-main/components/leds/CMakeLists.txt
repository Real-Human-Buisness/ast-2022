idf_component_register(
    SRCS leds.c
    INCLUDE_DIRS .
    REQUIRES led_strip lib8tion touch color
)

include_directories("${HOME}/esp/esp-idf-lib/components") 