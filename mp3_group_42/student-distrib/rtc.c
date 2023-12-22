#include "rtc.h"

#include "i8259.h"
#include "lib.h"
#include "pcb.h"
#include "time.h"
#include "types.h"

static float fd_freqs[MAX_TASK] = {0};
volatile uint8_t interrtupt_occured[MAX_TASK] = {0};
float time_tick[MAX_TASK] = {
    0}; /* every 1 tick : 1sec / (INTERRUPT_FREQ_HI * current_freq) */

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
    outb((prev_status_b & 0x70) | 0x40 | 0x06,
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
        if (!fd_freqs[fd]) continue;
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
    float freq;
    if (nbytes == sizeof(uint32_t))
        freq = *((uint32_t *)buf);
    else if (nbytes == sizeof(uint16_t))
        freq = 1000. / (float)(*((uint16_t *)buf));
    else
        return 1;

    if (freq > INTERRUPT_FREQ_HI) return 2;
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

static uint8_t read_rtc_reg(uint8_t reg) {
    outb(reg | CMOS_NMI_DISABLE, MC146818_ADDRESS_REG);
    return inb(MC146818_DATA_REG);
}

int32_t rtc_ioctl(int32_t fd, int32_t request, void *buf) {
    time_t time;
    switch (request) {
        case GET_TIME_CTL:
            time.Seconds = read_rtc_reg(RTC_SECONDS);
            time.Minutes = read_rtc_reg(RTC_MINUTES);
            time.Hours = read_rtc_reg(RTC_HOURS);
            time.Weekday = read_rtc_reg(RTC_WEEKDAY);
            time.Day_of_Month = read_rtc_reg(RTC_DAY_OF_MONTH);
            time.Month = read_rtc_reg(RTC_MONTH);
            time.Year = read_rtc_reg(RTC_YEAR);
            time.Century = read_rtc_reg(RTC_CENTURY);

            time.Timezone = ((time_t *)buf)->Timezone;
            time.Hours += time.Timezone;
            if (time.Hours < 0) time.Day_of_Month--;
            if (time.Hours >= 24) time.Day_of_Month++;
            if (time.Day_of_Month < 1) time.Month--;
            if (time.Day_of_Month > days_in_month[time.Month]) time.Month++;
            if (time.Month < 1) time.Year--;
            if (time.Month > 12) time.Year++;

            *((time_t *)buf) = time;
            break;
        default:
            return 1;
    }
    return 0;
}
