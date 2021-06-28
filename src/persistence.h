#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include <stdint.h>
#include "globals.h"

#define EEPROM_I2C_ADDRESS 0x50
#define EEPROM_I2C_BUS I2C1
#define EEPROM_DATA_START_ADDRESS 0x10
#define EEPROM_PAGE_SIZE 16U

#pragma pack(push, 1)
struct last_trip_info_t {
  uint16_t minutes;
  uint16_t meters;
};

/// Struct is automatically mapped to the EEPROM.
/// Do not change order of variables without settings reset.
struct eeprom_data_t {
  uint8_t contrast;
  uint8_t mute;
  uint32_t total_distance_meters;
  uint8_t anti_throttle_lock; ///< Factory cruise control cancellation
  uint16_t keys_volume;
  uint16_t signals_volume;
  uint16_t wheel_length_mm;
  uint16_t magnets_count;
  uint8_t zero_start_enabled;
  uint8_t soft_start_enabled;
  float soft_start_increment;
  uint8_t current_limit_enabled;
  float current_limit;
  float current_stabilization_Kp;
  uint8_t speed_stabilization_enabled;
  float speed_stabilization_Kp;
  uint8_t calibrate_triggers_on_startup;
  uint16_t brake_trigger_min_value;
  uint16_t brake_trigger_max_value;
  uint16_t throttle_trigger_min_value;
  uint16_t throttle_trigger_max_value;
  uint8_t speed_limit_last;
  uint8_t persist_speed_limit;
  struct last_trip_info_t last_trips[8];
};
#pragma pack(pop)

struct float_config_entry_t {
  float* ptr;
  float _min;
  float _max;
  float _default;
};

struct u16_config_entry_t  {
  uint16_t* ptr;
  uint16_t _min;
  uint16_t _max;
  uint16_t _default;
};

extern struct eeprom_data_t storage;
#define STORAGE_SIZE sizeof(storage)


// split bytes to pages and write
void eeprom_set_bytes(uint8_t addr, const void *ptr, uint8_t count);
void eeprom_read_bytes(uint8_t addr, void *ptr, uint8_t count);

void eeprom_read_and_validate(void);
void eeprom_reset_all(uint8_t commit);
void eeprom_write_all(void);

/// Persist variable inside @ref storage to the EEPROM
void eeprom_write_entry(void* variable_addr, uint8_t variable_size);

/// Helper for eeprom_write_entry
#define eeprom_persist(variable_addr) (eeprom_write_entry(variable_addr, sizeof(*variable_addr)))


void try_persist_distance(void);
void force_persist_distance(void);
void session_add_distance(uint16_t centimeters);
/// Current session distance in centimeters
uint32_t session_distance_cm();
void persist_last_trip();

#endif /* PERSISTENCE_H */
