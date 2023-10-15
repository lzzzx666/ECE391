#ifndef RTC_H
#define RTC_H

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

/* initialize rtc */
extern void rtc_init();

/* rtc interrupt handler */
extern void rtc_handler();

#endif
