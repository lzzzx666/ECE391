#include <stdint.h>

typedef struct time_t {
    uint8_t Seconds;        // 0–59
    uint8_t Minutes;        // 0–59
    uint8_t Hours;          // 0–23 in 24-hour mode
    uint8_t Weekday;        // 1–7, Sunday = 1
    uint8_t Day_of_Month;   // of Month 1–31
    uint8_t Month;          // 1–12
    uint8_t Year;           // 0–99
    uint8_t Century;        // (maybe) 19–20?
    int8_t  Timezone;       // -6 for American/Chicago
} time_t;

#define GET_TIME_CTL 1

#define TIMEZONE (-6)
