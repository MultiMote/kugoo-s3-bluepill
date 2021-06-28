#include "keyboard.h"
#include "hardware.h"

static uint16_t last_key_event = KB_EVT_NONE;

static uint32_t beep_btn_pressed_at = 0;
static uint32_t set_btn_pressed_at = 0;
static uint32_t power_btn_pressed_at = 0;
static uint32_t light_btn_pressed_at = 0;
static uint32_t speed_btn_pressed_at = 0;

static uint8_t check_key_event(uint8_t key_state, uint8_t key_id,
                               uint32_t *pressed_at) {
  uint32_t ms_presssed = millis() - *pressed_at;
  if (key_state) {
    if (*pressed_at == 0) {
      *pressed_at = millis();
      last_key_event = key_id | KB_EVT_TYPE_KEYDOWN;
    } else if ((ms_presssed > KEY_REPEAT_DELAY) &&
               (ms_presssed > (KEY_REPEAT_INTERVAL + KEY_REPEAT_DELAY))) {
      *pressed_at += KEY_REPEAT_INTERVAL;
      last_key_event = key_id | KB_EVT_TYPE_REPEAT;
    }
    return 1;
  } else if (!key_state && *pressed_at > 0) {
    last_key_event = key_id | KB_EVT_TYPE_KEYUP;
    *pressed_at = 0;
    return 1;
  }
  return 0;
}

void keyboard_poll(void) {
  if (last_key_event != KB_EVT_NONE) { // no poll if last event unprocessed
    return;
  }

  if (check_key_event(beep_button_pressed(), KB_EVT_KEY_BEEP,
                      &beep_btn_pressed_at)) {
    return;
  }

  if (check_key_event(set_button_pressed(), KB_EVT_KEY_SET,
                      &set_btn_pressed_at)) {
    return;
  }

  if (check_key_event(power_button_pressed(), KB_EVT_KEY_POWER,
                      &power_btn_pressed_at)) {
    return;
  }

  if (check_key_event(light_button_pressed(), KB_EVT_KEY_LIGHT,
                      &light_btn_pressed_at)) {
    return;
  }

  if (check_key_event(speed_button_pressed(), KB_EVT_KEY_SPEED,
                      &speed_btn_pressed_at)) {
    return;
  }
}

uint8_t keyboard_pop_event(void)
{
    uint8_t val = last_key_event;
    last_key_event = KB_EVT_NONE;
    return val;
}
