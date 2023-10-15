#include "rtc.h"
#include "lib.h"
#include "i8259.h"
#include "lib.h"

/* MC146818 RTC registers */
#define MC146818_ADDRESS_REG   0x70
#define MC146818_DATA_REG      0x71

#define MC146818_REGISTER_STATUS_A      0x0A
#define MC146818_REGISTER_STATUS_B      0x0B
#define MC146818_REGISTER_STATUS_C      0x0C

/* Interrupt freq. */
#define INTERRUPT_RATE 0xF
#define INTERRUPT_FREQ (32768 >> (INTERRUPT_RATE - 1))

#define RTC_IRQ 8


/* Init I/O ports */
void init_io() {
    cli();
    unsigned char prev_status_b;
    outb(MC146818_REGISTER_STATUS_B, MC146818_ADDRESS_REG);
    prev_status_b = inb(MC146818_DATA_REG);
    outb(MC146818_REGISTER_STATUS_B, MC146818_ADDRESS_REG);
    outb(prev_status_b | 0x40 , MC146818_DATA_REG);
    enable_irq(RTC_IRQ);
    sti();
}

/* Set interrupt freq. */
void set_interrupt_rate() {
    cli();
    unsigned char prev_status_a;
    outb(MC146818_REGISTER_STATUS_A, MC146818_ADDRESS_REG); /* select register B */
    prev_status_a = inb(MC146818_DATA_REG); /* previous value of registers B */
    outb(MC146818_REGISTER_STATUS_A, MC146818_ADDRESS_REG); /* select register B */
    outb((prev_status_a & 0xF0) | INTERRUPT_RATE, MC146818_DATA_REG); /* set new interrupt rate */
    sti();  
}

void rtc_init() {
    init_io(); /* init */ 
    set_interrupt_rate(); /* set interrupt freq. */
}

void rtc_handler() {
    cli();
    static char cnt = 'A';
    putc(cnt++);
    if(cnt == 'Z') cnt = 'A';
    outb(MC146818_REGISTER_STATUS_C, MC146818_ADDRESS_REG); /* select register C */
    (void)inb(MC146818_DATA_REG); /* previous value of registers C */
    send_eoi(RTC_IRQ);
    sti();
}
