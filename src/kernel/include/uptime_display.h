#ifndef UPTIME_DISPLAY_H
#define UPTIME_DISPLAY_H

void draw_system_uptime(void);
void draw_current_system_uptime(void);

void reverse(char* string, int length);
void utoa(uint32_t num, char* string);
void pad_zero(char* string, int min_width);


#endif
