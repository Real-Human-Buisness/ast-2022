
#include "leds.h"
#include "touch.h"

#include <led_strip.h>
#include <lib8tion.h>
#include <color.h>

#define LEDS_STACK_SIZE 4096
static const char *TAG = "leds";

#define BUG_SIZE 0

#if BUG_SIZE == 0
#define LEDS_PER_SEGMENT 12
#elif BUG_SIZE == 1
#define LEDS_PER_SEGMENT 24
// #define LEDS_PER_SEGMENT 3
#else
#define LEDS_PER_SEGMENT 42
#endif

static const size_t BUG_SEGMENTS = TOUCH_COUNT + 2;
static const size_t LED_COUNT = BUG_SEGMENTS * LEDS_PER_SEGMENT;

#define LED_GPIO 15
#define LED_TYPE LED_STRIP_WS2812
#define BRIGHTNESS 192

static TaskHandle_t leds_handle = NULL;
static uint8_t local_touch_value[TOUCH_COUNT];

static led_strip_t strip = {
    .type = LED_TYPE,
    .length = LED_COUNT,
    .gpio = LED_GPIO,
    .buf = NULL,
    .brightness = 255,
};


static esp_err_t initialize_strip(void);
static esp_err_t initialize_run_loop(void);

esp_err_t initialize_leds(void) {
    esp_err_t ret = ESP_OK;
    ESP_LOGI(TAG, "Initializing strip, task");
    
    ESP_GOTO_ON_ERROR(initialize_strip(), err_il, TAG, "Failed to initialize strip");
    ESP_GOTO_ON_ERROR(initialize_run_loop(), err_il, TAG, "Failed to initialize loop");
    
    ESP_LOGI(TAG, "Component successfully initialized");
    return ESP_OK;

err_il:
    return ret;
}

static esp_err_t initialize_strip(void) {
    led_strip_install();
    ESP_LOGI(TAG, "LED COUNT: %i", LED_COUNT);
    return led_strip_init(&strip);
}

static void run_led_handler(void *);

static esp_err_t initialize_run_loop(void) {
    xTaskCreatePinnedToCore(
        run_led_handler,
        "LED_HANDLER",
        LEDS_STACK_SIZE, 
        NULL,
        tskIDLE_PRIORITY + 1,
        &leds_handle,
        APP_CPU_NUM
    );
    // don't know how to make sure that worked...
    return ESP_OK;
}

static uint8_t touched_now;

static void update_local_store(void) {
    if (xSemaphoreTake(touch_store->mutex, ( TickType_t ) 5) == pdTRUE) {
        memcpy(local_touch_value, touch_store->touch_value, sizeof(local_touch_value));
        xSemaphoreGive(touch_store->mutex);
    } else {
        ESP_LOGE(TAG, "Couldn't get mutex?");
    }
    touched_now = 0;
    for (uint8_t i = 0; i < TOUCH_COUNT; i++) {
        if (local_touch_value[i] > 0) {
            touched_now += 1;
        }
    }
}

static void clear_lights(void) {
    memset(strip.buf, 0, sizeof(rgb_t) * LED_COUNT);
}

#define RGB_FROM_VALUES(_r, _g, _b) \
    ((rgb_t) { \
        .r = (uint8_t) (_b), \
        .g = (uint8_t) (_g), \
        .b = (uint8_t) (_r), \
     })

#define RGB_FROM_VALUES_SCALE(_r, _g, _b) \
    ((rgb_t) { \
        .r = (uint8_t)((((uint16_t) _b) * (1 + (uint16_t) (128))) >> 8), \
        .g = (uint8_t)((((uint16_t) _g) * (1 + (uint16_t) (196))) >> 8), \
        .b = (uint8_t)(((((uint16_t) _r) * (1 + (uint16_t) (255))) >> 8)), \
     })

static const rgb_t head_tail_color = RGB_FROM_VALUES(255, 255, 255);

static const rgb_t bug_colors[TOUCH_COUNT] =
{
    RGB_FROM_VALUES_SCALE(255, 0, 0), // r
    RGB_FROM_VALUES_SCALE(255, 128, 0), // o
    RGB_FROM_VALUES_SCALE(255, 255, 0), // y
    RGB_FROM_VALUES_SCALE(0, 255, 0), // g
    RGB_FROM_VALUES_SCALE(0, 255, 255), // t
    RGB_FROM_VALUES_SCALE(0, 0, 255), // b
    RGB_FROM_VALUES_SCALE(255, 0, 255), // V
    RGB_FROM_VALUES_SCALE(255, 0, 128), // P
};

static void set_segment(size_t segment, rgb_t *color) {
    // led_strip_fill(&strip, segment * LEDS_PER_SEGMENT, LEDS_PER_SEGMENT, *color);
    for (int i = 0; i < LEDS_PER_SEGMENT; i++) {
        memcpy(strip.buf + (segment * LEDS_PER_SEGMENT + i) * sizeof(rgb_t), color, sizeof(rgb_t));
    }
}

static void run_graphics(void) {
    static rgb_t head_tail_fade_color;
    static uint8_t head_tail_fade;
    // set head tail based on number of active nodes
    head_tail_fade = touched_now / (float) TOUCH_COUNT * 255.0;
    head_tail_fade_color = rgb_scale_video(head_tail_color, head_tail_fade);
    set_segment(0, &head_tail_fade_color);
    set_segment(BUG_SEGMENTS - 1, &head_tail_fade_color);
    // set other colors based on is touched
    for (int i = 0; i < TOUCH_COUNT; i++) {
        if (local_touch_value[i] > 0) {
            set_segment(1 + i, &bug_colors[i]);
        }
    }


}

static void write_out_graphics(void) {
    led_strip_flush(&strip);
}

static void run_led_handler(void * _p) {
    ESP_LOGI(TAG, "Running led handler");
    while (1) {
        update_local_store();
        clear_lights();
        run_graphics();
        write_out_graphics();
        vTaskDelay(10);
    }
}