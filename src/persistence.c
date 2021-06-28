#include "persistence.h"
#include "hardware.h"
#include "globals.h"
#include "utils.h"
#include <libopencm3/stm32/i2c.h>
#include <string.h>

#define EEPROM_HEADER_SIZE 6
static const uint8_t eeprom_valid_header[EEPROM_HEADER_SIZE] = {
    0x4C, 0x4F, 0x56, 0x45, 0x4C, 0x59};

struct eeprom_data_t storage;

static volatile uint32_t current_session_distance_cm = 0;
static volatile uint32_t distance_cm_accumulator = 0;

struct float_config_entry_t cfg_speed_stabilization_Kp = {
    .ptr = &storage.speed_stabilization_Kp,
    ._min = 0.0f, ._max = 1.0f, ._default = 0.14f,
};

struct float_config_entry_t cfg_current_stabilization_Kp = {
    .ptr = &storage.current_stabilization_Kp,
    ._min = 0.0f, ._max = 5.0f, ._default = 1.0f
};

struct float_config_entry_t cfg_current_limit = {
    .ptr = &storage.current_limit,
    ._min = 0.0f, ._max = 100.0f, ._default = 6.0f
};

struct float_config_entry_t cfg_soft_start_increment = {
    .ptr = &storage.soft_start_increment,
    ._min = 0.1f, ._max = 1.0f, ._default = 0.5f // 0.5 km/h every 50ms
};

struct u16_config_entry_t cfg_throttle_trigger_min_value = {
    .ptr = &storage.throttle_trigger_min_value,
    ._min = 500, ._max = 4096, ._default = 1200
};

struct u16_config_entry_t cfg_brake_trigger_min_value = {
    .ptr = &storage.brake_trigger_min_value,
    ._min = 500, ._max = 4096, ._default = 1200
};

struct u16_config_entry_t cfg_throttle_trigger_max_value = {
    .ptr = &storage.throttle_trigger_max_value,
    ._min = 500, ._max = 4096, ._default = 3050
};

struct u16_config_entry_t cfg_brake_trigger_max_value = {
    .ptr = &storage.brake_trigger_max_value,
    ._min = 500, ._max = 4096, ._default = 3050
};

struct u16_config_entry_t cfg_wheel_length_mm = {
    .ptr = &storage.wheel_length_mm,
    ._min = 300, ._max = 4470, ._default = 597
};

struct u16_config_entry_t cfg_magnets_count = {
    .ptr = &storage.magnets_count,
    ._min = 2, ._max = 1000, ._default = 30
};

struct u16_config_entry_t cfg_keys_volume = {
    .ptr = &storage.keys_volume,
    ._min = 0, ._max = 100, ._default = 16
};

struct u16_config_entry_t cfg_signals_volume = {
    .ptr = &storage.signals_volume,
    ._min = 0, ._max = 100, ._default = 255
};

static inline void validate_cfg_float(struct float_config_entry_t *entry) {
  *entry->ptr = clamp_float(*entry->ptr, entry->_min, entry->_max);
}

static inline void validate_cfg_u16(struct u16_config_entry_t *entry) {
  *entry->ptr = clamp_float(*entry->ptr, entry->_min, entry->_max);
}

static inline void reset_cfg_float(struct float_config_entry_t *entry) {
  *entry->ptr = entry->_default;
}

static inline void reset_cfg_u16(struct u16_config_entry_t *entry) {
  *entry->ptr = entry->_default;
}

static void eeprom_write(uint8_t addr, const void *ptr, uint8_t count) {
  // Mostly copy-pasted from i2c_write7_v1,
  // but data now leading with address byte
  while ((I2C_SR2(EEPROM_I2C_BUS) & I2C_SR2_BUSY)) {
  }

  i2c_send_start(EEPROM_I2C_BUS);

  // Wait for the end of the start condition, master mode selected, and BUSY
  // bit set
  while (!((I2C_SR1(EEPROM_I2C_BUS) & I2C_SR1_SB) &&
           (I2C_SR2(EEPROM_I2C_BUS) & I2C_SR2_MSL) &&
           (I2C_SR2(EEPROM_I2C_BUS) & I2C_SR2_BUSY))) {
  }

  i2c_send_7bit_address(EEPROM_I2C_BUS, EEPROM_I2C_ADDRESS, I2C_WRITE);

  // Waiting for address is transferred.
  while (!(I2C_SR1(EEPROM_I2C_BUS) & I2C_SR1_ADDR)) {
  }

  // Clearing ADDR condition sequence.
  (void)I2C_SR2(EEPROM_I2C_BUS);

  i2c_send_data(EEPROM_I2C_BUS, addr);

  while (!(I2C_SR1(EEPROM_I2C_BUS) & (I2C_SR1_BTF))) {
  }

  const uint8_t *byte_ptr = ptr;
  for (uint8_t i = 0; i < count; i++) {
    i2c_send_data(EEPROM_I2C_BUS, *byte_ptr);
    while (!(I2C_SR1(EEPROM_I2C_BUS) & (I2C_SR1_BTF))) {
    }
    ++byte_ptr;
  }

  i2c_send_stop(EEPROM_I2C_BUS);
  msleep(15); // see maximum write time in datasheet
}

void eeprom_set_bytes(uint8_t addr, const void *ptr, uint8_t count) {
  const uint8_t *bytes = (uint8_t *)ptr;
  uint8_t page_offset = addr % EEPROM_PAGE_SIZE;
  uint8_t bytes_to_write;

  while (count > 0) {
    if (count < EEPROM_PAGE_SIZE - page_offset) {
      bytes_to_write = count;
    } else {
      bytes_to_write = EEPROM_PAGE_SIZE - page_offset;
    }
    eeprom_write(addr, bytes, bytes_to_write);

    page_offset = 0;
    addr += bytes_to_write;
    bytes += bytes_to_write;
    count -= bytes_to_write;
  }
}

void eeprom_set_byte(uint8_t addr, uint8_t data) {
  uint8_t tmp[] = {addr, data};
  i2c_transfer7(EEPROM_I2C_BUS, EEPROM_I2C_ADDRESS, tmp, 2, NULL, 0);
}

void eeprom_read_bytes(uint8_t addr, void *ptr, uint8_t count) {
  uint8_t mem_addr[1] = {addr};
  i2c_transfer7(EEPROM_I2C_BUS, EEPROM_I2C_ADDRESS, mem_addr, 1, (uint8_t *)ptr,
                count);
}

void eeprom_read_and_validate(void) {
  uint8_t eeprom_read_header[EEPROM_HEADER_SIZE];
  eeprom_read_bytes(0, eeprom_read_header, EEPROM_HEADER_SIZE);
  if(memcmp(eeprom_read_header, eeprom_valid_header, EEPROM_HEADER_SIZE) != 0) {
    eeprom_reset_all(true);
    beep(200, 1000, 255);
  }

  eeprom_read_bytes(EEPROM_DATA_START_ADDRESS, &storage, STORAGE_SIZE);

  if (storage.calibrate_triggers_on_startup){
    storage.brake_trigger_min_value = brake_adc_value() + TRIGGER_ANTI_JITTER;
    storage.throttle_trigger_min_value = throttle_adc_value() + TRIGGER_ANTI_JITTER;
  }


  validate_cfg_float(&cfg_current_limit);
  validate_cfg_float(&cfg_speed_stabilization_Kp);

  validate_cfg_u16(&cfg_brake_trigger_min_value);
  validate_cfg_u16(&cfg_throttle_trigger_min_value);
  validate_cfg_u16(&cfg_brake_trigger_max_value);
  validate_cfg_u16(&cfg_throttle_trigger_max_value);
  validate_cfg_u16(&cfg_wheel_length_mm);
  validate_cfg_u16(&cfg_magnets_count);
  validate_cfg_u16(&cfg_keys_volume);
  validate_cfg_u16(&cfg_signals_volume);

  storage.speed_limit_last = clamp_u8(storage.speed_limit_last, 5, 100);

  // this is bad
  if (storage.brake_trigger_max_value <= storage.brake_trigger_min_value) {
    storage.brake_trigger_max_value = storage.brake_trigger_min_value + 100;
  }

  if (storage.throttle_trigger_max_value <= storage.throttle_trigger_min_value) {
    storage.throttle_trigger_max_value = storage.throttle_trigger_min_value + 100;
  }

  if(!storage.persist_speed_limit) {
    storage.speed_limit_last = DEFAULT_SPEED_LIMIT_KMH;
  }

}

void eeprom_reset_all(uint8_t commit) {
  storage.contrast = 0xff;
  storage.soft_start_enabled = 0;
  storage.zero_start_enabled = 0;
  storage.current_limit_enabled = 0;
  storage.speed_stabilization_enabled = 0;
  storage.total_distance_meters = 0;
  storage.anti_throttle_lock = 1;
  storage.mute = 0;
  storage.calibrate_triggers_on_startup = 1;
  storage.speed_limit_last = DEFAULT_SPEED_LIMIT_KMH;
  storage.persist_speed_limit = 0;

  reset_cfg_u16(&cfg_brake_trigger_min_value);
  reset_cfg_u16(&cfg_throttle_trigger_min_value);
  reset_cfg_u16(&cfg_brake_trigger_max_value);
  reset_cfg_u16(&cfg_throttle_trigger_max_value);
  reset_cfg_u16(&cfg_wheel_length_mm);
  reset_cfg_u16(&cfg_magnets_count);
  reset_cfg_u16(&cfg_keys_volume);
  reset_cfg_u16(&cfg_signals_volume);

  reset_cfg_float(&cfg_speed_stabilization_Kp);
  reset_cfg_float(&cfg_current_limit);
  reset_cfg_float(&cfg_current_stabilization_Kp);
  reset_cfg_float(&cfg_soft_start_increment);

  memset(&storage.last_trips, 0U, sizeof(storage.last_trips));

  if (commit) {
    eeprom_write_all();
  }
}

void eeprom_write_all(void) {
  eeprom_set_bytes(0, eeprom_valid_header, EEPROM_HEADER_SIZE);
  eeprom_set_bytes(EEPROM_DATA_START_ADDRESS, &storage, STORAGE_SIZE);
}

void eeprom_write_entry(void *variable_addr, uint8_t variable_size) {
  void *storage_addr = &storage;

  // entry is outside struct
  if (variable_addr < storage_addr ||
      variable_addr > storage_addr + STORAGE_SIZE) {
    return;
  }

  uint8_t eeprom_addr =
      EEPROM_DATA_START_ADDRESS + (variable_addr - storage_addr);
  eeprom_set_bytes(eeprom_addr, variable_addr, variable_size);
}

void try_persist_distance(void) {
  if(distance_cm_accumulator > (PERSIST_DISTANCE_EVERY_METERS * 100)) {
    distance_cm_accumulator -= (PERSIST_DISTANCE_EVERY_METERS * 100);
    storage.total_distance_meters += PERSIST_DISTANCE_EVERY_METERS;
    eeprom_persist(&storage.total_distance_meters);
  }
}

void force_persist_distance(void) {
  storage.total_distance_meters += distance_cm_accumulator / 100;
  distance_cm_accumulator = 0;
  eeprom_persist(&storage.total_distance_meters);
}


void session_add_distance(uint16_t centimeters) {
  current_session_distance_cm += centimeters;
  distance_cm_accumulator += centimeters;
}

uint32_t session_distance_cm() {
  return current_session_distance_cm;
}

void persist_last_trip() {
  struct last_trip_info_t new_info = {
      .minutes = millis() / 1000 / 60,
      .meters = session_distance_cm() / 100,
  };
  struct last_trip_info_t *info = storage.last_trips;
  uint8_t free_index = 0;

  if(new_info.meters == 0) {
    return;
  }

  for (; free_index < 8; free_index++) {
    if(info->minutes == 0 && info->meters == 0){
      break;
    }
    info++;
  }

  if(free_index > 7) { // shift array and insert to last position
    info = storage.last_trips;
    for (uint8_t i = 0; i < 7; i++) {
      *info = *(info + 1);
      ++info;
    }
    storage.last_trips[7] = new_info;
  } else {
    storage.last_trips[free_index] = new_info;
  }
  eeprom_persist(&storage.last_trips);
}
