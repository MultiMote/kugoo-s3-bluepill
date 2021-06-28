#include "kugoo_s3.h"
#include "hardware.h"
#include "persistence.h"
#include "utils.h"
#include <stdio.h>

volatile struct kugoo_s3_packet_in kugoo_s3_rx;
struct kugoo_s3_packet_out kugoo_s3_tx;
volatile uint8_t kugoo_s3_tx_ready = false;

static volatile uint8_t packet_in_buf[8];
static volatile uint8_t packet_in_buf_pos = 0;
static volatile uint32_t last_packet_time = 0;

void kugoo_s3_packet_init(struct kugoo_s3_packet_out *packet) {
  packet->header = KUGOO_S3_PACKET_OUT_HEADER;
  packet->features = KUGOO_S3_FEATURE_UNLOCK;
  packet->n_a_1 = 0U;
  packet->magnets_count = storage.magnets_count;
  packet->config_batt_voltage1 = KUGOO_S3_BATTERY_VOLTAGE;
  packet->brake = 0U;
  packet->throttle = 0U;
  packet->config_batt_voltage2 = KUGOO_S3_BATTERY_VOLTAGE;
  packet->n_a_2 = 0U;
  packet->speed_limit = 0U;
  packet->speed_level = 3U;
}

static void kugoo_s3_packet_update_checksum(struct kugoo_s3_packet_out *p) {
  uint8_t *ptr = (uint8_t*) p;

  p->checksum = 0;
  for(uint8_t i = 0; i < sizeof(struct kugoo_s3_packet_out) - 1; ++i) {
    p->checksum ^= *ptr;
    ++ptr;
  }
}

void kugoo_s3_packet_send(struct kugoo_s3_packet_out *p) {
  kugoo_s3_packet_update_checksum(p);
  usart_send_blocking(KUGOO_S3_USART, p->header);
  usart_send_blocking(KUGOO_S3_USART, p->features);
  usart_send_blocking(KUGOO_S3_USART, p->n_a_1);
  usart_send_blocking(KUGOO_S3_USART, p->magnets_count);
  usart_send_blocking(KUGOO_S3_USART, (p->config_batt_voltage1 >> 8) & 0xFF);
  usart_send_blocking(KUGOO_S3_USART, p->config_batt_voltage1 & 0xFF);
  usart_send_blocking(KUGOO_S3_USART, (p->brake >> 8) & 0xFF);
  usart_send_blocking(KUGOO_S3_USART, p->brake & 0xFF);
  usart_send_blocking(KUGOO_S3_USART, (p->throttle >> 8) & 0xFF);
  usart_send_blocking(KUGOO_S3_USART, p->throttle & 0xFF);
  usart_send_blocking(KUGOO_S3_USART, (p->config_batt_voltage2 >> 8) & 0xFF);
  usart_send_blocking(KUGOO_S3_USART, p->config_batt_voltage2 & 0xFF);
  usart_send_blocking(KUGOO_S3_USART, p->n_a_2);
  usart_send_blocking(KUGOO_S3_USART, p->speed_limit);
  usart_send_blocking(KUGOO_S3_USART, p->speed_level);
  usart_send_blocking(KUGOO_S3_USART, p->checksum);
  //printf("%X %X\r\n", (p->throttle >> 8) & 0xFF,  p->throttle & 0xFF);
}


float kugoo_s3_get_speed(void) {
  // last_packet.ms_per_rev - время в миллисекундах, за которое колесо делает полный
  // оборот диаметр колеса - 19 см 1 оборот - 2*PI*r = 2 * 3.141592653589793 *
  // (19/2)
  //    = 59.690260418206066 см = 0.5969026041820606м
  // допустим, время оборота = 500мс, 0.5969м за 500мс
  // важно умножить сначала на 1000, а не на (1000/500), чтобы не потерять
  // точность сантиметры в секунду = (59.69 * 1000) / 500 = 119.38 см/с

  // Как перевести метры в секунду в километры в час:
  // Нужно 1 м/с разделить на 1000 (количество метров в километре)
  // и умножить на 3600 (количество секунд в часе) получаем 3.6 километра в час;
  // ((59.69 * 1000 / (double)last_packet.ms_per_rev) / 100.0) * 3.6;
  // упрощаем
  // (((596.9mm * 10) / last_packet.ms_per_rev * 36) / 100)
  // или для float
  // (596.9mm / (float) last_packet.ms_per_rev) * 3.6f
  // ух.

  if (kugoo_s3_rx.ms_per_rev == 0) { // is this possible?
    return 0;
  } else if (kugoo_s3_rx.ms_per_rev == KUGOO_S3_SPEED_VAL_STOPPED) {
    return 0;
  }

  return ((float) storage.wheel_length_mm / (float) kugoo_s3_rx.ms_per_rev) * 3.6f;
}

void kugoo_s3_set_speed_approx(float kmh) {
  float new_throttle = 0;
  if(kmh <= 0) {
    kugoo_s3_tx.throttle = 0;
    return;
  }
  new_throttle = CONTROLLER_STOPPED_VAL + kmh * THROTTLE_TO_SPEED_COEFF;
  kugoo_s3_tx.throttle = clamp_u16((uint16_t)new_throttle, 0, CONTROLLER_TRIGGER_MAX);
}

static inline void update_distance(void) {
  if (kugoo_s3_rx.ms_per_rev == 0) {
    return;
  } else if (kugoo_s3_rx.ms_per_rev == KUGOO_S3_SPEED_VAL_STOPPED) {
    return;
  }
  // получаем сантиметры в секунду как описано выше,
  // но так как мы получаем пакеты чаще, чем 1 раз в секунду,
  // то делим на количество пакетов в секунду
  uint32_t cm =
      ((storage.wheel_length_mm * 100) / kugoo_s3_rx.ms_per_rev) /
      KUGOO_S3_PACKETS_IN_PER_SECOND;
  session_add_distance(cm);
}

inline uint32_t kugoo_s3_last_packet_time(void) { return last_packet_time; }

void kugoo_s3_byte_received(uint8_t b) {
  static uint16_t ms_per_rev = 0;
  if (b == KUGOO_S3_PACKET_IN_HEADER) {
    packet_in_buf_pos = 0;
  } else if (++packet_in_buf_pos > 7) {
    packet_in_buf_pos = 7;
  }
  packet_in_buf[packet_in_buf_pos] = b;

  // @todo ?
  if (packet_in_buf_pos == 7) {
    uint8_t crc = 0;
    for (uint8_t i = 0; i < 7; i++) {
      crc ^= packet_in_buf[i];
    }

    if (crc == packet_in_buf[7]) {
      kugoo_s3_rx.header = packet_in_buf[0];
      kugoo_s3_rx.state = packet_in_buf[1];
      kugoo_s3_rx.n_a_1 = packet_in_buf[2];
      kugoo_s3_rx.current =
          (((uint16_t)packet_in_buf[3]) << 8) | packet_in_buf[4];
      ms_per_rev = (((uint16_t)packet_in_buf[5]) << 8) | packet_in_buf[6];
      if (ms_per_rev > 20) { // 20ms is ~100km/h (trash measured or you are an idiot)
         kugoo_s3_rx.ms_per_rev = ms_per_rev;
      }
      kugoo_s3_rx.checksum = crc;

      last_packet_time = millis();
      update_distance();
      kugoo_s3_tx_ready = true;
    }
  }
}
