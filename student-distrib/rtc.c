#include "rtc.h"
#include "lib.h"
#include "i8259.h"
#include "lib.h"

/* Init I/O ports */
void enalbe_inter() {
    cli();
    unsigned char prev_status_b;
    outb(MC146818_REGISTER_STATUS_B, MC146818_ADDRESS_REG); /* select register B */
    prev_status_b = inb(MC146818_DATA_REG); /* get previous value of B */
    outb(MC146818_REGISTER_STATUS_B, MC146818_ADDRESS_REG); /* select register B */
    outb(prev_status_b | 0x40 , MC146818_DATA_REG); /* set PIE to enale periodic interrupt */
    enable_irq(RTC_IRQ);    /* enable IRQ8 */
    sti();
}

/* Set interrupt freq. */
void set_interrupt_rate() {
    cli();
    unsigned char prev_status_a;
    outb(MC146818_REGISTER_STATUS_A, MC146818_ADDRESS_REG); /* select register A */
    prev_status_a = inb(MC146818_DATA_REG); /* previous value of registers A */
    outb(MC146818_REGISTER_STATUS_A, MC146818_ADDRESS_REG); /* select register A */
    outb((prev_status_a & 0xF0) | INTERRUPT_RATE, MC146818_DATA_REG); /* write new interrupt rate : 2 Hz */
    sti();  
}

void rtc_init() {
    enalbe_inter(); /* enalbe periodic interrupt */ 
    set_interrupt_rate(); /* set interrupt freq. */
}

void rtc_handler() {
    cli();
    static char cnt = 'A';
    putc(cnt++);    /* output a char */
    if(cnt == 'Z') cnt = 'A';
    outb(MC146818_REGISTER_STATUS_C, MC146818_ADDRESS_REG); /* select register C */
    (void)inb(MC146818_DATA_REG); /* read registers C, this cleares (IRQ) signal */
    send_eoi(RTC_IRQ);  /* end-of-interrupt */
    sti();
}
