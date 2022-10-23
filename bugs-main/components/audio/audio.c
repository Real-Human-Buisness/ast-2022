
#include "audio.h"
#include "samples.h"

#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"

#include "driver/i2s.h"

#define SAMPLE_RATE 22050
#define FRAMES_PER_SAMPLE 64
#define AUDIO_STACK_SIZE 4096
#define BUFFER_COUNT 6

static const char* TAG = "audio";

static QueueHandle_t i2s_queue;
static TaskHandle_t audio_handle = NULL;

int16_t *sample_buffer = NULL;

static esp_err_t initialize_i2s(void);
static esp_err_t initialize_run_loop(void);

esp_err_t initialize_audio(void) {
    esp_err_t ret = ESP_OK;
    ESP_LOGI(TAG, "Initializing samples, i2s, task");
    
    ESP_GOTO_ON_ERROR(initialize_i2s(), err_ia, TAG, "Failed to initialize i2s");
    ESP_GOTO_ON_ERROR(initialize_samples(), err_ia, TAG, "Failed to initialize samples");
    ESP_GOTO_ON_ERROR(initialize_run_loop(), err_ia, TAG, "Failed to initialize loop");
    
    ESP_LOGI(TAG, "Component successfully initialized");
    return ESP_OK;

err_ia:
    return ret;
}

static esp_err_t initialize_i2s(void) {

    esp_err_t ret = ESP_OK;

    ESP_LOGI(TAG, "CONFIG_I2S_NUM=%d", CONFIG_I2S_NUM);
	ESP_LOGI(TAG, "CONFIG_I2S_BCK_IO=%d", CONFIG_I2S_BCK_IO);
	ESP_LOGI(TAG, "CONFIG_I2S_WS_IO=%d", CONFIG_I2S_WS_IO);
	ESP_LOGI(TAG, "CONFIG_I2S_DO_IO=%d", CONFIG_I2S_DO_IO);

    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = BUFFER_COUNT,
        .dma_buf_len = FRAMES_PER_SAMPLE * 2,
    };

    i2s_pin_config_t pin_config = {
        .mck_io_num = I2S_PIN_NO_CHANGE, //?
        .bck_io_num = CONFIG_I2S_BCK_IO, //Kconfig
        .ws_io_num = CONFIG_I2S_WS_IO, //Kconfig
        .data_out_num = CONFIG_I2S_DO_IO, //Kconfig
        .data_in_num = -1 //Not used
    };
    i2s_queue = xQueueCreate(BUFFER_COUNT, sizeof(i2s_event_t));
    if (i2s_queue == NULL) {
        ESP_LOGE(TAG, "Unable to create i2s queue");
        ret = ESP_ERR_INVALID_STATE;
        goto err_i2S;
    }
    ESP_GOTO_ON_ERROR(
        i2s_driver_install(CONFIG_I2S_NUM, &i2s_config, BUFFER_COUNT, &i2s_queue), err_i2S, TAG, "i2s unable to install"
    );
    ESP_GOTO_ON_ERROR(
        i2s_set_pin(CONFIG_I2S_NUM, &pin_config), err_i2S, TAG, "i2s unable to set pins"
    );
    ESP_GOTO_ON_ERROR(
        i2s_zero_dma_buffer(CONFIG_I2S_NUM), err_i2S, TAG, "i2s unable to clear dma buffer"
    );

    return ESP_OK;

err_i2S:
    return ret;
}

static void run_audio_handler(void *);

static esp_err_t initialize_run_loop(void) {
    xTaskCreatePinnedToCore(
        run_audio_handler,
        "AUDIO_HANDLER",
        AUDIO_STACK_SIZE, 
        NULL,
        tskIDLE_PRIORITY + 1,
        &audio_handle,
        PRO_CPU_NUM
    );
    // don't know how to make sure that worked...
    return ESP_OK;
}

static void run_audio_handler(void *p) {
    ESP_LOGI(TAG, "Running audio handler");
    int available_bytes = 0;
    int buffer_position = 0;
    frame_t *sample_buffer = heap_caps_malloc(sizeof(frame_t) * FRAMES_PER_SAMPLE, MALLOC_CAP_8BIT);
    while(1) {
        // wait for some data to be requested
        i2s_event_t evt;
        if (xQueueReceive(i2s_queue, &evt, portMAX_DELAY) == pdPASS) {
            if (evt.type == I2S_EVENT_TX_DONE) {
                size_t i2s_bytes_write = 0;
                do {
                    if (available_bytes == 0) {
                        memset(sample_buffer, 0, sizeof(frame_t) * FRAMES_PER_SAMPLE);
                        write_frames_to_buffer(sample_buffer, FRAMES_PER_SAMPLE);
                        available_bytes = FRAMES_PER_SAMPLE * sizeof(uint32_t);
                        buffer_position = 0;
                    }
                    if (available_bytes > 0) {
                        i2s_write(CONFIG_I2S_NUM, buffer_position + (uint8_t *)sample_buffer, available_bytes, &i2s_bytes_write, portMAX_DELAY);
                        available_bytes -= i2s_bytes_write;
                        buffer_position += i2s_bytes_write;
                    }
                } while (i2s_bytes_write > 0);
            }
        }
	}
    free(sample_buffer);
}