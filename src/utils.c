#include "utils.h"

uint8_t clamp_u8(uint8_t val, uint8_t min_val, uint8_t max_val) {
  if (val < min_val)
    return min_val;
  if (val > max_val)
    return max_val;
  return val;
}

inline uint16_t clamp_u16(uint16_t val, uint16_t min_val, uint16_t max_val) {
  if (val < min_val)
    return min_val;
  if (val > max_val)
    return max_val;
  return val;
}

float clamp_float(float val, float min_val, float max_val) {
  if (val < min_val)
    return min_val;
  if (val > max_val)
    return max_val;
  return val;
}

float convert_range_float(float val, float old_max, float new_max) {
  if (val < 0) {
    return 0;
  } else if (val > old_max) {
    return new_max;
  }
  return (val * new_max) / old_max;
}

uint16_t convert_range_u16(uint16_t val, uint16_t old_max, uint16_t new_max) {
  if (val > old_max) {
    return new_max;
  }
  return (val * new_max) / old_max;
}

inline void incr_u8_loop(uint8_t *val, uint8_t min_val, uint8_t max_val,
                         uint8_t step) {
  if ((*val + step) <= max_val) {
    *val += step;
  } else {
    *val = min_val;
  }
}

inline void decr_u8_loop(uint8_t *val, uint8_t min_val, uint8_t max_val,
                         uint8_t step) {
  if ((*val - step) >= min_val) {
    *val -= step;
  } else {
    *val = max_val;
  }
}

void incr_u16_loop(uint16_t *val, uint16_t min_val, uint16_t max_val,
                   uint16_t step) {
  if ((*val + step) <= max_val) {
    *val += step;
  } else {
    *val = min_val;
  }
}

void decr_u16_loop(uint16_t *val, uint16_t min_val, uint16_t max_val,
                   uint16_t step) {
  if ((*val - step) >= min_val) {
    *val -= step;
  } else {
    *val = max_val;
  }
}

void incr_float_loop(float *val, float min_val, float max_val, float step) {
  if ((*val + step) <= max_val) {
    *val += step;
  } else {
    *val = min_val;
  }
}

void decr_float_loop(float *val, float min_val, float max_val, float step) {
  if ((*val - step) >= min_val) {
    *val -= step;
  } else {
    *val = max_val;
  }
}

uint8_t digit_count(uint16_t val) {
  uint8_t count = 0;
  while (val > 0) {
    val /= 10;
    ++count;
  }
  return count;
}

uint16_t pow_u16(uint16_t base, uint16_t exponent) {
  uint16_t result = 1;
  for (; exponent > 0; exponent--) {
    result = result * base;
  }
  return result;
}

float pid_update(struct pid_t *pid, float error, float dt) {
  pid->P = error;
  pid->I += error * dt;
  pid->D = (error - pid->last_error) / dt;
  pid->last_error = error;
  return pid->P * pid->Kp + pid->I * pid->Ki + pid->D * pid->Kd;
}

void pid_reset(struct pid_t *pid) {
  pid->P = 0;
  pid->I = 0;
  pid->D = 0;
  pid->last_error = 0;
}
