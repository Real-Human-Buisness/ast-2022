
#include "tracker.h"
#include "touch.h"

#include "esp_timer.h"

#include "lib8tion.h"


/*
    HACK TIME
    we know that the touch tracker runs every 10ms.
    We can use that do decide some arbitray value to ramp up and down.

    Lets use 170ms ramp up and 300ms ramp down.

    at 256 (uint 8) for ramp up we can just add 170ms / 10ms = 17 increments.
    256 / 17 ~ 15. So every step we can add 15 to value until its at 255, where
    we move it to sustain

    same with 300ms - 300ms / 10ms = 30 steps, 256 / 30 ~ 9, so subtract 9 till 0

    using lib8tion qadd8, qsub8 to handle bounds checking, state transition
*/

static touch_tracker_t tracker_store[TOUCH_COUNT];

static uint8_t ramp_down_inc = 9;
static uint8_t ramp_up_inc = 15;

esp_err_t initialize_touch_tracker(void) {
    for (uint8_t i = 0; i < TOUCH_COUNT; i++) {
        tracker_store[i].touch_state = TOUCH_STATE_OFF;
    }
    return ESP_OK;
}

void handle_touch_state_off(touch_tracker_t *tracker, uint8_t *value, bool is_touch);
void handle_touch_state_ramp_up(touch_tracker_t *tracker, uint8_t *value, bool is_touch);
void handle_touch_state_sustain(touch_tracker_t *tracker, uint8_t *value, bool is_touch);
void handle_touch_state_ramp_down(touch_tracker_t *tracker, uint8_t *value, bool is_touch);

void update_touch_tracker(uint8_t position, uint8_t *value, bool is_touch) {
    touch_tracker_t *tracker = &tracker_store[position];
    switch (tracker->touch_state) {
        case TOUCH_STATE_OFF:
            handle_touch_state_off(tracker, value, is_touch);
            break;
        case TOUCH_STATE_RAMP_UP:
            handle_touch_state_ramp_up(tracker, value, is_touch);
            break;
        case TOUCH_STATE_SUSTAIN:
            handle_touch_state_sustain(tracker, value, is_touch);
            break;
        case TOUCH_STATE_RAMP_DOWN:
            handle_touch_state_ramp_down(tracker, value, is_touch);
            break;
    }
}

void handle_touch_state_off(touch_tracker_t *tracker, uint8_t *value, bool is_touch) {
    if (!is_touch) {
        (*value) = 0;
        return;
    }
    // tracker->touch_state = TOUCH_STATE_RAMP_UP;
    // (*value) = ramp_up_inc;
    (*value) = 255;
    tracker->touch_state = TOUCH_STATE_SUSTAIN;
}

void handle_touch_state_ramp_up(touch_tracker_t *tracker, uint8_t *value, bool is_touch) {
    // leaving ramp up b/c pad is no longer touched
    if (!is_touch) {
        if ((*value) > ramp_up_inc) {
            // if we were at some decent value, go ahead and ramp down form current position
            tracker->touch_state = TOUCH_STATE_RAMP_DOWN;
            // might call handle_ramp_down here
        } else {
            // otherwise, lets just go ahead and say its off
            tracker->touch_state = TOUCH_STATE_OFF;
            (*value) = 0;
        }
        return;
    }
    (*value) = qadd8((*value), ramp_up_inc);
    if ((*value) == 255) {
        tracker->touch_state = TOUCH_STATE_SUSTAIN;
    }
}

void handle_touch_state_sustain(touch_tracker_t *tracker, uint8_t *value, bool is_touch) {
    if (is_touch) {
        (*value) = 255;
        return;
    }
    // tracker->touch_state = TOUCH_STATE_RAMP_DOWN;
    // (*value) = 255 - ramp_down_inc;
    (*value) = 0;
    tracker->touch_state = TOUCH_STATE_OFF;
}
void handle_touch_state_ramp_down(touch_tracker_t *tracker, uint8_t *value, bool is_touch) {
    // leaving ramp down b/c pad is being touched
    if (is_touch) {
        if ((*value) < (255 - ramp_down_inc)) {
            tracker->touch_state = TOUCH_STATE_RAMP_UP;
            // might call handle_ramp_up here
        } else {
            // otherwise, lets just go ahead and say its on
            tracker->touch_state = TOUCH_STATE_SUSTAIN;
            (*value) = 255;
        }
        return;
    }
    (*value) = qsub8((*value), ramp_down_inc);
    if ((*value) == 0) {
        tracker->touch_state = TOUCH_STATE_OFF;
    }
}