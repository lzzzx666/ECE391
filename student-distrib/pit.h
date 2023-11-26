#ifndef PIT_H
#define PIT_H
#ifndef NOPIT
#include "types.h"
/*PIT Port*/

#define PIT_8254_CHANNEL_0 0x40
#define PIT_8254_CHANNEL_1 0x41
#define PIT_8254_CHANNEL_2 0x42
#define MODE_COMMAND_RESIGTER 0x43
#define CHANNEL_0 0
#define CHANNEL_1 1
#define CHANNEL_2 2
#define PIT_IRQ 0

/*file variable*/
uint8_t mode_register;

void set_mode(int32_t select_channel, int32_t access_mode, int32_t operating_mode,
              int32_t binary_mode);
void pit_init();

extern void pit_handler();

#endif
#endif
