#include "detailed_view.h"
#include "globals.h"
#include "gui.h"
#include "persistence.h"
#include "hardware.h"
#include "keyboard.h"
#include "kugoo_s3.h"
#include "ssd1306.h"
#include "utils.h"
#include <stdio.h>

void detailed_view_redraw() {
  uint16_t current = kugoo_s3_rx.current;
  uint16_t batt = battery_value();
  uint32_t distance_m = session_distance_cm() / 100;

#ifdef DEBUG_RESET_REASON
  ssd1306_framebuffer_setpos(0, 0);
  sprintf(sprintf_buf, "0X%lX", reset_reason);
  ssd1306_string(sprintf_buf);

  ssd1306_framebuffer_setpos(1, 0);

  if(reset_reason & RCC_CSR_LPWRRSTF) {
    ssd1306_string("LOW-POWER\n");
  }
  if(reset_reason & RCC_CSR_WWDGRSTF) {
    ssd1306_string("WINDOW WATCHDOG\n");
  }
  if(reset_reason & RCC_CSR_IWDGRSTF) {
    ssd1306_string("INDEPENDENT WATCHDOG\n");
  }
  if(reset_reason & RCC_CSR_SFTRSTF) {
    ssd1306_string("SOFTWARE\n");
  }
  if(reset_reason & RCC_CSR_PORRSTF) {
    ssd1306_string("POWERON/POWERDOWN\n");
  }
  if(reset_reason & RCC_CSR_PINRSTF) {
    ssd1306_string("RESET BTN\n");
  }
#else
  ssd1306_font_scale(2);
  ssd1306_framebuffer_setpos(0, 0);
  sprintf(sprintf_buf, "%d.%d", batt / 10, batt % 10);
  ssd1306_string(sprintf_buf);
  ssd1306_font_scale(1);
  ssd1306_framebuffer_nextline(false);
  ssd1306_string(" В");

  ssd1306_font_scale(2);
  ssd1306_framebuffer_setpos(2, 0);
  sprintf(sprintf_buf, "%d.%d", current / 10, current % 10);
  ssd1306_string(sprintf_buf);
  ssd1306_font_scale(1);
  ssd1306_framebuffer_nextline(false);
  ssd1306_string(" А");

  ssd1306_font_scale(2);
  ssd1306_framebuffer_setpos(4, 0);
  sprintf(sprintf_buf, "%lu", distance_m);
  ssd1306_string(sprintf_buf);

  ssd1306_font_scale(1);
  ssd1306_framebuffer_nextline(false);
  ssd1306_string(" М");
#endif

  ssd1306_framebuffer_setpos(6, 0);
  ssd1306_font_scale(2);
  sprintf(sprintf_buf, "%lu.%lu", storage.total_distance_meters / 1000,
          (storage.total_distance_meters / 100) % 10);

  ssd1306_string(sprintf_buf);
  ssd1306_font_scale(1);
  ssd1306_framebuffer_nextline(false);
  ssd1306_string(" КМ");
}