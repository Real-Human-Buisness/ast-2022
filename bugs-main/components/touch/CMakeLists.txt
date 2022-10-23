idf_component_register(
    SRCS mpr121.c tracker.c touch.c
    INCLUDE_DIRS .
    REQUIRES lib8tion
)

include_directories("${HOME}/esp/esp-idf-lib/components") 