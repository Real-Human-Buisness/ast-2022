
#ifndef TOUCH_H
#define TOUCH_H

#include <stdio.h>
#include "esp_check.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#define TOUCH_COUNT 8

struct touch_store_s {
    uint8_t touch_value[TOUCH_COUNT];
    SemaphoreHandle_t mutex;
};

typedef struct touch_store_s touch_store_t;

extern touch_store_t *touch_store;

esp_err_t initialize_touch(void);


#endif // TOUCH_H