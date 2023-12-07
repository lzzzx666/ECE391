#include "types.h"

typedef struct time_t {
    uint8_t Seconds;        // 0–59
    uint8_t Minutes;        // 0–59
    uint8_t Hours;          // 0–23 in 24-hour mode
    uint8_t Weekday;        // 1–7, Sunday = 1
    uint8_t Day_of_Month;   // of Month 1–31
    uint8_t Month;          // 1–12
    uint8_t Year;           // 0–99
    uint8_t Century;        // (maybe) 19–20?
} time_t;

#define RTC_SECONDS             0x00  // Seconds register, range 0-59
#define RTC_MINUTES             0x02  // Minutes register, range 0-59
#define RTC_HOURS               0x04  // Hours register, range 0-23 in 24-hour mode, 1-12 in 12-hour mode
#define RTC_WEEKDAY             0x06  // Weekday register, range 1-7, Sunday = 1
#define RTC_DAY_OF_MONTH        0x07  // Day of Month register, range 1-31
#define RTC_MONTH               0x08  // Month register, range 1-12
#define RTC_YEAR                0x09  // Year register, range 0-99
#define RTC_CENTURY             0x32  // Century register, range 19-20 (maybe)

#define GET_TIME_CTL 1
