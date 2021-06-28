#include "main_view.h"
#include "globals.h"
#include "gui.h"
#include "hardware.h"
#include "persistence.h"
#include "keyboard.h"
#include "kugoo_s3.h"
#include "ssd1306.h"
#include "utils.h"
#include <stdio.h>

float speed_limit_prev = 0;
uint32_t speed_limit_changed_at;

void main_view_keyhandler(uint8_t e) {
  if (e & KB_EVT_TYPE_KEYDOWN) {
    if (e & KB_EVT_KEY_BEEP) {
      buzzer_freq_vol(1000, 255);
    } else if (e & KB_EVT_KEY_SET) {
      // enable cruise control when throttle trigger pressed
      if(throttle_value_normalized() > 10) {
        // cruise_control_speed_kmh = kugoo_s3_get_speed();
        // todo: maybe use current speed, not throttle trigger
        cruise_ctl_speed_kmh = throttle_value_to_kmh();
        cruise_ctl_status = CRUISE_CTL_WAITING_RELEASE;
      } else {
        gui_set_view(GUI_VIEW_SETTINGS);
        gui_redraw_force();
      }

    } else if (e & KB_EVT_KEY_LIGHT) {
      beep_blocking(2000, 10, storage.keys_volume);
      light_toggle();
    } else if (e & KB_EVT_KEY_SPEED) {
      beep_blocking(3000, 10, storage.keys_volume);
      if(brake_value_normalized() > 10) {
        storage.current_limit_enabled = !storage.current_limit_enabled;
        eeprom_persist(&storage.current_limit_enabled);
      } else {
        incr_u8_loop(&storage.speed_limit_last, 10, 30, 5);
        gui_redraw_force();
      }
    }
  } else if (e & KB_EVT_TYPE_KEYUP) {
    if (e & KB_EVT_KEY_BEEP) {
      buzzer_freq_vol(0, 255);
    } else if (e & KB_EVT_KEY_POWER) {
      if (gui_is_view(GUI_VIEW_MAIN)) {
        gui_set_view(GUI_VIEW_MAIN_DETAILED);
      } else if (gui_is_view(GUI_VIEW_MAIN_DETAILED)) {
        gui_set_view(GUI_VIEW_MAIN);
      }
      gui_redraw_force();
    }
  }
}

void main_view_redraw() {
  uint16_t current = kugoo_s3_rx.current;
  uint32_t ms = millis();
  uint16_t speed = kugoo_s3_get_speed();
  uint16_t batt = battery_value();
  uint16_t batt_clamp =
      clamp_u16(batt, BATTERY_VOLTS_0_PERCENTS, BATTERY_VOLTS_100_PERCENTS);

  uint16_t batt_pixels = ((batt_clamp - BATTERY_VOLTS_0_PERCENTS) * 83) /
                    (BATTERY_VOLTS_100_PERCENTS - BATTERY_VOLTS_0_PERCENTS);

  uint32_t distance_m = session_distance_cm() / 100;


  if(speed_limit_prev != storage.speed_limit_last) {
    speed_limit_prev = storage.speed_limit_last;
    speed_limit_changed_at = ms;
  }

  if(speed_limit_changed_at - ms < 2000) {
    ssd1306_framebuffer_setpos(0, 20);
    ssd1306_font_scale(8);

    sprintf(sprintf_buf, "%u", storage.speed_limit_last);
    ssd1306_string(sprintf_buf);
    return;
  }

  if (batt > BATTERY_VOLTS_0_PERCENTS || (ms / 500) % 2 == 0) {
    // left battery cap
    ssd1306_framebuffer_setpos(0, 39);
    ssd1306_framebuffer_byte(0x7F);
    ssd1306_framebuffer_byte(0x41);


    // right battery cap
    ssd1306_framebuffer_setpos(0, SSD1306_WIDTH - 4);
    ssd1306_framebuffer_byte(0x41);
    ssd1306_framebuffer_byte(0x7F);
    ssd1306_framebuffer_byte(0x1C);

    // battery progressbar
    ssd1306_framebuffer_setpos(0, 41);
    for (uint8_t i = 0; i < 83; i++) {
      if (i < batt_pixels) {
        ssd1306_framebuffer_byte(0x5D);
      } else {
        ssd1306_framebuffer_byte(0x41);
      }
    }
  }


  // vertical line
  for (uint8_t i = 0; i < 8; i++) {
    ssd1306_framebuffer_setpos(i, 37);
    ssd1306_framebuffer_byte(0x55);
  }

  // bottom horizontal line
  ssd1306_framebuffer_setpos(6, 0);
  for (uint8_t i = 0; i < 19; i++) {
    ssd1306_framebuffer_byte(0x80);
    ssd1306_framebuffer_byte(0x00);
  }

  if (current > 0 && millis() / 1000 % 7 > 3) {
    // current
    ssd1306_framebuffer_setpos(3, 41);
    ssd1306_font_scale(4);
    sprintf(sprintf_buf, "%d", current / 10);
    ssd1306_string(sprintf_buf);
    ssd1306_framebuffer_nextline(false);
    ssd1306_framebuffer_nextline(false);
    ssd1306_font_scale(2);
    sprintf(sprintf_buf, ".%d", current % 10);
    ssd1306_string(sprintf_buf);
    ssd1306_font_scale(1);
    ssd1306_framebuffer_nextline(false);
    ssd1306_string(" A");
  } else {
    // speeed
    ssd1306_framebuffer_setpos(0, 39);
    ssd1306_font_scale(8);
    if (speed > 99) {
      ssd1306_string("ЖП");
    } else {
      sprintf(sprintf_buf, "%02d", speed);
      ssd1306_string(sprintf_buf);
    }
  }

  ssd1306_font_scale(1);
  ssd1306_framebuffer_setpos(7, 3);
  sprintf(sprintf_buf, "%02lu%c%02lu", (ms / 1000) / 60,
          (ms / 500) % 2 == 0 ? ':' : ' ', (ms / 1000) % 60);
  ssd1306_string(sprintf_buf);

  // current limit indicator
  if(storage.current_limit_enabled) {
    ssd1306_font_scale(2);
    ssd1306_framebuffer_setpos(2, 0);
    ssd1306_string("LIM");
  }

  // curent session distance
  ssd1306_font_scale(2);
  ssd1306_framebuffer_setpos(4, 0);
  if(distance_m < 99) {
    sprintf(sprintf_buf, "%lu", distance_m);
    ssd1306_string(sprintf_buf);
    ssd1306_font_scale(1);
    ssd1306_framebuffer_nextline(false);
    ssd1306_string("М");
  } else {
    uint32_t km =  distance_m / 1000;
    sprintf(sprintf_buf, "%lu", km);
    ssd1306_string(sprintf_buf);

    ssd1306_font_scale(1);
    ssd1306_framebuffer_nextline(false);
    if(km < 10) {
      sprintf(sprintf_buf, ".%luКМ", distance_m % 1000 / 100);
    } else {
      sprintf(sprintf_buf, "КМ");
    }
    ssd1306_string(sprintf_buf);
  }

  // todo: trottle lock indicator, remove
  if(kugoo_s3_rx.state & KUGOO_S3_STATE_THROTTLE_LOCKED) {
    ssd1306_framebuffer_setpos(0, 0);
    for (uint8_t i = 0; i < 24; i++) {
      ssd1306_framebuffer_byte(0xFF);
    }
  }
}
