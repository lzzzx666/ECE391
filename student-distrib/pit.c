#include "pit.h"

#ifndef NOPIT
void pit_init() {
    set_mode(CHANNEL_0, );
    enable_irq(PIT_IRQ);
}

void set_mode(int32_t select_channel, int32_t access_mode, int32_t operating_mode,
              int32_t binary_mode) {}
void pit_handler() {}
#endif