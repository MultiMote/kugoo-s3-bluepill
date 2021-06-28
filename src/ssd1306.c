#include "ssd1306.h"
#include "font.h"
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/spi.h>
#include <string.h>

#define SSD1306_FRAMEBUFFER_SIZE (SSD1306_LINES * SSD1306_WIDTH)
static uint8_t framebuffer[SSD1306_FRAMEBUFFER_SIZE];
static uint8_t *framebuffer_pointer = framebuffer;
static const uint8_t *framebuffer_end = framebuffer + SSD1306_FRAMEBUFFER_SIZE;
static enum ssd1306_blend_mode_t blend_mode = SSD1306_BLEND_MODE_LIGHTEN;
static enum ssd1306_blend_mode_t blend_mode_prev = SSD1306_BLEND_MODE_LIGHTEN;
static uint8_t font_scale = 1;

// If the Co bit is set as logic “0”, the transmission of the following
// information will contain data bytes only.
#define SSD1306_CONTROL_BYTE_CONTINUATION_OFF (1 << 7)
// The D/C# bit determines the next data byte is acted as a command or a data.
// If the D/C# bit is set to logic “0”, it defines the following data byte as a
// command. If the D/C# bit is set to logic “1”, it defines the following data
// byte as a data which will be stored at the GDDRAM.
#define SSD1306_CONTROL_BYTE_DC (1 << 6)

static inline void wait_for_data_transfer_finished(void) {
  while (!(I2C_SR1(SSD1306_I2C_REGISTER) & (I2C_SR1_BTF))) {
  }
}

static inline void wait_for_address_transfer_finished(void) {
  while (!(I2C_SR1(SSD1306_I2C_REGISTER) & I2C_SR1_ADDR)) {
  }
}

void ssd1306_transfer_start(void) {
  i2c_send_start(SSD1306_I2C_REGISTER);

  // wait for i2c ready
  while (!((I2C_SR1(SSD1306_I2C_REGISTER) & I2C_SR1_SB) &
           (I2C_SR2(SSD1306_I2C_REGISTER) & (I2C_SR2_MSL | I2C_SR2_BUSY)))) {
  }

  i2c_send_7bit_address(SSD1306_I2C_REGISTER, SSD1306_ADDRESS, I2C_WRITE);
  wait_for_address_transfer_finished();
  (void)I2C_SR2(SSD1306_I2C_REGISTER);
}

void ssd1306_transfer_end(void) { i2c_send_stop(SSD1306_I2C_REGISTER); }

void ssd1306_byte(uint8_t b) {
  i2c_send_data(SSD1306_I2C_REGISTER, b);
  wait_for_data_transfer_finished();
}

void ssd1306_cmd(uint8_t cmd) {
  ssd1306_byte(0x00 | SSD1306_CONTROL_BYTE_CONTINUATION_OFF); // control byte
  ssd1306_byte(cmd);
}

void ssd1306_cmd_with_value(uint8_t cmd, uint8_t value) {
  ssd1306_cmd(cmd);
  ssd1306_cmd(value);
}

void ssd1306_init() {
  ssd1306_transfer_start();

  ssd1306_cmd(0xAE); // Set display OFF
  ssd1306_cmd(0xD5); // Set display clock divide ratio or oscillator frequency
  ssd1306_cmd(0x80); // Display Clock Divide Ratio / OSC Frequency

  ssd1306_cmd_with_value(0xA8, 0x3F); // Multiplex Ratio for 128x64 (64-1)

  ssd1306_cmd_with_value(0xD3, 0x00); // Display Offset

  ssd1306_cmd(0x40); // Set Display Start Line

  ssd1306_cmd_with_value(
      0x8D, 0x14); // Charge Pump (0x10 External, 0x14 Internal DC/DC)

  // ssd1306_cmd(0x20); // Set Memory Addressing Mode
  // ssd1306_cmd(0x00); // Horizontal addressing mode

  ssd1306_cmd(0xA1); // Set Segment Re-Map
  ssd1306_cmd(0xC8); // Set Com Output Scan Direction
  ssd1306_cmd(0xDA); // Set COM Hardware Configuration
  ssd1306_cmd(0x12); // COM Hardware Configuration
  ssd1306_cmd_with_value(0x81, 0x7f); // contrast control
  ssd1306_cmd_with_value(0xD9, 0xF1); // Set Pre-Charge Period (0x22 External, 0xF1 Internal)
  ssd1306_cmd_with_value(0xDB, 0x40); // VCOMH Deselect Level
  ssd1306_cmd(0xA4);                  // Disable Entire Display On
  ssd1306_cmd(0xA6); // Normal display, 0xA7 - inverse
  ssd1306_transfer_end();

  ssd1306_clear();
  ssd1306_redraw();

  ssd1306_transfer_start();
  ssd1306_cmd(0xAF); // Set display On
  ssd1306_transfer_end();
}

void ssd1306_contrast(uint8_t val) {
  ssd1306_transfer_start();
  ssd1306_cmd_with_value(0x81, val); // contrast control
  ssd1306_transfer_end();
}

void ssd1306_clear(void) { ssd1306_fill(0x00); }

void ssd1306_fill(uint8_t pattern) {
  memset(framebuffer, pattern, SSD1306_FRAMEBUFFER_SIZE);
}

void ssd1306_redraw(void) {
  for (uint8_t line = 0; line < SSD1306_LINES; line++) {
    ssd1306_transfer_start();
    ssd1306_cmd(0xB0 + line); // Set page start address for page addressing mode
    ssd1306_cmd(0x10); // Set higher column start address for page addressing mode
    ssd1306_cmd(0x00); // Set lower column start address for page addressing mode

    ssd1306_byte(SSD1306_CONTROL_BYTE_DC); // Control byte

    uint8_t *framebuffer_pos = framebuffer + (line * SSD1306_WIDTH);
    for (uint8_t col = 0; col < SSD1306_WIDTH; col++) {
      ssd1306_byte(*(framebuffer_pos++));
    }

    ssd1306_transfer_end();
  }
}

void ssd1306_blend_mode(enum ssd1306_blend_mode_t mode) { blend_mode = mode; }

void ssd1306_blend_mode_push() { blend_mode_prev = blend_mode; }

void ssd1306_blend_mode_pop() { blend_mode = blend_mode_prev; }

void ssd1306_framebuffer_setpos(uint8_t line, uint8_t column) {
  uint8_t *new_pos = framebuffer + (line * SSD1306_WIDTH) + column;
  if (new_pos >= framebuffer && new_pos < framebuffer_end) {
    framebuffer_pointer = new_pos;
  }
}

void ssd1306_framebuffer_byte(uint8_t data) {
  if (framebuffer_pointer >= framebuffer &&
      framebuffer_pointer < framebuffer_end) { // is pointer within framebuffer
    switch (blend_mode) {
    case SSD1306_BLEND_MODE_REPLACE:
      *framebuffer_pointer = data;
      break;
    case SSD1306_BLEND_MODE_LIGHTEN:
      *framebuffer_pointer |= data;
      break;
    case SSD1306_BLEND_MODE_EXCLUSION:
      *framebuffer_pointer &= ~data;
      break;
    case SSD1306_BLEND_MODE_XOR:
      *framebuffer_pointer ^= data;
      break;
    }

    ++framebuffer_pointer;
  }
}

void ssd1306_framebuffer_byte_fixedline(uint8_t data) {
  uint8_t current_line = (framebuffer_pointer - framebuffer) / SSD1306_WIDTH;
  uint8_t next_line = (framebuffer_pointer + 1U - framebuffer) / SSD1306_WIDTH;

  if (current_line != next_line) {
    return;
  }

  ssd1306_framebuffer_byte(data);
}

void ssd1306_framebuffer_nextline(uint8_t start_from_zero) {
  uint8_t line = (framebuffer_pointer - framebuffer) / SSD1306_WIDTH;
  if (start_from_zero) {
    ssd1306_framebuffer_setpos(line + 1, 0);
  } else if (framebuffer_pointer + SSD1306_WIDTH < framebuffer_end) {
    framebuffer_pointer += SSD1306_WIDTH;
  }
}

static void draw_font_bytes(const uint8_t *bytes) {
  uint8_t column = (framebuffer_pointer - framebuffer) % SSD1306_WIDTH;
  uint8_t *starting_framebuffer_pos = framebuffer_pointer;

  // todo: character wrap

  if (font_scale == 1) { // optimization :D
    for (uint8_t col = 0; col < FONT_WIDTH; col++) {
      ssd1306_framebuffer_byte(*bytes);
      ++bytes;
    }
  } else {
    const uint8_t *ptr;
    for (uint8_t pass = 0; pass < font_scale; pass++) {
      ptr = bytes;
      for (uint8_t col = 0; col < FONT_WIDTH; col++) {
        uint8_t b = 0;
        uint8_t source_pos = 0;

        // multiply column pixel heights by blocks
        for (uint8_t i = 0; i < 8; i++) {
          // What I have made?
          source_pos = (pass * (8 / font_scale) + i / font_scale);
          b |= !!(*ptr & (1 << source_pos)) << i;
        }

        // multiply column width
        for (uint8_t i = 0; i < font_scale; i++) {
          ssd1306_framebuffer_byte(b);
        }

        ++ptr;
      }
      ssd1306_framebuffer_nextline(true);
      framebuffer_pointer += column;
    }
  }
  framebuffer_pointer =
      starting_framebuffer_pos + (FONT_WIDTH + 1) * font_scale;
}

void ssd1306_font_scale(uint8_t scale) { font_scale = scale; }


void ssd1306_char(char ch) {
  if (ch == '\n') {
    ssd1306_framebuffer_nextline(true);
  } else {
    ssd1306_mbchar(0x000000ff & ch);
  }
}

/// auto-generated by font_convert.py
void ssd1306_mbchar(uint32_t ch) {
  uint16_t mapped_idx;

  if (ch >= 0x00000020 && ch <= 0x0000005f) {
    mapped_idx = 0 + (ch - 0x00000020);
  } else if (ch == 0x0000d081) {
    mapped_idx = 64;
  } else if (ch >= 0x0000d090 && ch <= 0x0000d0af) {
    mapped_idx = 65 + (ch - 0x0000d090);
  } else if (ch == 0x00e296aa) {
    mapped_idx = 97;
  } else if (ch == 0x00e29c94) {
    mapped_idx = 98;
  } else {
    draw_font_bytes(font_error_symbol);
    return;
  }

  draw_font_bytes(font_data + mapped_idx * FONT_WIDTH);
}


void ssd1306_string(const char *str) {
  uint8_t byte_count;
  uint32_t char_code;

  while (*str != '\0') {
    // determine size of sequence
    if ((*str & 0x80) == 0x00) {
      byte_count = 1;
    } else if ((*str & 0xE0) == 0xC0) {
      byte_count = 2;
    } else if ((*str & 0xF0) == 0xE0) {
      byte_count = 3;
    } else if ((*str & 0xF8) == 0xF0) {
      byte_count = 4;
    } else {
      ssd1306_mbchar(0); // error
      break;
    }

    char_code = 0;
    // read multibyte sequence to the 32-bit int
    for (uint8_t i = 0; i < byte_count; i++) {
      if (*str == '\0') {
        break;
      }
      char_code |= (0x000000FF & *str) << ((byte_count - 1 - i) * 8);
      str++;
    }

    if(byte_count == 1 && char_code == '\n') {
      ssd1306_framebuffer_nextline(true);
      continue;
    }

    ssd1306_mbchar(char_code);
  }
}

void ssd1306_horizontal_line(uint8_t x1, uint8_t x2, uint8_t y) {
  ssd1306_framebuffer_setpos(y / SSD1306_LINES, x1);
  uint8_t bits = (1 << (y % 8));
  for (uint16_t x = x1; x < x2 + 1; ++x) {
    ssd1306_framebuffer_byte(bits);
  }
}

void ssd1306_vertical_line(uint8_t y1, uint8_t y2, uint8_t x) {
  uint8_t pixels_left = y2 - y1;
  uint8_t line = y1 / SSD1306_LINES;
  uint8_t tmp_y = y1 % 8;
  uint8_t bits;
  uint8_t line_pixels;

  if (y2 < y1) {
    return;
  }

  while (pixels_left > 0) {
    line_pixels = (pixels_left < 8 ? pixels_left : 8) - tmp_y;
    bits = 0xff >> tmp_y;
    bits &= 0xff << (8 - line_pixels - tmp_y);
    pixels_left -= line_pixels;
    tmp_y = 0;
    ssd1306_framebuffer_setpos(line++, x);
    ssd1306_framebuffer_byte(bits);
  }
}

void ssd1306_pixel(uint8_t x, uint8_t y) {
  ssd1306_framebuffer_setpos(y / SSD1306_LINES, x);
  ssd1306_framebuffer_byte((1 << (y % 8)));
}
