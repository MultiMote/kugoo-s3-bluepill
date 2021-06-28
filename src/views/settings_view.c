#include "settings_view.h"
#include "globals.h"
#include "gui.h"
#include "hardware.h"
#include "keyboard.h"
#include "kugoo_s3.h"
#include "persistence.h"
#include "ssd1306.h"
#include "utils.h"
#include <stdio.h>

/// Selected menu item index
static uint8_t menu_index = 0;
static uint8_t is_editing = 0;
static uint8_t editor_digit_index = 0;

// @see persistence.c
extern struct float_config_entry_t cfg_speed_stabilization_Kp;
extern struct float_config_entry_t cfg_current_stabilization_Kp;
extern struct float_config_entry_t cfg_current_limit;
extern struct float_config_entry_t cfg_soft_start_increment;
extern struct u16_config_entry_t cfg_wheel_length_mm;
extern struct u16_config_entry_t cfg_magnets_count;
extern struct u16_config_entry_t cfg_keys_volume;
extern struct u16_config_entry_t cfg_signals_volume;

#ifdef MENU_SEPARATORS
  #define MENU_SEPARATOR(text) { \
    .title = (text), \
    .type = SETTING_TYPE_NONE, \
    .data = NULL \
  },
#else
  #define MENU_SEPARATOR(text)
#endif

static const struct settings_item_t menu_items[] = {
    {
      .title = "   == ВЕРСИЯ " FIRMWARE_VERSION " ==",
      .type = SETTING_TYPE_NONE,
      .data = NULL
    },
    {
      .title = "ОТКЛЮЧИТЬ ЗВУКИ",
      .type = SETTING_TYPE_TOGGLE,
      .data = &storage.mute
    },
    {
      .title = "ГРОМКОСТЬ КНОПОК",
      .type = SETTING_TYPE_EDIT_U16,
      .data = &cfg_keys_volume
    },
    {
      .title = "ГРОМКОСТЬ СИГНАЛОВ",
      .type = SETTING_TYPE_EDIT_U16,
      .data = &cfg_signals_volume
    },
    MENU_SEPARATOR("")
    {
      .title = "АНТИ-ФИКСАЦИЯ УСКОР.",
      .type = SETTING_TYPE_TOGGLE,
      .data = &storage.anti_throttle_lock
    },
    {
      .title = "ZERO-START",
      .type = SETTING_TYPE_TOGGLE,
      .data = &storage.zero_start_enabled
    },
    MENU_SEPARATOR("")
    {
      .title = "ПЛАВНОЕ УСКОРЕНИЕ",
      .type = SETTING_TYPE_TOGGLE,
      .data = &storage.soft_start_enabled
    },
    {
      .title = "ИНКРЕМ. ПЛАВН. УСКОР.",
      .type = SETTING_TYPE_EDIT_FLOAT,
      .data = &cfg_soft_start_increment
    },
    MENU_SEPARATOR("")
    {
      .title = "ОГРАНИЧЕНИЕ ТОКА",
      .type = SETTING_TYPE_TOGGLE,
      .data = &storage.current_limit_enabled
    },
    {
      .title = "ЗНАЧ. ОГРАНИЧЕН. ТОКА",
      .type = SETTING_TYPE_EDIT_FLOAT,
      .data = &cfg_current_limit
    },
    {
      .title = "КОЭФФ. СТАБИЛИЗ. ТОКА",
      .type = SETTING_TYPE_EDIT_FLOAT,
      .data = &cfg_current_stabilization_Kp
    },
    MENU_SEPARATOR("")
    {
      .title = "СТАБИЛИЗАЦИЯ СКОР.",
      .type = SETTING_TYPE_TOGGLE,
      .data = &storage.speed_stabilization_enabled
    },
    {
      .title = "КОЭФФ. СТАБ. СКОРОСТИ",
      .type = SETTING_TYPE_EDIT_FLOAT,
      .data = &cfg_speed_stabilization_Kp
    },
    MENU_SEPARATOR("")
    {
      .title = "КАЛИБРОВКА РУЧЕК",
      .type = SETTING_TYPE_VIEW_CHANGE,
      .data = (void*)GUI_VIEW_TRIGGER_CALIBRATION
    },
    {
      .title = "АВТОКАЛИБРОВКА РУЧЕК",
      .type = SETTING_TYPE_TOGGLE,
      .data = &storage.calibrate_triggers_on_startup
    },
    {
      .title = "СОХРАНЯТЬ СКОРОСТЬ",
      .type = SETTING_TYPE_TOGGLE,
      .data = &storage.persist_speed_limit
    },
    {
      .title = "ДЛИНА ОКР. КОЛЕСА, ММ",
      .type = SETTING_TYPE_EDIT_U16,
      .data = &cfg_wheel_length_mm
    },
    {
      .title = "КОЛИЧЕСТВО МАГНИТОВ",
      .type = SETTING_TYPE_EDIT_U16,
      .data = &cfg_magnets_count
    },
    MENU_SEPARATOR("")
    {
      .title = "ПОСЛЕДНИЕ ПОЕЗДКИ",
      .type = SETTING_TYPE_VIEW_CHANGE,
      .data = (void*)GUI_VIEW_LAST_TRIPS
    }
  };

static const uint8_t menu_items_count = sizeof(menu_items) / sizeof(struct settings_item_t);

void settings_view_keyhandler(uint8_t e) {
  const struct settings_item_t *item = menu_items + menu_index;

  if (!(e & KB_EVT_TYPE_KEYDOWN || e & KB_EVT_TYPE_REPEAT)) {
    return;
  }

  beep(KEY_BEEP_FREQ, 10, storage.keys_volume);

  if (is_editing) {
    if (e & KB_EVT_KEY_BEEP) { // persist data after closing editor
      is_editing = false;

      if(item->type == SETTING_TYPE_EDIT_FLOAT) {
        struct float_config_entry_t *entry = (struct float_config_entry_t *) item->data;
        eeprom_write_entry(entry->ptr, sizeof(float));
      } else if(item->type == SETTING_TYPE_EDIT_U16) {
        struct u16_config_entry_t *entry = (struct u16_config_entry_t *) item->data;
        eeprom_write_entry(entry->ptr, sizeof(uint16_t));
      }
    } else if (item->type == SETTING_TYPE_EDIT_U16) {
      struct u16_config_entry_t *entry = (struct u16_config_entry_t *) item->data;
      uint8_t digits = digit_count(entry->_max);
      uint16_t step = pow_u16(10, editor_digit_index);
      if (e & KB_EVT_KEY_POWER) {
        incr_u8_loop(&editor_digit_index, 0,  digits - 1, 1);
      } if (e & KB_EVT_KEY_SET) {
        decr_u16_loop(entry->ptr, entry->_min,  entry->_max, step);
      } else if (e & KB_EVT_KEY_LIGHT) {
        incr_u16_loop(entry->ptr, entry->_min,  entry->_max, step);
      } else if (e & KB_EVT_KEY_SPEED) {
        *entry->ptr = entry->_default;
      }
    } else if (item->type == SETTING_TYPE_EDIT_FLOAT) {
      struct float_config_entry_t *entry = (struct float_config_entry_t *) item->data;
      float step = 0.1f;

      if (editor_digit_index > 3) { // integer part
        step = pow_u16(10, editor_digit_index - 4);
      } else if (editor_digit_index == 0) { // 0.1 * 10 ^ -y is too hard
        step = 0.0001f;
      } else if (editor_digit_index == 1) {
        step = 0.001f;
      } else if (editor_digit_index == 2) {
        step = 0.01f;
      }

      if (e & KB_EVT_KEY_POWER) {
        incr_u8_loop(&editor_digit_index, 0, 7, 1);
      } if (e & KB_EVT_KEY_SET) {
        decr_float_loop(entry->ptr, entry->_min, entry->_max, step);
      } else if (e & KB_EVT_KEY_LIGHT) {
        incr_float_loop(entry->ptr, entry->_min, entry->_max, step);
      } else if (e & KB_EVT_KEY_SPEED) {
        *entry->ptr = entry->_default;
      }
    }
  } else {
    if (e & KB_EVT_KEY_SET) {
      decr_u8_loop(&menu_index, 0, menu_items_count - 1, 1);
    } else if (e & KB_EVT_KEY_LIGHT) {
      incr_u8_loop(&menu_index, 0, menu_items_count - 1, 1);
    } else if (e & KB_EVT_KEY_BEEP) {
      gui_set_view(GUI_VIEW_MAIN);
    } else if (e & KB_EVT_KEY_POWER) {
      if (item->type == SETTING_TYPE_TOGGLE) {
        uint8_t *val = (uint8_t *)item->data;
        *val = !*val;
        eeprom_persist(val);
      } else if (item->type == SETTING_TYPE_VIEW_CHANGE) {
        gui_set_view((enum view_id_t)item->data);
      } else if (item->type == SETTING_TYPE_EDIT_U16 || item->type == SETTING_TYPE_EDIT_FLOAT) {
        is_editing = true;
        editor_digit_index = 0;
      }
    }
  }
  gui_redraw_force();
}

void settings_view_redraw() {
  const struct settings_item_t *item;
  uint8_t menu_shift;
  uint8_t new_index;

  if(is_editing) {
    item = menu_items + menu_index;
    ssd1306_framebuffer_setpos(0, 0);
    ssd1306_string(item->title);

    ssd1306_font_scale(2);
    ssd1306_framebuffer_setpos(3, 0);

    if(item->type == SETTING_TYPE_EDIT_U16) {
      struct u16_config_entry_t *params = (struct u16_config_entry_t *) item->data;
      uint8_t digits = digit_count(params->_max);
      if(digits < 6) { // suppress compiler warning
        sprintf(sprintf_buf, "%0*u", digits, *params->ptr);
        ssd1306_string(sprintf_buf);
      }
      ssd1306_blend_mode_push();
      ssd1306_blend_mode(SSD1306_BLEND_MODE_XOR);

      ssd1306_framebuffer_setpos(5, 0);
      for (uint8_t i = 1; i <= digits; i++){
        if(i == (digits - editor_digit_index)) {
          ssd1306_char('^');
        } else {
          ssd1306_char(' ');
        }
      }
      ssd1306_blend_mode_pop();
    } else if(item->type == SETTING_TYPE_EDIT_FLOAT) {
      struct float_config_entry_t *params = (struct float_config_entry_t *) item->data;
      float f = *params->ptr;
      uint16_t integer_part = f;
      uint16_t fractional_part = (f - integer_part) * 10000;
      sprintf(sprintf_buf, "%04d.%04d", integer_part, fractional_part);

      ssd1306_string(sprintf_buf);
      ssd1306_blend_mode_push();
      ssd1306_blend_mode(SSD1306_BLEND_MODE_XOR);
      ssd1306_framebuffer_setpos(5, 0);
      for (uint8_t i = 1; i <= 8; i++) {

        if(i == (8 - editor_digit_index)) {
          if(i > 4) { // do not point to point
            ssd1306_char(' ');
          }
          ssd1306_char('^');
        } else {
          ssd1306_char(' ');
        }
      }
      ssd1306_blend_mode_pop();
    }
    return;
  }

  for (uint8_t i = 0; i < SSD1306_LINES; i++) {
    menu_shift = menu_index / SSD1306_LINES;
    new_index = (menu_shift * SSD1306_LINES) + i;
    if (new_index >= menu_items_count) {
      break;
    }

    item = menu_items + new_index;

    ssd1306_framebuffer_setpos(i, 0);
    ssd1306_string(item->title);

    if(new_index == menu_index) {
      ssd1306_framebuffer_setpos(i, 0);
      ssd1306_blend_mode_push();
      ssd1306_blend_mode(SSD1306_BLEND_MODE_XOR);
      for (uint8_t i = 0; i < SSD1306_WIDTH; i++){
        ssd1306_framebuffer_byte(0xff);
      }
    }

    if (item->type == SETTING_TYPE_TOGGLE) {
      ssd1306_framebuffer_setpos(i, 122);
      ssd1306_string(*((uint8_t *)item->data) ? "✔" : "▪");
    }
  }
}
