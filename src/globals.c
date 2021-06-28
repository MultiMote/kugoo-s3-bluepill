#include "globals.h"
#include <string.h>

char sprintf_buf[64] = { '\0' };
// float desired_speed_kmh = 0;
float cruise_ctl_speed_kmh = 0;
// float control_output_kmh = 0;
uint32_t reset_reason = 0;

enum cruise_control_status_t cruise_ctl_status = CRUISE_CTL_DISABLED;
