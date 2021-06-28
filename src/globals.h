#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdint.h>

#define TRIGGER_MAX 1000U
#define CONTROLLER_TRIGGER_MAX 1000U

#define PERSIST_DISTANCE_EVERY_METERS 100
#define DEFAULT_SPEED_LIMIT_KMH 15.0f

#define BATTERY_VOLTS_0_PERCENTS 310
#define BATTERY_VOLTS_100_PERCENTS 420
/// Calibration values saved +- this value
#define TRIGGER_ANTI_JITTER 30
/// GUI repain interval in milliseconds
#define DISPLAY_REFRESH_INTERVAL_MS 500

#define LOGIC_REFRESH_INTERVAL_MS 50
#define PACKET_SEND_INTERVAL_MS 50
#define ANTI_THROTTLE_LOCK_PERIOD_MS 1000
#define ANTI_THROTTLE_LOCK_ZERO_PACKETS 2

#define STATIC_BEEP_VOLUME 16
#define KEY_BEEP_FREQ 4000

// Show separators between menu categories
#define MENU_SEPARATORS

/// throttle value after which wheel starts to rotate
#define CONTROLLER_STOPPED_VAL 175

#define SCREENSAVER_INACTIVITY_PERIOD_MS (1000U * 60U)

/**
 * speed to throttle value ratio, approximately
 * used in kugoo_s3_set_speed_approx
 *
 * see speed.ods
 *
 * Example:
 * CONTROLLER_STOPPED_VAL + 10 km/H * 23.07046 = 175 + 230.7 = 405
 */
#define THROTTLE_TO_SPEED_COEFF 23.07046f

/// Used in anti throttle lock logic.
/// After throttle sets to 0, then it slowly rises to normal value.
#define THROTTLE_RECOVER_INCREMENT 3

// Print reset reason in detailed view instead of some indicators
// #define DEBUG_RESET_REASON


enum control_state_t {
  CS_NORMAL,
  CS_THROTTLE_RECOVER,
  CS_FORCE_ZERO_THROTTLE,
};

enum cruise_control_status_t {
  CRUISE_CTL_DISABLED,
  CRUISE_CTL_WAITING_RELEASE, ///< Waiting for throttle trigger value becomes 0
  CRUISE_CTL_ENABLED,
};

extern char sprintf_buf[64];
// extern float desired_speed_kmh;
extern float cruise_ctl_speed_kmh;
// extern float control_output_kmh;
extern uint32_t reset_reason;
extern enum cruise_control_status_t cruise_ctl_status;

#endif /* GLOBALS_H */
