#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

struct pid_t {
  float Kp;
  float Ki;
  float Kd;

  float P;
  float I;
  float D;
  float last_error;
};

struct pid_coeff_t {
  float Kp;
  float Ki;
  float Kd;
};

float pid_update(struct pid_t *pid, float error, float dt);
void pid_reset(struct pid_t *pid);

uint8_t clamp_u8(uint8_t val, uint8_t min_val, uint8_t max_val);
uint16_t clamp_u16(uint16_t val, uint16_t min_val, uint16_t max_val);
float clamp_float(float val, float min_val, float max_val);
float convert_range_float(float val, float old_max, float new_max);
uint16_t convert_range_u16(uint16_t val, uint16_t old_max, uint16_t new_max);
void incr_u8_loop(uint8_t *val, uint8_t min_val, uint8_t max_val, uint8_t step);
void decr_u8_loop(uint8_t *val, uint8_t min_val, uint8_t max_val, uint8_t step);
void incr_u16_loop(uint16_t *val, uint16_t min_val, uint16_t max_val, uint16_t step);
void decr_u16_loop(uint16_t *val, uint16_t min_val, uint16_t max_val, uint16_t step);
void incr_float_loop(float *val, float min_val, float max_val, float step);
void decr_float_loop(float *val, float min_val, float max_val, float step);
uint8_t digit_count(uint16_t val);
uint16_t pow_u16(uint16_t base, uint16_t exponent);


#endif /* UTILS_H */
