#include "rtc.h"
#include "lib.h"

/* MC146818 RTC registers */
#define MC146818_ADDRESS_REG   0x70
#define MC146818_DATA_REG      0x71

#define MC146818_REGISTER_STATUS_A      0x0A
#define MC146818_REGISTER_STATUS_B      0x0B

/* Interrupt freq. */
#define INTERRUPT_RATE 0xF
#define INTERRUPT_FREQ (32768 >> (INTERRUPT_RATE-1))


/* Init I/O ports */
void init_io() {
    cli();
    outw(1, MC146818_ADDRESS_REG);
    sti();
}

/* Set interrupt freq. */
void set_interrupt_rate() {
    cli();
    unsigned char prev_status_b;
    outb(MC146818_REGISTER_STATUS_A, MC146818_ADDRESS_REG); /* select register B */
    prev_status_b = inb(MC146818_DATA_REG); /* previous value of registers B */
    outb(MC146818_REGISTER_STATUS_A, MC146818_ADDRESS_REG); /* select register B */
    outb((prev_status_b & 0xF0) | INTERRUPT_RATE, MC146818_DATA_REG); /* set new interrupt rate */
    sti();  
}

void rtc_init() {
    init_io(); /* init */ 
    set_interrupt_rate(); /* set interrupt freq. */
}
