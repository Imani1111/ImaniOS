#ifndef KEYBOARD_H
#define KEYBOARD_H

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_BUFFER_SIZE 64

#define KEY_LSHIFT         0x2A
#define KEY_RSHIFT         0x36
#define KEY_BACKSPACE      0x0E
#define KEY_ENTER          0x1C
#define KEY_LEFT           0x4B
#define KEY_RIGHT          0x4D
#define KEY_UP             0x48
#define KEY_DOWN           0x50

extern int keyboard_focus;

void keyboard_handler(void);
char get_char(void);

#endif
