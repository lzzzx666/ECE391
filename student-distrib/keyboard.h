#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "types.h"
#include "i8259.h"
#include "lib.h"

#define KEY_BOARD_BUF_SIZE 128
#define READ_MAX_SIZE 127
#define KEYBOARD_DATA_PORT 0x60
#define REGISTER_PORT 0x64
#define KEYBOARD_IRQ 1
#define NUM_SCANCODES 60
#define BACKSPACE   0x0E
#define TAB 0x0F
#define ENTER 0x1C
#define CAPS_LOCK  0x3A
#define LSHIFT_MAKE  0x2A
#define LSHIFT_BREAK 0xAA
#define RSHIFT_MAKE	 0x36
#define RSHIFT_BREAK 0xB6
#define CTRL_MAKE 0x1D
#define CTRL_BREAK 0x9D
#define ALT_MAKE 0x38
#define ALT_BREAK  0xB8
#define UP 0x48
#define UPASCII 0xFE
#define DOWN 0x50
#define DOWNASCII 0xFF
#define F1  0x3B
#define F2  0x3C
#define F3  0x3D

#define TERMINAL_HK1 alt_pressed
#define TERMINAL1_HK2 F1
#define TERMINAL2_HK2 F2
#define TERMINAL3_HK2 F3

void init_keyboard();
extern void keyboard_handler();

#endif
