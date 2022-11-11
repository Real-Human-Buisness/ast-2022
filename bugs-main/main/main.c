#include <stdio.h>
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "esp_idf_version.h"

#include <string.h>

#include "touch.h"
#include "audio.h"
#include "leds.h"

static const char *TAG = "main";

void app_main(void)
{

    ESP_LOGI(TAG, "Running AST 2022, bugs");
    ESP_LOGI(TAG, "IDF version: %s", esp_get_idf_version());

    ESP_ERROR_CHECK(initialize_touch());
    ESP_ERROR_CHECK(initialize_audio());
    ESP_ERROR_CHECK(initialize_leds());
    
    ESP_LOGI(TAG, "Program Initialized");

}
