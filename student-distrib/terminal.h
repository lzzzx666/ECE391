#ifndef TERMINAL_H
#define TERMINAL_H

#include "types.h"
#include "lib.h"
#include "keyboard.h"

#define MAX_TERMINAL_SIZE 128
typedef struct terminal_t
{
    uint16_t cursor_x;
    uint16_t cursor_y;
    uint8_t count;
    volatile uint8_t enter_pressed;
    volatile unsigned char terminal_buf[MAX_TERMINAL_SIZE];
}terminal_t;

void initialize_terminal();
int32_t terminal_read(int32_t fd, void *buf, int32_t nbytes);
int32_t terminal_write(int32_t fd, void *buf, int32_t nbytes);
int32_t terminal_open(const char *filename);
int32_t terminal_close(int32_t fd);
void terminal_clear();


#endif

