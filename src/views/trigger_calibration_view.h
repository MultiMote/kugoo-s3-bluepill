#ifndef TRIGGER_CALIBRATION_VIEW_H
#define TRIGGER_CALIBRATION_VIEW_H

#include <stdint.h>

enum trigger_calibration_phase_t {
  TRIGGER_CAL_PHASE_NONE,
  TRIGGER_CAL_PHASE_THROTTLE_MIN,
  TRIGGER_CAL_PHASE_THROTTLE_MAX,
  TRIGGER_CAL_PHASE_BRAKE_MIN,
  TRIGGER_CAL_PHASE_BRAKE_MAX,
  TRIGGER_CAL_PHASE_FINISHED
};

void trigger_calibration_view_keyhandler(uint8_t e);
void trigger_calibration_view_redraw();
void trigger_calibration_view_reset();

#endif /* TRIGGER_CALIBRATION_VIEW_H */
