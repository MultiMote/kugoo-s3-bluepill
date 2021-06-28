#include <stdio.h>
#include <string.h>
#include "globals.h"
#include "gui.h"
#include "hardware.h"
#include "kugoo_s3.h"
#include "persistence.h"
#include "ssd1306.h"
#include "utils.h"


void usart1_isr(void) {
  // while data is not empty
  while (USART_SR(USART1) & USART_SR_RXNE) {
    kugoo_s3_byte_received(usart_recv(USART1));
  }
}

int main(void) {
  reset_reason = RCC_CSR;
  RCC_CSR |= RCC_CSR_RMVF;

  memset((void *)&kugoo_s3_rx, 0, sizeof(struct kugoo_s3_packet_in));
  uint32_t next_packet_send_time;
  uint32_t next_logic_refresh;
  uint32_t next_throttle_lock_cancellation = 0;
  uint16_t throttle_trigger = 0;
  uint8_t prev_throttle_lock_state = 0;
  uint8_t throttle_locked = 0;
  uint8_t zero_packets_left = 0; // anti throttle lock zero packets
  enum control_state_t control_state = CS_NORMAL;

  float desired_speed_kmh = 0;
  float control_output_kmh = 0;
  float speed_error = 0;
  float current_error = 0;
  float speed = 0;

  hardware_setup();

  eeprom_read_and_validate();

  beep_blocking(1000, 10, storage.keys_volume);

  ssd1306_init();
  ssd1306_contrast(0xf0);
  ssd1306_blend_mode(SSD1306_BLEND_MODE_LIGHTEN);
  ssd1306_fill(0xff); // screen test
  ssd1306_redraw();
  msleep(200);


  if (beep_button_pressed()) {
    ssd1306_clear();
    ssd1306_string("СБРОС НАСТРОЕК\n\n"
                   "СОХРАНИТЬ ОДОМЕТР:\n"
                   "SET\n\n"
                   "СТЕРЕТЬ ВСЁ:\n"
                   "LIGHT");
    ssd1306_redraw();

    while (1) {
      if (set_button_pressed()) {
        uint32_t distance = storage.total_distance_meters;
        eeprom_reset_all(false);
        storage.total_distance_meters = distance;
        eeprom_write_all();
        beep_blocking(2000, 300, STATIC_BEEP_VOLUME);
        power_disable();
      } else if (light_button_pressed()) {
        eeprom_reset_all(true);
        beep_blocking(2000, 300, STATIC_BEEP_VOLUME);
        power_disable();
      } else if (power_button_pressed()) {
        break;
      }
      msleep(20);
    }
  }

  while (power_button_pressed()) {
    msleep(20);
  }

  kugoo_s3_packet_init(&kugoo_s3_tx);
  next_packet_send_time = millis() + PACKET_SEND_INTERVAL_MS;
  next_logic_refresh = millis() + LOGIC_REFRESH_INTERVAL_MS;

  beep_blocking(2000, 10, storage.keys_volume);

  while (1) {

    // update only when new speed and current received
    if (kugoo_s3_tx_ready) {
      kugoo_s3_tx_ready = false;

      // control loop
      speed = kugoo_s3_get_speed();
      speed_error = desired_speed_kmh - speed;
      current_error = storage.current_limit - ((float) kugoo_s3_rx.current / 10.0f);

      // when current overflows maximum current, switch to current limiting
      // instead of speed stabilization
      if(control_state == CS_NORMAL) {
        if (storage.current_limit_enabled &&
          storage.current_limit > 0 &&
          current_error < 0) {
          control_output_kmh += current_error * storage.current_stabilization_Kp;
        } else if(storage.speed_stabilization_enabled) {
          control_output_kmh += speed_error * storage.speed_stabilization_Kp;
        }
      }

      try_persist_distance();
    }

    if (millis() >= next_logic_refresh) {
      next_logic_refresh = millis() + LOGIC_REFRESH_INTERVAL_MS;
      iwdg_reset();
      process_events();

      speed = kugoo_s3_get_speed();

      if(gui_is_view(GUI_VIEW_TRIGGER_CALIBRATION)){
        throttle_trigger = 0;
        kugoo_s3_tx.brake = 0;
      } else {
        throttle_trigger = throttle_value_normalized();
        kugoo_s3_tx.brake = brake_value_normalized();
      }

      throttle_locked = kugoo_s3_rx.state & KUGOO_S3_STATE_THROTTLE_LOCKED;
      if (prev_throttle_lock_state != throttle_locked) {
        prev_throttle_lock_state = throttle_locked;
        if (throttle_locked && !storage.anti_throttle_lock) {
          beep(2000, 10, storage.signals_volume);
        }
      }


      // factory cruise control cancellation
      // sometimes doesn't work
      // fuck this
      if (throttle_locked && storage.anti_throttle_lock && millis() > next_throttle_lock_cancellation) {
          if(control_state == CS_NORMAL) {
            next_throttle_lock_cancellation = millis() + ANTI_THROTTLE_LOCK_PERIOD_MS;
            control_state = CS_FORCE_ZERO_THROTTLE;
            zero_packets_left = ANTI_THROTTLE_LOCK_ZERO_PACKETS;
          }
      }

      // todo: reduce complexity
      if (control_state == CS_THROTTLE_RECOVER) {
        desired_speed_kmh += THROTTLE_RECOVER_INCREMENT;
        if (cruise_ctl_status == CRUISE_CTL_DISABLED) {
          if (desired_speed_kmh >= throttle_value_to_kmh()) {
            desired_speed_kmh = throttle_value_to_kmh();
            control_state = CS_NORMAL;
          }
        } else {
          if (desired_speed_kmh >= cruise_ctl_speed_kmh) {
            desired_speed_kmh = cruise_ctl_speed_kmh;
            control_state = CS_NORMAL;
          }
        }
      } else if (cruise_ctl_status == CRUISE_CTL_DISABLED) {
        if (throttle_trigger > 10) { // throttle trigger  pressed
          if (control_state == CS_FORCE_ZERO_THROTTLE) {
            // do nothing
          } else if (storage.soft_start_enabled) {
            if (desired_speed_kmh < speed / 2) {
              desired_speed_kmh =
                  speed / 2; // do not start with zero when speed > 0
            } else if (desired_speed_kmh + storage.soft_start_increment <=
                       throttle_value_to_kmh()) {
              desired_speed_kmh += storage.soft_start_increment;
            } else {
              desired_speed_kmh = throttle_value_to_kmh();
            }
          } else {
            desired_speed_kmh = throttle_value_to_kmh();
          }
        } else { // throttle trigger not pressed
          desired_speed_kmh = 0;
          control_output_kmh = 0;
        }
      } else if (cruise_ctl_status == CRUISE_CTL_WAITING_RELEASE && throttle_trigger < 50) {
        desired_speed_kmh = cruise_ctl_speed_kmh;
        cruise_ctl_status = CRUISE_CTL_ENABLED;
      } else if (cruise_ctl_status == CRUISE_CTL_ENABLED && throttle_trigger > 50) {
        desired_speed_kmh = cruise_ctl_speed_kmh;
        cruise_ctl_status = CRUISE_CTL_DISABLED;
        beep(2000, 10, storage.signals_volume);
      }

      if (kugoo_s3_tx.brake > 50) {
        cruise_ctl_status = CRUISE_CTL_DISABLED;
        control_output_kmh = 0;
      }

      // Do not run throttle lock cancellation while idle
      if(desired_speed_kmh == 0) {
        next_throttle_lock_cancellation = millis() + ANTI_THROTTLE_LOCK_PERIOD_MS;
      }

      if(control_state == CS_FORCE_ZERO_THROTTLE && zero_packets_left > 0){
        kugoo_s3_tx.throttle = 0;
        if(--zero_packets_left == 0) {
          control_state = CS_NORMAL;
        }
      } else {
        kugoo_s3_set_speed_approx(desired_speed_kmh + control_output_kmh);
      }

      kugoo_s3_tx.features = KUGOO_S3_FEATURE_UNLOCK;
      if (storage.zero_start_enabled) {
        kugoo_s3_tx.features |= KUGOO_S3_FEATURE_ZERO_START;
      }
    }

    if (millis() >= next_packet_send_time) {
      next_packet_send_time = millis() + PACKET_SEND_INTERVAL_MS;
      kugoo_s3_packet_send(&kugoo_s3_tx);
      // printf("%d;%d;%d;%d;%d\r\n", throttle, (int)desired_speed_kmh, kugoo_s3_tx.throttle, (int)kugoo_s3_get_speed(), (int)(kugoo_s3_rx.current/10));
    }

    gui_redraw();
  }

  return 0;
}
