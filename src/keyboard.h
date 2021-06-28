#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

#define KEY_REPEAT_DELAY 750
#define KEY_REPEAT_INTERVAL 100

#define KB_EVT_MASK_KEYS            0x1f

#define KB_EVT_NONE                 0x00

#define KB_EVT_KEY_BEEP             0x01
#define KB_EVT_KEY_SET              0x02
#define KB_EVT_KEY_POWER            0x04
#define KB_EVT_KEY_LIGHT            0x08
#define KB_EVT_KEY_SPEED            0x10

#define KB_EVT_TYPE_KEYDOWN         0x20
#define KB_EVT_TYPE_REPEAT          0x40
#define KB_EVT_TYPE_KEYUP           0x80

void keyboard_poll(void);
uint8_t keyboard_pop_event(void);

#endif /* KEYBOARD_H */
