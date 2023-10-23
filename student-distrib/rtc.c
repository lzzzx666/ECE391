#include "rtc.h"
#include "lib.h"
#include "i8259.h"

volatile unsigned char test_rtc = 0;
char test_rtc_cnt = 0;

#ifdef RTC_VIRTUALIZE
uint32_t current_freq = 2;
uint32_t time_tick = 0; /* every 1 tick : 1sec / (INTERRUPT_FREQ_HI * current_freq) */
#endif /* RTC_VIRTUALIZE */
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
#ifdef RTC_VIRTUALIZE
    time_tick += current_freq;
    /* if (inter_count / INTERRUPT_FREQ_HI >= 1 / current_freq) */
    /* if(inter_count * current_freq >= INTERRUPT_FREQ_HI) {
        inter_count = 0;
        interrtupt_occured = 1;
    } */
    if(time_tick >= INTERRUPT_FREQ_HI) {
        time_tick -= INTERRUPT_FREQ_HI;
        interrtupt_occured = 1;
    }
#else
    interrtupt_occured = 1;
#endif /* RTC_VIRTUALIZE */
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

/**
 * rtc_open
 * set RTC interrupt frequency
 */
int rtc_open() {
#ifdef RTC_VIRTUALIZE
    set_interrupt_rate(INTERRUPT_RATE_HI);
#else
    set_interrupt_rate(INTERRUPT_RATE_2Hz);
#endif /* RTC_VIRTUALIZE */
    return 0;
}

/**
 * rtc_close
 * close RTC
 */
int rtc_close(int32_t fd) {
    return 0;
}

/**
 * rtc_write
 * set interrupt value
 */
int rtc_write(int32_t fd, const void* buf, int32_t nbytes) {
    if(nbytes != sizeof(uint32_t)) return -1;
    uint32_t freq = *((uint32_t*)buf);
#ifdef RTC_VIRTUALIZE
    set_interrupt_rate(INTERRUPT_RATE_HI);
    time_tick = 0;
    current_freq = freq;
#else
    if(!RTC_VALID_FREQ(freq)) return -1;
    set_interrupt_rate(RTC_FREQ2RATE(freq));
#endif
    return 0;
}

/**
 * rtc_write
 * return after an interrupt occured
 */
int rtc_read(int32_t fd, const void* buf, int32_t nbytes) {
    interrtupt_occured = 0;
    while(!interrtupt_occured);
    return 0;
}