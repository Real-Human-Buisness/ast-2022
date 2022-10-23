
#include <stdio.h>
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "esp_idf_version.h"

#include "mpr121.h"


static const char *TAG = "main";


void run_and_loop_mpr121_test(void) {
    ESP_LOGI(TAG, "CONFIG_I2C_ADDRESS=0x%X", CONFIG_I2C_ADDRESS);
	ESP_LOGI(TAG, "CONFIG_SCL_GPIO=%d", CONFIG_SCL_GPIO);
	ESP_LOGI(TAG, "CONFIG_SDA_GPIO=%d", CONFIG_SDA_GPIO);
    
	MPR121_t dev;
    
    uint16_t touchThreshold = 40;
	uint16_t releaseThreshold = 20;

    bool ret = MPR121_begin(&dev, CONFIG_I2C_ADDRESS, touchThreshold, releaseThreshold, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO);
	ESP_LOGI(TAG, "MPR121_begin=%d", ret);
	if (ret == false) {
		switch (MPR121_getError(&dev)) {
			case NO_ERROR:
				ESP_LOGE(TAG, "no error");
				break;
			case ADDRESS_UNKNOWN:
				ESP_LOGE(TAG, "incorrect address");
				break;
			case READBACK_FAIL:
				ESP_LOGE(TAG, "readback failure");
				break;
			case OVERCURRENT_FLAG:
				ESP_LOGE(TAG, "overcurrent on REXT pin");
				break;
			case OUT_OF_RANGE:
				ESP_LOGE(TAG, "electrode out of range");
				break;
			case NOT_INITED:
				ESP_LOGE(TAG, "not initialised");
				break;
			default:
				ESP_LOGE(TAG, "unknown error");
				break;
		}
		while(1) {
			vTaskDelay(1);
		}
	}


	MPR121_setFFI(&dev, FFI_10); // AFE Configuration 1
	MPR121_setSFI(&dev, SFI_10); // AFE Configuration 2
	MPR121_setGlobalCDT(&dev, CDT_4US);  // reasonable for larger capacitances
	MPR121_autoSetElectrodesDefault(&dev, true);	// autoset all electrode settings

    while(1) {
		MPR121_updateAll(&dev);
		for (int i = 0; i < 8; i++) {
			if (MPR121_isNewTouch(&dev, i)) {
				ESP_LOGI(TAG, "electrode %d was just touched", i);
			} else if (MPR121_isNewRelease(&dev, i)) {
				ESP_LOGI(TAG, "electrode %d was just released", i);
			}
		}
		vTaskDelay(10);
	}

}


void app_main(void)
{
    ESP_LOGI(TAG, "Running Bugs Test");
    ESP_LOGI(TAG, "IDF version: %s", esp_get_idf_version());

    ESP_LOGI(TAG, "Initializing mpr121 and looping");
    run_and_loop_mpr121_test();
}