#include "last_trips_view.h"
#include <stdio.h>
#include <string.h>
#include "persistence.h"
#include "keyboard.h"
#include "gui.h"
#include "ssd1306.h"
#include "hardware.h"


void last_trips_view_keyhandler(uint8_t e) {
  if (e & KB_EVT_TYPE_KEYDOWN) {
    beep(KEY_BEEP_FREQ, 10, storage.keys_volume);
  }

  if (e & KB_EVT_TYPE_KEYDOWN && e & KB_EVT_KEY_BEEP) {
    gui_set_view(GUI_VIEW_SETTINGS);
  } else if (e & KB_EVT_KEY_SPEED && e & KB_EVT_TYPE_REPEAT) {
    memset(&storage.last_trips, 0U, sizeof(storage.last_trips));
    eeprom_persist(&storage.last_trips);
  }
}

void last_trips_view_redraw() {
  struct last_trip_info_t *info = storage.last_trips;
  uint8_t i = 0;

  for (; i < 8; i++) {
    if(info->minutes == 0 && info->meters == 0){
      break;
    }
    ssd1306_framebuffer_setpos(i, 0);

    if(info->minutes < 60) {
      sprintf(sprintf_buf, "%d МИН - ", info->minutes);
    } else {
      sprintf(sprintf_buf, "%d.%d Ч", info->minutes / 60, info->meters % 60 / 6);
    }
    ssd1306_string(sprintf_buf);

    if(info->meters < 1000) {
      sprintf(sprintf_buf, "%d М", info->meters);
    } else {
      sprintf(sprintf_buf, "%d.%d КМ", info->meters / 1000, info->meters % 1000 / 100);
    }
    ssd1306_string(sprintf_buf);

    ++info;
  }

  if(i == 0) {
    // 6 = font width (5) + interval (1)
    // 5 - symbols count
    ssd1306_framebuffer_setpos(3, (millis() / 50) % (SSD1306_WIDTH - 6 * 5));
    ssd1306_string("ПУСТО");
  }

}
