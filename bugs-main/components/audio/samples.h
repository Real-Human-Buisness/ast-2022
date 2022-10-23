#ifndef AUDIO_SAMPLES_H
#define AUDIO_SAMPLES_H

#include <stdio.h>
#include "esp_check.h"

typedef struct
{
    int16_t left;
    int16_t right;
} frame_t;

// since were using 16 bit mono, frames are int16_t
struct sample_s {
    int16_t *frame_buffer;
    uint32_t size_in_frames;
    uint32_t position_in_frames;
};
typedef struct sample_s sample_t;

esp_err_t initialize_samples(void);

void write_frames_to_buffer(frame_t *sample_buffer, uint32_t frames_per_sample);

#endif //AUDIO_SAMPLES_H