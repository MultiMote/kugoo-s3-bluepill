#include "trigger_calibration_view.h"
#include "keyboard.h"
#include "gui.h"
#include "ssd1306.h"
#include "globals.h"
#include "hardware.h"
#include "persistence.h"
#include <stdio.h>

static enum trigger_calibration_phase_t current_phase = TRIGGER_CAL_PHASE_NONE;

void trigger_calibration_view_keyhandler(uint8_t e) {
  if (!(e & KB_EVT_TYPE_KEYDOWN)) {
    return;
  }

  beep(KEY_BEEP_FREQ, 10, storage.keys_volume);

  if (e & KB_EVT_KEY_BEEP) {
    gui_set_view(GUI_VIEW_SETTINGS);
  } else if (e & KB_EVT_KEY_POWER) {

    if(current_phase == TRIGGER_CAL_PHASE_NONE) {
      current_phase = TRIGGER_CAL_PHASE_THROTTLE_MIN;

    } else if(current_phase == TRIGGER_CAL_PHASE_THROTTLE_MIN) {
      storage.throttle_trigger_min_value = throttle_adc_value() + TRIGGER_ANTI_JITTER;
      current_phase = TRIGGER_CAL_PHASE_THROTTLE_MAX;

    } else if(current_phase == TRIGGER_CAL_PHASE_THROTTLE_MAX) {
      storage.throttle_trigger_max_value = throttle_adc_value() - TRIGGER_ANTI_JITTER;
      current_phase = TRIGGER_CAL_PHASE_BRAKE_MIN;

    } else if(current_phase == TRIGGER_CAL_PHASE_BRAKE_MIN) {
      storage.brake_trigger_min_value = brake_adc_value() + TRIGGER_ANTI_JITTER;
      current_phase = TRIGGER_CAL_PHASE_BRAKE_MAX;

    } else if(current_phase == TRIGGER_CAL_PHASE_BRAKE_MAX) {
      storage.brake_trigger_max_value = brake_adc_value() - TRIGGER_ANTI_JITTER;
      eeprom_persist(&storage.throttle_trigger_min_value);
      eeprom_persist(&storage.throttle_trigger_max_value);
      eeprom_persist(&storage.brake_trigger_min_value);
      eeprom_persist(&storage.brake_trigger_max_value);
      current_phase = TRIGGER_CAL_PHASE_FINISHED;
    }

    gui_redraw_force();
  }


}

void trigger_calibration_view_redraw() {
  ssd1306_framebuffer_setpos(0, 0);
  ssd1306_string("КАЛИБРОВКА РУЧЕК");
  ssd1306_framebuffer_setpos(2, 0);

  if(current_phase == TRIGGER_CAL_PHASE_NONE) {
    ssd1306_string("ДЛЯ НАЧАЛА\n"
                   "НАЖМИТЕ [POWER]");
  } else if(current_phase == TRIGGER_CAL_PHASE_THROTTLE_MIN) {
    ssd1306_string("ВЫВЕДИТЕ РУЧКУ\n"
                   "[АКСЕЛЕРАТОРА]\n"
                   "В НУЛЕВОЕ ПОЛОЖЕНИЕ\n"
                   "И НАЖМИТЕ [POWER]");
  } else if(current_phase == TRIGGER_CAL_PHASE_THROTTLE_MAX) {
    ssd1306_string("ВЫВЕДИТЕ РУЧКУ\n"
                   "[АКСЕЛЕРАТОРА]\n"
                   "В МАКС. ПОЛОЖЕНИЕ\n"
                   "И НАЖМИТЕ [POWER]");
  } else if(current_phase == TRIGGER_CAL_PHASE_BRAKE_MIN) {
    ssd1306_string("ВЫВЕДИТЕ РУЧКУ\n"
                   "[ТОРМОЗА]\n"
                   "В НУЛЕВОЕ ПОЛОЖЕНИЕ\n"
                   "И НАЖМИТЕ [POWER]");
  } else if(current_phase == TRIGGER_CAL_PHASE_BRAKE_MAX) {
    ssd1306_string("ВЫВЕДИТЕ РУЧКУ\n"
                   "[ТОРМОЗА]\n"
                   "В МАКС. ПОЛОЖЕНИЕ\n"
                   "И НАЖМИТЕ [POWER]");
  } else if(current_phase == TRIGGER_CAL_PHASE_FINISHED) {
    ssd1306_string("ПОЗДРАВЛЯЕМ\n"
                   "ВЫ ПОБЕДИЛИ\n"
                   "ЗНАЧЕНИЯ СОХРАНЕНЫ");
  }

  if(current_phase == TRIGGER_CAL_PHASE_THROTTLE_MIN ||
     current_phase == TRIGGER_CAL_PHASE_THROTTLE_MAX) {
    ssd1306_framebuffer_setpos(7, 0);
    sprintf(sprintf_buf, "ЗНАЧЕНИЕ АЦП: %d", throttle_adc_value());
    ssd1306_string(sprintf_buf);
  } else if(current_phase == TRIGGER_CAL_PHASE_BRAKE_MIN ||
            current_phase == TRIGGER_CAL_PHASE_BRAKE_MAX) {
    ssd1306_framebuffer_setpos(7, 0);
    sprintf(sprintf_buf, "ЗНАЧЕНИЕ АЦП: %d", brake_adc_value());
    ssd1306_string(sprintf_buf);
  } else if(current_phase == TRIGGER_CAL_PHASE_NONE ||
            current_phase == TRIGGER_CAL_PHASE_FINISHED) {
    ssd1306_framebuffer_setpos(6, 0);
    sprintf(sprintf_buf, "УСКОРЕНИЕ %d = %d%%\n"
                         "   ТОРМОЗ %d = %d%%",
                          throttle_adc_value(),
                          throttle_value_normalized() / 10,
                          brake_adc_value(),
                          brake_value_normalized() / 10);
    ssd1306_string(sprintf_buf);
  }

}

void trigger_calibration_view_reset() {
  current_phase = TRIGGER_CAL_PHASE_NONE;
}
