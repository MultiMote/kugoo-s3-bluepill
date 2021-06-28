#ifndef SSD1306_H
#define SSD1306_H

#include <stdint.h>

enum ssd1306_blend_mode_t {
    SSD1306_BLEND_MODE_REPLACE, // Replace background and pixels
    SSD1306_BLEND_MODE_LIGHTEN, // Keep background, switch pixels on
    SSD1306_BLEND_MODE_EXCLUSION, // Keep background, switch pixels off
    SSD1306_BLEND_MODE_XOR, // Keep background, toggle pixels
};

#define SSD1306_ADDRESS 0x3c
#define SSD1306_WIDTH 128
#define SSD1306_LINES 8 // 64 pixels = 8 lines of 8 pixels
#define SSD1306_I2C_REGISTER I2C1

void ssd1306_init(void);
void ssd1306_contrast(uint8_t val);
void ssd1306_clear(void);
void ssd1306_fill(uint8_t pattern);
void ssd1306_redraw(void);
void ssd1306_blend_mode(enum ssd1306_blend_mode_t mode);
void ssd1306_blend_mode_push();
void ssd1306_blend_mode_pop();

/// Origin is top-left corner
void ssd1306_framebuffer_setpos(uint8_t line, uint8_t column);
void ssd1306_framebuffer_byte(uint8_t data);
void ssd1306_framebuffer_byte_fixedline(uint8_t data);
void ssd1306_framebuffer_nextline(uint8_t start_from_zero);

// Only powers of 2 accepted (1, 2, 4, 8)
void ssd1306_font_scale(uint8_t scale);
void ssd1306_string(const char* str);
/// ASCII char
void ssd1306_char(char ch);
void ssd1306_mbchar(uint32_t ch);

void ssd1306_horizontal_line(uint8_t x1, uint8_t x2, uint8_t y);
void ssd1306_vertical_line(uint8_t y1, uint8_t y2, uint8_t x);

/// The most innefficient method of drawing
void ssd1306_pixel(uint8_t x, uint8_t y);

#endif /* SSD1306_H */
