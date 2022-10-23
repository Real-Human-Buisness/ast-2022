
#ifndef TOUCH_TRACKER_H
#define TOUCH_TRACKER_H

#include "esp_check.h"

enum touch_state_e {
    TOUCH_STATE_OFF,
    TOUCH_STATE_RAMP_UP,
    TOUCH_STATE_SUSTAIN,
    TOUCH_STATE_RAMP_DOWN
};
typedef enum touch_state_e touch_state_t;

struct touch_tracker_s {
    touch_state_t touch_state;
};
typedef struct touch_tracker_s touch_tracker_t;

esp_err_t initialize_touch_tracker(void);

void update_touch_tracker(uint8_t position, uint8_t *value, bool is_touch);

#endif //TOUCH_TRACKER_H