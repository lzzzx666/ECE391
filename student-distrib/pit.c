#include "pit.h"
#include "systemcall.h"
/*file variable*/
uint8_t mode_register;
uint32_t pit_cnt = 0;

void pit_init()
{
    set_mode(CHANNEL_0, LOW_HIGH_BYTE, SQUARE_WAVE_GENERATOR, 0);
    set_frequency(FREQ);
    enable_irq(PIT_IRQ);
}

void set_mode(int32_t select_channel, int32_t access_mode, int32_t operating_mode,
              int32_t binary_mode)
{
    int8_t mode_setting = binary_mode | (operating_mode << 1) | (access_mode << 4) | (select_channel << 6);
    outb(mode_setting, MODE_COMMAND_RESIGTER);
}
void set_frequency(int16_t frequency)
{
    cli();
    outb((int8_t)frequency && 0x00ff, PIT_8254_CHANNEL_0);
    outb((int8_t)(frequency && 0xff00 >> 8), PIT_8254_CHANNEL_0);
    sti();
}
void pit_handler() {
    // cli();
    pit_cnt++;
 
    send_eoi(PIT_IRQ);
    //    putc('s');
    // sti();
    // if(pit_cnt<=4)
    // execute("shell");
    // putc("s");
    // printf("%d\n",pit_cnt);
    schedule();
    // sti();
    


}
