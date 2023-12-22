#include "pit.h"
#include "systemcall.h"
#include "terminal.h"

/*file variable*/
uint8_t mode_register;
uint32_t pit_cnt = 0; // pit counter

volatile uint8_t user_exit[TERMINAL_NUMBER] = {0};
// Description: initialize the pit
// Parameters:None.
// Return Value:None.
// Side Effects: it sets the mode to channel 0 and initial frequency, then turn
//               on the interrupt
void pit_init()
{
    set_mode(CHANNEL_0, LOW_HIGH_BYTE, SQUARE_WAVE_GENERATOR, 0);
    set_frequency(FREQ);
    enable_irq(PIT_IRQ);
}
// Description: set the mode of the pit
// Parameters:select_channel: the channel(0,1 or 2)
//            access_mode: like low byte/high byte/...
//            operating_mode: like square wave generator
//            binary_mode: it only has 0 or 1
// Return Value:None.
// Side Effects: it sets the pit into the specified mode
void set_mode(int32_t select_channel, int32_t access_mode, int32_t operating_mode,
              int32_t binary_mode)
{
    int8_t mode_setting = binary_mode | (operating_mode << 1) | (access_mode << 4) | (select_channel << 6); // combine the parameters in to one byte
    outb(mode_setting, MODE_COMMAND_RESIGTER);
}
// Description: set frequency for the pit
// Parameters:frequency: the specified frequency for the pit
// Return Value:None.
// Side Effects: it changes the frequency of the pit
void set_frequency(int16_t frequency)
{
    cli();
    outb((int8_t)frequency && 0x00ff, PIT_8254_CHANNEL_0);        // low 8 bits
    outb((int8_t)(frequency && 0xff00 >> 8), PIT_8254_CHANNEL_0); // high 8 bits
    sti();
}
// Description: the handler triggered by pit
// Parameters:None.
// Return Value:None.
// Side Effects: call the schedule function
void pit_handler()
{
    pit_cnt++;

    send_eoi(PIT_IRQ);
    // observe if the program halt by ctrl+c
    if (user_exit[sche_index])
    {
        user_exit[sche_index] = 0;
        pcb_t *cur_pcb = get_current_pcb();
        if (cur_pcb->pid != 0 && cur_pcb->pid >= TERMINAL_NUMBER)
        {
            halt(1);
        }
    }
    user_exit[sche_index] = 0;
    // call the schedule function
    schedule();
}
