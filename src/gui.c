#include "gui.h"
#include "globals.h"
#include "hardware.h"
#include "keyboard.h"
#include "kugoo_s3.h"
#include "persistence.h"
#include "ssd1306.h"
#include "utils.h"
#include <stdio.h>

#include "views/detailed_view.h"
#include "views/main_view.h"
#include "views/settings_view.h"
#include "views/trigger_calibration_view.h"
#include "views/last_trips_view.h"

static uint32_t next_display_refrash_time = 0;
static uint32_t last_activity_time = 0;

static const struct view_t views[] = {
    {
     .id = GUI_VIEW_MAIN,
     .on_draw = main_view_redraw,
     .on_key = main_view_keyhandler,
     .on_open = NULL
    },
    {
     .id = GUI_VIEW_MAIN_DETAILED,
     .on_draw = detailed_view_redraw,
     .on_key = main_view_keyhandler,
     .on_open = NULL
    },
    {
     .id = GUI_VIEW_SETTINGS,
     .on_draw = settings_view_redraw,
     .on_key = settings_view_keyhandler,
     .on_open = NULL
    },
    {
     .id = GUI_VIEW_TRIGGER_CALIBRATION,
     .on_draw = trigger_calibration_view_redraw,
     .on_key = trigger_calibration_view_keyhandler,
     .on_open = trigger_calibration_view_reset
    },
    {
     .id = GUI_VIEW_LAST_TRIPS,
     .on_draw = last_trips_view_redraw,
     .on_key = last_trips_view_keyhandler,
     .on_open = NULL
    }
  };
static const uint8_t views_count = sizeof(views) / sizeof(struct view_t);

const struct view_t *current_view = views;

void process_events(void) {
  keyboard_poll();
  uint16_t key_evt = keyboard_pop_event();

  if(key_evt ||
     kugoo_s3_get_speed() > 0  ||
     brake_value_normalized() > 0 ||
     throttle_value_normalized() > 0) {
    last_activity_time = millis();
  }

  if ((gui_is_view(GUI_VIEW_MAIN) ||
       gui_is_view(GUI_VIEW_MAIN_DETAILED)) &&
      key_evt & KB_EVT_TYPE_REPEAT &&
      key_evt & KB_EVT_KEY_POWER) {

    force_persist_distance();
    persist_last_trip();
    eeprom_persist(&storage.speed_limit_last);

    beep_blocking(500, 10, storage.keys_volume);
    ssd1306_clear();
    ssd1306_redraw();
    power_disable();
  }

  if (current_view->on_key) {
    current_view->on_key(key_evt);
  }
}

static void error_message(const char *msg) {
  ssd1306_framebuffer_setpos(1, 2);
  ssd1306_font_scale(2);
  ssd1306_string("ТОВАРИЩ!");
  ssd1306_framebuffer_setpos(4, 0);
  ssd1306_font_scale(1);
  ssd1306_string(msg);
  ssd1306_redraw();
}

void gui_redraw(void) {
  if (millis() < next_display_refrash_time) {
    return;
  }

  next_display_refrash_time = millis() + DISPLAY_REFRESH_INTERVAL_MS;

  ssd1306_clear();

  if((millis() - last_activity_time) > SCREENSAVER_INACTIVITY_PERIOD_MS) {
    ssd1306_font_scale(2);
    ssd1306_framebuffer_setpos(millis() / 5 % 7, (millis() / 3) % (SSD1306_WIDTH - 24));
    ssd1306_string(":)");
    ssd1306_redraw();
    return;
  }

  ssd1306_font_scale(1);

  if (gui_is_view(GUI_VIEW_MAIN) || gui_is_view(GUI_VIEW_MAIN_DETAILED)) {
    // throttle lock is not an error
    if (kugoo_s3_rx.state & ~KUGOO_S3_STATE_THROTTLE_LOCKED) {
      if (kugoo_s3_rx.state & KUGOO_S3_STATE_MOTOR_ERROR) {
        error_message("ОТКАЗ\nМОТОР-КОЛЕСА!");
      } else if (kugoo_s3_rx.state & KUGOO_S3_STATE_OVERCURRENT) {
        error_message("ПЕРЕТОК!");
      } else {
        error_message("КОНТРОЛЛЕР\nМОТОР-КОЛЕСА\nИЗВЕЩАЕТ ОБ ОШИБКЕ!");
      }
      return;
    } else if (millis() - kugoo_s3_last_packet_time() > 2000) {
      error_message("КОНТРОЛЛЕР\nМОТОР-КОЛЕСА\nНЕ ОТВЕЧАЕТ!");
      return;
    } else if (brake_adc_value() < 500) {
      error_message("ПОДКЛЮЧИ РУЧКУ\nТОРМОЗА!");
      return;
    } else if (throttle_adc_value() < 500) {
      error_message("ПОДКЛЮЧИ РУЧКУ\nАКСЕЛЕРАТОРА!");
      return;
    }
  }

  if (current_view->on_draw) {
    current_view->on_draw();
  }

  ssd1306_redraw();
}

void gui_redraw_force(void) {
  next_display_refrash_time = 0;
  gui_redraw();
}

void gui_set_view(enum view_id_t id) {
  for (uint8_t i = 0; i < views_count; i++) {
    if (views[i].id == id) {
      current_view = &views[i];
      if (current_view->on_open) {
        current_view->on_open();
      }
    }
  }
}

uint8_t gui_is_view(enum view_id_t id) { return current_view->id == id; }
