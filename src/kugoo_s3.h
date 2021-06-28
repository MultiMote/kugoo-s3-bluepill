#ifndef KUGOO_S3_H
#define KUGOO_S3_H

#include <stdint.h>

#define KUGOO_S3_USART USART1

#define KUGOO_S3_PACKET_OUT_HEADER 0x2F
#define KUGOO_S3_PACKET_IN_HEADER 0x28
#define KUGOO_S3_SPEED_VAL_STOPPED 0xbb8

/// 1000ms in second / 200ms period, for speed calculation
#define KUGOO_S3_PACKETS_IN_PER_SECOND 5


enum kugoo_s3_battery_voltage_t {
  KUGOO_S3_BATTERY_VOLTAGE_24V = 0x00D2,
  KUGOO_S3_BATTERY_VOLTAGE_36V = 0x0136,
  KUGOO_S3_BATTERY_VOLTAGE_48V = 0x019A
};

#define KUGOO_S3_BATTERY_VOLTAGE KUGOO_S3_BATTERY_VOLTAGE_36V

enum kugoo_s3_state_t {
  KUGOO_S3_STATE_MOTOR_ERROR = 0x01,
  KUGOO_S3_STATE_THROTTLE_LOCKED = 0x02, ///< Not an error
  KUGOO_S3_STATE_OVERCURRENT = 0x08
};

enum kugoo_s3_feature_t {
  KUGOO_S3_FEATURE_UNLOCK = 0x01,
  KUGOO_S3_FEATURE_ZERO_START = 0x02,
  KUGOO_S3_FEATURE_REAR_LIGHT = 0x04,
  // KUGOO_S3_FEATURE_4 = 0x08,
  // KUGOO_S3_FEATURE_5 = 0x10,
  // KUGOO_S3_FEATURE_6 = 0x20,
  // KUGOO_S3_FEATURE_7 = 0x40,
  // KUGOO_S3_FEATURE_8 = 0x80,
};

#pragma pack(push, 1)

/// Packet from me to the wheel controller
struct kugoo_s3_packet_out {
  uint8_t header;   ///< @see KUGOO_S3_PACKET_OUT_HEADER
  uint8_t features; ///< @see kugoo_s3_feature_t
  uint8_t n_a_1;
  uint8_t magnets_count;
  uint16_t config_batt_voltage1; ///< @see kugoo_s3_battery_voltage_t
  uint16_t brake;          ///< 0-1000
  uint16_t throttle;       ///< 0-1000
  uint16_t config_batt_voltage2; ///< @see kugoo_s3_battery_voltage_t
  uint8_t n_a_2;
  uint8_t speed_limit; ///< Unused
  uint8_t speed_level; ///< 1-3
  uint8_t checksum;    ///< All bytes XOR
};

/// Packet from wheel controller to me
struct kugoo_s3_packet_in {
  uint8_t header; ///< @see KUGOO_S3_PACKET_IN  _HEADER
  uint8_t state;  ///< @see kugoo_s3_state_t
  uint8_t n_a_1;
  uint16_t current; ///< Current * 10 (95 = 9.5A)
  uint16_t ms_per_rev;   ///< Milliseconds per full wheel revolution
  uint8_t checksum; ///< All bytes XOR
};
#pragma pack(pop)

/// Last received packet
extern volatile struct kugoo_s3_packet_in kugoo_s3_rx;
/// Last sent packet
extern struct kugoo_s3_packet_out kugoo_s3_tx;
/// Sets to true when packet successfully processed
extern volatile uint8_t kugoo_s3_tx_ready;

void kugoo_s3_packet_init(struct kugoo_s3_packet_out *packet);
void kugoo_s3_packet_send(struct kugoo_s3_packet_out *packet);
void kugoo_s3_try_persist_distance(void);
void kugoo_s3_force_persist_distance(void);

void kugoo_s3_byte_received(uint8_t b);

/// fuck everything, I want to float
float kugoo_s3_get_speed(void);

/// @see KUGOO_S3_SPEED_COEFF
void kugoo_s3_set_speed_approx(float kmh);

uint32_t kugoo_s3_last_packet_time(void);

#endif /* KUGOO_S3_H */
