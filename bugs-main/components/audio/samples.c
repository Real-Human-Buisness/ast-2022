
#include "samples.h"

#include "touch.h"

#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"

#include "lib8tion.h"


static const char* TAG = "audio_samples";

static uint8_t local_touch_value[TOUCH_COUNT];
static sample_t sample_store[TOUCH_COUNT];

static esp_err_t do_initialize_sample(uint8_t position);

esp_err_t initialize_samples(void) {

    ESP_LOGI(TAG, "Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf = {
      .base_path = "/spiffs",
      .partition_label = NULL,
      .max_files = 5,
      .format_if_mount_failed = true
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return ret;
    }

    for (uint8_t i = 0; i < TOUCH_COUNT; i++) {
        ret = do_initialize_sample(i);
        if (ret != ESP_OK) {
            goto cleanup_ifs;
        }
    }

    ESP_LOGI(TAG, "Successfully initialized samples");

cleanup_ifs:
    esp_vfs_spiffs_unregister(conf.partition_label);
    return ret;
}

#pragma pack(push, 1)
typedef struct
{
    // RIFF Header
    char riff_header[4]; // Contains "RIFF"
    int wav_size;        // Size of the wav portion of the file, which follows the first 8 bytes. File size - 8
    char wave_header[4]; // Contains "WAVE"

    // Format Header
    char fmt_header[4]; // Contains "fmt " (includes trailing space)
    int fmt_chunk_size; // Should be 16 for PCM
    short audio_format; // Should be 1 for PCM. 3 for IEEE Float
    short num_channels;
    int sample_rate;
    int byte_rate;          // Number of bytes per second. sample_rate * num_channels * Bytes Per Sample
    short sample_alignment; // num_channels * Bytes Per Sample
    short bit_depth;        // Number of bits per sample

    // Data
    char data_header[4]; // Contains "data"
    int data_bytes;      // Number of bytes in data. Number of samples * num_channels * sample byte size
    // uint8_t bytes[]; // Remainder of wave file is bytes
} wav_header_t;
#pragma pack(pop)

static esp_err_t do_initialize_sample(uint8_t position) {

    // for now, hardcode bug 
    char bug_file[20];
    
    // indigo indexed from 1, lol
    sprintf(bug_file, "/spiffs/b-1-%d.wav", position + 1);

    ESP_LOGI(TAG, "Reading file %s", bug_file);
    FILE* f = fopen((const char *)&bug_file, "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t ret = ESP_OK;

    // read the header, sanity checks
    wav_header_t wav_header;
    if (fread((char*)&wav_header, sizeof(wav_header_t), 1, f)!= 1) {
        ESP_LOGE(TAG, "Couldn't read header...");
        ret = ESP_ERR_INVALID_STATE;
        goto cleanup_dis;
    }

    if (wav_header.bit_depth != 16){
        ESP_LOGE(TAG, "ERROR: bit depth %d is not supported\n", wav_header.bit_depth);
        ret = ESP_ERR_INVALID_STATE;
        goto cleanup_dis;
    } else if (wav_header.num_channels != 1) {
        ESP_LOGE(TAG, "ERROR: channels %d is not 1", wav_header.num_channels);
        ret = ESP_ERR_INVALID_STATE;
        goto cleanup_dis;
    } else if (wav_header.sample_rate != 22050) {
        ESP_LOGE(TAG, "ERROR: sample rate %d is not 22050", wav_header.num_channels);
        ret = ESP_ERR_INVALID_STATE;
        goto cleanup_dis;
    }

    // read into buffer
    sample_store[position].size_in_frames = wav_header.data_bytes / 2;
    ESP_LOGI(TAG, "Audio is %d frames (int16)", sample_store[position].size_in_frames);


    sample_store[position].position_in_frames = 0;
    sample_store[position].frame_buffer = NULL;
    sample_store[position].frame_buffer = heap_caps_malloc(sizeof(uint8_t) * wav_header.data_bytes, MALLOC_CAP_8BIT);
    if (sample_store[position].frame_buffer == NULL) {
        ESP_LOGE(TAG, "Couldn't create buffer of size %d frames (int16)", sample_store[position].size_in_frames);
        ret = ESP_ERR_INVALID_STATE;
        goto cleanup_dis;
    }
    memset(sample_store[position].frame_buffer, 0, sizeof(uint8_t) * wav_header.data_bytes);

    fseek(f, 44, SEEK_SET);
    if (fread((uint8_t*)sample_store[position].frame_buffer, sizeof(uint8_t), wav_header.data_bytes, f) != wav_header.data_bytes) {
        ESP_LOGE(TAG, "couldn't read wav data");
        ret = ESP_ERR_INVALID_STATE;
        goto cleanup_dis;
    }

    // avoid clipping by bringing down the samples & then dividing by rms
    for (int i = 0; i < sample_store[position].size_in_frames; i++) {
        sample_store[position].frame_buffer[i] *= 0.75;
    }

    ESP_LOGI(TAG, "Succesfully read in file %s", bug_file);

cleanup_dis: 
    fclose(f);
    return ret;
}

static float rms_multipliers[TOUCH_COUNT + 1] = { 0.0, 1.0, 0.5, 0.333, 0.25, 0.2, 0.166, 0.142, 0.124 };

static float rms_multiplier = 0.0;

static void update_local_touch_store(void) {
    if (xSemaphoreTake(touch_store->mutex, ( TickType_t ) 5) == pdTRUE) {
        memcpy(local_touch_value, touch_store->touch_value, sizeof(local_touch_value));
        xSemaphoreGive(touch_store->mutex);
    } else {
        ESP_LOGE(TAG, "Couldn't get mutex?");
    }
    uint8_t touched_now = 0;
    for (uint8_t i = 0; i < TOUCH_COUNT; i++) {
        if (local_touch_value[i] > 0) {
            touched_now += 1;
        }
    }
    rms_multiplier = rms_multipliers[touched_now];
}

static void write_out_no_multiplier(frame_t *sample_buffer, uint32_t frames_per_sample, sample_t *sample);

void write_frames_to_buffer(frame_t *sample_buffer, uint32_t frames_per_sample) {
    update_local_touch_store();
    sample_t *sample = NULL;
    for (uint8_t i = 0; i < TOUCH_COUNT; i++) {
        sample = &sample_store[i];
        if (local_touch_value[i] == 0) {
            sample->position_in_frames = 0;
        } else if (local_touch_value[i] == 255) {
            write_out_no_multiplier(sample_buffer, frames_per_sample, sample);
        } else {
            // handle with care...
        }
    }
}

LIB8STATIC_ALWAYS_INLINE int16_t qadd15(int16_t i, int16_t j)
{
    int32_t t = i + j;
    if (t > 32767)
        t = 32767;
    else if (t < -32768)
        t = -32768;
    return t;
}

static void write_out_no_multiplier(frame_t *sample_buffer, uint32_t frames_per_sample, sample_t *sample) {
    uint32_t samples_unwrapped = frames_per_sample;
    uint32_t samples_wrapped = 0;
    if ((sample->position_in_frames + frames_per_sample) >= sample->size_in_frames) {
        samples_unwrapped = sample->size_in_frames - sample->position_in_frames;
        samples_wrapped = frames_per_sample - samples_unwrapped;
    }
    for (uint32_t i = 0; i < samples_unwrapped; i++) {
        sample_buffer[i].left = qadd15(
            sample_buffer[i].left,
            (int16_t) ((float) sample->frame_buffer[sample->position_in_frames + i] * rms_multiplier)
        );
        sample_buffer[i].right += sample_buffer[i].left;
    }
    // didn't need the circular buffer; move the offset and continue
    if (samples_wrapped == 0) {
        sample->position_in_frames += frames_per_sample;
        return;
    }
    // circling around the buffer
    for (uint32_t i = 0; i < samples_wrapped; i++) {
        sample_buffer[i + samples_unwrapped].left = qadd15(
            sample_buffer[i + samples_unwrapped].left,
            (int16_t) ((float) sample->frame_buffer[i] * rms_multiplier)
        );
        sample_buffer[i + samples_unwrapped].right += sample_buffer[i + samples_unwrapped].left;
    }
    sample->position_in_frames = samples_wrapped;
}
