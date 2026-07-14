#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

// PIT I/O Ports
#define PIT_PORT_CHANNEL_0 0x40
#define PIT_PORT_CHANNEL_1 0x41
#define PIT_PORT_CHANNEL_2 0x42
#define PIT_PORT_COMMAND   0x43

// Bits 7-6: Channel Selection
#define PIT_CMD_CHANNEL_0  0x00 // 00000000b
#define PIT_CMD_CHANNEL_1  0x40 // 01000000b
#define PIT_CMD_CHANNEL_2  0x80 // 10000000b
#define PIT_CMD_READBACK   0xC0 // 11000000b

// Bits 5-4: Access Mode
#define PIT_CMD_LATCH      0x00 // 00000000b - Latch count value
#define PIT_CMD_LOW_ONLY   0x10 // 00010000b - Access low byte only
#define PIT_CMD_HIGH_ONLY  0x20 // 00100000b - Access high byte only
#define PIT_CMD_BOTH       0x30 // 00110000b - Access low byte then high byte

// Bits 3-1: Operating Mode
#define PIT_CMD_MODE_0     0x00 // 00000000b - Interrupt on terminal count
#define PIT_CMD_MODE_1     0x02 // 00000010b - Hardware re-triggerable one-shot
#define PIT_CMD_MODE_2     0x04 // 00000100b - Rate generator (Periodic)
#define PIT_CMD_MODE_3     0x06 // 00000110b - Square wave generator (Standard)
#define PIT_CMD_MODE_4     0x08 // 00001000b - Software triggered strobe
#define PIT_CMD_MODE_5     0x0A // 00001010b - Hardware triggered strobe

// Bit 0: Counter Mode
#define PIT_CMD_BINARY     0x00 // 00000000b - 16-bit binary counter
#define PIT_CMD_BCD        0x01 // 00000001b - 4-digit BCD counter
                                //
#define PIT_BASE_FREQUENCY 1193182

typedef struct {
    uint32_t hours;
    uint32_t minutes;
    uint32_t seconds;
    uint32_t milliseconds;
}Uptime_t;

void initialize_timer(uint32_t frequency);
void timer_handler(void);
void stall_cpu(uint32_t ticks);

Uptime_t get_system_uptime(void);

#endif
