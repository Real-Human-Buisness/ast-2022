
#include "touch.h"
#include "mpr121.h"
#include "tracker.h"

#include <string.h>
#include "esp_log.h"
#include "esp_check.h"
#include "esp_heap_caps.h"


#define TOUCH_STACK_SIZE 2048
static const char *TAG = "touch";

#define TOUCH_THRESHOLD 40
#define RELEASE_THRESHOLD 20


touch_store_t *touch_store = NULL;


static TaskHandle_t touch_handle = NULL;
static MPR121_t dev;
static uint8_t local_touch_value[TOUCH_COUNT];


static esp_err_t initialize_touch_store(void);
static esp_err_t initialize_touch_driver(void);
static esp_err_t initialize_run_loop(void);

esp_err_t initialize_touch(void) {
    esp_err_t ret = ESP_OK;
    ESP_LOGI(TAG, "Initializing store, mpr121, task");
    
    ESP_GOTO_ON_ERROR(initialize_touch_store(), err_it, TAG, "Failed to initialize store");
    ESP_GOTO_ON_ERROR(initialize_touch_driver(), err_it, TAG, "Failed to initialize driver");
    ESP_GOTO_ON_ERROR(initialize_touch_tracker(), err_it, TAG, "Failed to initialize tracker");
    ESP_GOTO_ON_ERROR(initialize_run_loop(), err_it, TAG, "Failed to initialize loop");
    
    ESP_LOGI(TAG, "Component successfully initialized");
    return ESP_OK;

err_it:
    return ret;
}

static esp_err_t initialize_touch_store(void) {
    touch_store = heap_caps_malloc(sizeof(touch_store_t), MALLOC_CAP_8BIT);
    if (touch_store == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    memset(touch_store, 0, sizeof(touch_store_t));
    touch_store->mutex = xSemaphoreCreateMutex();
    if (touch_store->mutex == NULL) {
        ESP_LOGE(TAG, "GPIO ISR handler install failed");
        return ESP_ERR_INVALID_STATE;
    }
    return ESP_OK;
}

static esp_err_t initialize_touch_driver(void) {

    esp_err_t ret = ESP_OK;

    ESP_LOGI(TAG, "CONFIG_I2C_ADDRESS=0x%X", CONFIG_I2C_ADDRESS);
	ESP_LOGI(TAG, "CONFIG_SCL_GPIO=%d", CONFIG_SCL_GPIO);
	ESP_LOGI(TAG, "CONFIG_SDA_GPIO=%d", CONFIG_SDA_GPIO);

    ESP_GOTO_ON_FALSE(
        MPR121_begin(&dev, CONFIG_I2C_ADDRESS, TOUCH_THRESHOLD, RELEASE_THRESHOLD, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO),
        ESP_ERR_INVALID_STATE, err_itd, TAG, "Couldn't begin MPR121"
    );
    ESP_GOTO_ON_ERROR((esp_err_t) MPR121_getError(&dev), err_itd, TAG, "MPR121 initialized with err?");
    
    MPR121_setFFI(&dev, FFI_10); // AFE Configuration 1
    ESP_GOTO_ON_ERROR((esp_err_t) MPR121_getError(&dev), err_itd, TAG, "MPR121 unable to write config");
	
    MPR121_setSFI(&dev, SFI_10); // AFE Configuration 2
    ESP_GOTO_ON_ERROR((esp_err_t) MPR121_getError(&dev), err_itd, TAG, "MPR121 unable to write config");
	
    MPR121_setGlobalCDT(&dev, CDT_4US);  // reasonable for larger capacitances
    ESP_GOTO_ON_ERROR((esp_err_t) MPR121_getError(&dev), err_itd, TAG, "MPR121 unable to write config");
	
    MPR121_autoSetElectrodesDefault(&dev, true);	// autoset all electrode settings
    ESP_GOTO_ON_ERROR((esp_err_t) MPR121_getError(&dev), err_itd, TAG, "MPR121 unable to write config");

    return ESP_OK;

err_itd:
    return ret;
}

static TaskFunction_t run_touch_handler(void);

static esp_err_t initialize_run_loop(void) {
    xTaskCreatePinnedToCore(
        run_touch_handler,
        "TOUCH_HANDLER",
        TOUCH_STACK_SIZE, 
        NULL,
        tskIDLE_PRIORITY + 1,
        &touch_handle,
        APP_CPU_NUM
    );
    // don't know how to make sure that worked...
    return ESP_OK;
}

static TaskFunction_t run_touch_handler() {
    ESP_LOGI(TAG, "Running touch handler");
    while(1) {
		MPR121_updateAll(&dev);
		for (uint8_t i = 0; i < TOUCH_COUNT; i++) {
            // update_touch_tracker(i, &local_touch_value[i], MPR121_getTouchData(&dev, i));
            local_touch_value[i] = MPR121_getTouchData(&dev, i) ? 255 : 0;
		}
        if (xSemaphoreTake(touch_store->mutex, ( TickType_t ) 5) == pdTRUE)
        {
            memcpy(touch_store->touch_value, local_touch_value, sizeof(local_touch_value));
            xSemaphoreGive(touch_store->mutex);
        } else {
            ESP_LOGE(TAG, "Couldn't grab the mutex, fkn panicking");
            esp_restart();
        }
		vTaskDelay(10);
	}
}