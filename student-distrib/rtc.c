#include "rtc.h"
#include "lib.h"
#include "i8259.h"

volatile unsigned char TEST_RTC = 0;
char TEST_RTC_CNT = 0;

/**
 * enalbe_inter
 * Enalbe periodic interrupt of RTC
 * INPUT: none
 * OUTPUT: set register B of RTC
 */
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

/**
 * set_interrupt_rate
 * Set interrupt freq.
 * INPUT: none
 * OUTPUT: set register A of RTC
 */
void set_interrupt_rate() {
    cli();
    unsigned char prev_status_a;
    outb(MC146818_REGISTER_STATUS_A, MC146818_ADDRESS_REG); /* select register A */
    prev_status_a = inb(MC146818_DATA_REG); /* previous value of registers A */
    outb(MC146818_REGISTER_STATUS_A, MC146818_ADDRESS_REG); /* select register A */
    outb((prev_status_a & 0xF0) | INTERRUPT_RATE, MC146818_DATA_REG); /* write new interrupt rate : 2 Hz */
    sti();  
}


/**
 * rtc_init
 * initialize RTC
 * INPUT: none1
 * OUTPUT: set register A, B of RTC
 */
void rtc_init() {
    enalbe_inter(); /* enalbe periodic interrupt */ 
    set_interrupt_rate(); /* set interrupt freq. */
}

/**
 * handler of RTC periodic interrupt
 * INPUT: none
 * OUTPUT: set register C of RTC
 */
void rtc_handler() {
    cli();
    if(TEST_RTC) {
        test_interrupts();
        TEST_RTC_CNT++;
    }
    /* output a char */
#ifdef TEST_PRINT_PERIODIC
    static char cnt = 'A';
    putc(cnt++);    
    if(cnt > 'Z') cnt = 'A';
#endif
    outb(MC146818_REGISTER_STATUS_C, MC146818_ADDRESS_REG); /* select register C */
    (void)inb(MC146818_DATA_REG); /* read registers C, this cleares (IRQ) signal */
    send_eoi(RTC_IRQ);  /* end-of-interrupt */
    sti();
}

/**
 * rtc_test_event
 * Switch whether rtc test (increment video memory) is on
 * INPUT: none
 * OUTPUT: restore video memory of test if disabled
 */
void rtc_test_event() {
    cli();
    TEST_RTC ^= 1;
    if(!TEST_RTC) {
        while(TEST_RTC_CNT){
            test_interrupts();
            TEST_RTC_CNT++;
        }
    }
    sti();
}