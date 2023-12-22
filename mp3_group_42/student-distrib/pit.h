#ifndef PIT_H
#define PIT_H
#ifndef NOPIT
#include "types.h"
#include "lib.h"
#include "i8259.h"
#include "terminal.h"
/*PIT Port*/

#define PIT_8254_CHANNEL_0 0x40
#define PIT_8254_CHANNEL_1 0x41
#define PIT_8254_CHANNEL_2 0x42
#define MODE_COMMAND_RESIGTER 0x43
#define CHANNEL_0 0
#define CHANNEL_1 1
#define CHANNEL_2 2
#define PIT_IRQ 0
#define LOW_HIGH_BYTE 3
#define SQUARE_WAVE_GENERATOR 3
#define FREQ 11932*2 //the actual frequency will be 1193182/FREQ


void set_mode(int32_t select_channel, int32_t access_mode, int32_t operating_mode,
              int32_t binary_mode);
void pit_init();
void set_frequency(int16_t frequency);

extern void pit_handler();
extern void schedule();

extern volatile uint8_t user_exit[TERMINAL_NUMBER];

#endif
#endif
