#ifndef TERMINAL_H
#define TERMINAL_H

#include "types.h"
#include "lib.h"
#include "keyboard.h"

#include "pcb.h"

#define MAX_TERMINAL_SIZE 128
#define TERMINAL_NUMBER 3

#define VIDEOMEM_SIZE (NUM_COLS * NUM_ROWS * 2)

#define VIDEOMEM_SIZE (NUM_COLS * NUM_ROWS * 2)

#define SIMKB 1

#define NON_BUFFER_READ 2

typedef struct terminal_t
{
    uint16_t cursor_x;
    uint16_t cursor_y;
    int32_t mouse_x, mouse_y;
    uint8_t count;
    volatile uint8_t simulateKeyboard;
    volatile uint8_t enter_pressed;
    volatile uint8_t tab_pressed;
    volatile uint8_t up_pressed;
    volatile uint8_t down_pressed;
    volatile unsigned char terminal_buf[MAX_TERMINAL_SIZE];
    uint8_t *video_mem_backup;
    
    volatile uint8_t last_key;
} terminal_t;

void initialize_terminal();
int32_t terminal_read(int32_t fd, void *buf, int32_t nbytes);
int32_t terminal_write(int32_t fd, void *buf, int32_t nbytes);
int32_t terminal_open(const uint8_t *filename);
int32_t terminal_close(int32_t fd);
int32_t terminal_ioctl(int32_t fd, int32_t request, void* buf);
void terminal_clear();

// @@Y
int32_t switch_terminal(int32_t terminal_num);
extern volatile int32_t current_terminal;

extern terminal_t main_terminal[TERMINAL_NUMBER];
extern terminal_t prev_terminal[TERMINAL_NUMBER];

#endif
