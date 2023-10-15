#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "types.h"
#include "i8259.h"
#include "lib.h"

#define KEYBOARD_DATA_PORT 0x60
#define STATUS_REGISTER 0x64
#define COMMAND REGISTER 0x64 
#define KEYBOARD_IRQ 1

void init_keyboard();
void keyboard_handler();

#endif