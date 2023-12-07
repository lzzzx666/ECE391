#ifndef RTC_H
#define RTC_H

#include "keyboard.h"

/* MC146818 RTC registers */
#define MC146818_ADDRESS_REG 0x70
#define MC146818_DATA_REG 0x71

#define MC146818_REGISTER_STATUS_A 0x0A
#define MC146818_REGISTER_STATUS_B 0x0B
#define MC146818_REGISTER_STATUS_C 0x0C

/**
 * REGISTER B
 *   MSB                                         LSB
 * +-----+-----+-----+-----+------+----+-------+-----+
 * |  b7 |  b6 |  b5 |  b4 |  b3  | b2 |  bl   | bO  |  R/W Register
 * +-----+-----+-----+-----+------+----+-------+-----+
 * | SET | PIE | AIE | UIE | SQWE | DM | 24/12 | DSE |
 * +-----+-----+-----+-----+------+----+-------+-----+
 *
 *
 * REGISTER A
 *   MSB                                         LSB
 * +-----+-----+-----+-----+-----+-----+-----+-----+
 * | b7  | b6  | b5  | b4  | b3  | b2  | b1  | bO  |  R/W Register except UIP
 * +-----+-----+-----+-----+-----+-----+-----+-----+
 * | UIP | DV2 | DV1 | DV0 | RS3 | RS2 | RS1 | RS0 |
 * +-----+-----+-----+-----+-----+-----+-----+-----+
 *
 */

#define CMOS_NMI_DISABLE 0x80

/* Interrupt freq. */
#define INTERRUPT_RATE_HI 6
#define INTERRUPT_FREQ_HI (32768 >> (INTERRUPT_RATE_HI - 1))
/* interrupt frequency is (32768 >> (INTERRUPT_RATE - 1)) */

#define RTC_IRQ 8
// #define TEST_PRINT_PERIODIC

/* initialize rtc */
extern void rtc_init();

/* rtc interrupt handler */
extern void rtc_handler();

extern int32_t rtc_open(const uint8_t *fname);

extern int32_t rtc_close(int32_t fd);

extern int32_t rtc_write(int32_t fd, const void *buf, int32_t nbytes);

extern int32_t rtc_read(int32_t fd, void *buf, int32_t nbytes);

extern int32_t rtc_ioctl(int32_t fd, int32_t request, void *buf);

#define MAX_FD_NUM 8

#endif
