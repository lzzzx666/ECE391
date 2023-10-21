#include "rtc.h"
#include "lib.h"
#include "i8259.h"

volatile unsigned char test_rtc = 0;
char test_rtc_cnt = 0;

volatile unsigned char interrtupt_occured = 0;

/**
 * enalbe_inter
 * Enalbe periodic interrupt of RTC
 * INPUT: none
 * OUTPUT: set register B of RTC
 */
void enalbe_inter() {
    cli();
    unsigned char prev_status_b;
    outb(MC146818_REGISTER_STATUS_B | CMOS_NMI_DISABLE, MC146818_ADDRESS_REG); /* select register B, disalbe NMI */
    prev_status_b = inb(MC146818_DATA_REG); /* get previous value of B */
    outb(MC146818_REGISTER_STATUS_B | CMOS_NMI_DISABLE, MC146818_ADDRESS_REG); /* select register B, disalbe NMI  */
    outb((prev_status_b & 0x70) | 0x40 , MC146818_DATA_REG); /* set PIE to enale periodic interrupt, and clear AIE, UIE */
    outb(MC146818_REGISTER_STATUS_B, MC146818_ADDRESS_REG); /* enable NMI  */
    enable_irq(RTC_IRQ);    /* enable IRQ8 */
    sti();
}

/**
 * set_interrupt_rate
 * Set interrupt freq.
 * INPUT: none
 * OUTPUT: set register A of RTC
 */
void set_interrupt_rate(const int interrupt_rate) {
    cli();
    unsigned char prev_status_a;
    outb(MC146818_REGISTER_STATUS_A | CMOS_NMI_DISABLE, MC146818_ADDRESS_REG); /* select register A, disalbe NMI  */
    prev_status_a = inb(MC146818_DATA_REG); /* previous value of registers A */
    outb(MC146818_REGISTER_STATUS_A | CMOS_NMI_DISABLE, MC146818_ADDRESS_REG); /* select register A, disalbe NMI  */
    outb((prev_status_a & 0xF0) | interrupt_rate, MC146818_DATA_REG); /* write new interrupt rate : 2 Hz */
    outb(MC146818_REGISTER_STATUS_A, MC146818_ADDRESS_REG); /* enable NMI  */
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
    set_interrupt_rate(INTERRUPT_RATE_2Hz); /* set interrupt freq. */
}

/**
 * handler of RTC periodic interrupt
 * INPUT: none
 * OUTPUT: set register C of RTC
 */
void rtc_handler() {
    cli();
    if(test_rtc) {
        test_interrupts();
        test_rtc_cnt++;
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
    interrtupt_occured = 1;
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
    test_rtc ^= 1;
    if(!test_rtc) {
        while(test_rtc_cnt){
            test_interrupts();
            test_rtc_cnt++;
        }
    }
    sti();
}

int _log2(int n) {
    int ret = 0;
    while(!(n & 1)) ret++, n >>= 1;
    return ret;
}

int rtc_open() {
    set_interrupt_rate(INTERRUPT_RATE_2Hz);
    return 0;
}

int rtc_close(int32_t fd) {
    return 0;
}

int rtc_write(int32_t fd, const void* buf, int32_t nbytes) {
    if(nbytes != sizeof(uint32_t)) return -1;
    uint32_t freq = *((uint32_t*)buf);
    if(!RTC_VALID_FREQ(freq)) return -1;
    set_interrupt_rate(RTC_FREQ2RATE(freq));
    return 0;
}

int rtc_read(int32_t fd, const void* buf, int32_t nbytes) {
    interrtupt_occured = 0;
    while(!interrtupt_occured);
    return 0;
}