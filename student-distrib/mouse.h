#ifndef MOUSE_H
#define MOUSE_H
#include "types.h"
#include "i8259.h"

#define MOUSE_IRQ    12
#define MOUSE_PORT   0x60
#define COMMAND_PORT 0x64
#define STATUS_PORT  0x64
#define MOUSE_ABIT   0x02
#define MOUSE_BBIT   0x01
#define MOUSE_WRITE  0xD4
#define MOUSE_F_BIT  0x20
#define MOUSE_V_BIT  0x08

typedef union mouse_packet_t {
    uint8_t val;
    struct {
        uint8_t btn_left    : 1;
        uint8_t btn_right   : 1;
        uint8_t btn_middle  : 1;
        uint8_t always1     : 1;
        uint8_t x_sign      : 1;
        uint8_t y_sign      : 1;
        uint8_t x_overflow  : 1;
        uint8_t y_overflow  : 1;
    } __attribute__ ((packed));
} mouse_packet_t;

void mouse_wait(uint8_t type);
void mouse_init(void);
int32_t mouse_read(void);
int32_t mouse_write(unsigned char data);
int32_t mouse_open(void);
int32_t mouse_close(void);
extern void mouse_handler(void);


#endif

