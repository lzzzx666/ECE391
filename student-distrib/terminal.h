#ifndef TERMINAL_H
#define TERMINAL_H

#include "types.h"
#include "lib.h"
#include "keyboard.h"

#define MAX_TERMINAL_SIZE 128
#define TERMINAL_NUM 3

#define VIDEOMEM_SIZE (NUM_COLS * NUM_ROWS * 2)

typedef struct terminal_t
{
    uint16_t cursor_x;
    uint16_t cursor_y;
    uint8_t count;
    volatile uint8_t simulateKeyboard;
    volatile uint8_t enter_pressed;
    volatile uint8_t tab_pressed;
    volatile uint8_t up_pressed;
    volatile uint8_t down_pressed;
    volatile unsigned char terminal_buf[MAX_TERMINAL_SIZE];
    uint8_t *video_mem_backup;
} terminal_t;

void initialize_terminal();
int32_t terminal_read(int32_t fd, void *buf, int32_t nbytes);
int32_t terminal_write(int32_t fd, void *buf, int32_t nbytes);
int32_t terminal_open(const uint8_t *filename);
int32_t terminal_close(int32_t fd);
void terminal_clear();

// @@Y
int32_t switch_terminal(int32_t terminal_num);
extern int32_t current_terminal;

extern terminal_t main_terminal[TERMINAL_NUM];
extern terminal_t prev_terminal[TERMINAL_NUM];

#endif
