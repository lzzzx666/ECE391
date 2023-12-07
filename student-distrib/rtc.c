#include "rtc.h"

#include "i8259.h"
#include "lib.h"
#include "types.h"

#include "pcb.h"

static uint32_t fd_freqs[MAX_TASK] = {0};
volatile uint8_t interrtupt_occured[MAX_TASK] = {0};
uint32_t time_tick[MAX_TASK] = {
    0}; /* every 1 tick : 1sec / (INTERRUPT_FREQ_HI * current_freq) */

uint32_t current_freq = 2;

/**
 * enalbe_inter
 * Enalbe periodic interrupt of RTC
 * INPUT: none
 * OUTPUT: set register B of RTC
 */
void enalbe_inter() {
    cli();
    unsigned char prev_status_b;
    outb(MC146818_REGISTER_STATUS_B | CMOS_NMI_DISABLE,
         MC146818_ADDRESS_REG);             /* select register B, disalbe NMI */
    prev_status_b = inb(MC146818_DATA_REG); /* get previous value of B */
    outb(MC146818_REGISTER_STATUS_B | CMOS_NMI_DISABLE,
         MC146818_ADDRESS_REG); /* select register B, disalbe NMI  */
    outb((prev_status_b & 0x70) | 0x40,
         MC146818_DATA_REG); /* set PIE to enale periodic interrupt, and clear AIE, UIE */
    outb(MC146818_REGISTER_STATUS_B, MC146818_ADDRESS_REG); /* enable NMI  */
    enable_irq(RTC_IRQ);                                    /* enable IRQ8 */
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
    outb(MC146818_REGISTER_STATUS_A | CMOS_NMI_DISABLE,
         MC146818_ADDRESS_REG);             /* select register A, disalbe NMI  */
    prev_status_a = inb(MC146818_DATA_REG); /* previous value of registers A */
    outb(MC146818_REGISTER_STATUS_A | CMOS_NMI_DISABLE,
         MC146818_ADDRESS_REG); /* select register A, disalbe NMI  */
    outb((prev_status_a & 0xF0) | interrupt_rate,
         MC146818_DATA_REG); /* write new interrupt rate : 2 Hz */
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
    enalbe_inter();                        /* enalbe periodic interrupt */
    set_interrupt_rate(INTERRUPT_RATE_HI); /* set interrupt freq. */
}

/**
 * handler of RTC periodic interrupt
 * INPUT: none
 * OUTPUT: set register C of RTC
 */
void rtc_handler() {
    cli();
    outb(MC146818_REGISTER_STATUS_C, MC146818_ADDRESS_REG); /* select register C */
    (void)inb(MC146818_DATA_REG); /* read registers C, this cleares (IRQ) signal */
    send_eoi(RTC_IRQ);            /* end-of-interrupt */
    int32_t fd;
    for (fd = 0; fd < MAX_FD_NUM; fd++) {
        if(!fd_freqs[fd]) continue;
        time_tick[fd] += fd_freqs[fd];
        if (time_tick[fd] >= INTERRUPT_FREQ_HI) {
            time_tick[fd] -= INTERRUPT_FREQ_HI;
            interrtupt_occured[fd] = 1;
        }
    }
    sti();
}

/**
 * rtc_open
 * set RTC interrupt frequency
 */
int32_t rtc_open(const uint8_t *fname) { return 0; }

/**
 * rtc_close
 * close RTC
 */
int32_t rtc_close(int32_t fd) {
    fd = get_current_pcb()->pid;
    fd_freqs[fd] = 0;
    return 0;
}

/**
 * rtc_write
 * set interrupt value
 */
int32_t rtc_write(int32_t fd, const void *buf, int32_t nbytes) {
    fd = get_current_pcb()->pid;
    if (nbytes != sizeof(uint32_t)) return -1;
    uint32_t freq = *((uint32_t *)buf);
    if (freq > INTERRUPT_FREQ_HI) return 1;
    fd_freqs[fd] = freq;
    time_tick[fd] = 0;
    return 0;
}

/**
 * rtc_read
 * return after an interrupt occured
 */
int32_t rtc_read(int32_t fd, void *buf, int32_t nbytes) {
    fd = get_current_pcb()->pid;
    interrtupt_occured[fd] = 0;
    while (!interrtupt_occured[fd])
        ;
    return 0;
}

// TODO comment
int32_t rtc_ioctl(int32_t fd, int32_t request, void *buf) { return 0; }
