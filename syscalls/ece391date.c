#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"
#include "time.h"

int main() {
    int32_t fd = ece391_open("rtc");
    if (fd == -1) return 1;
    time_t time;
    time.Timezone = TIMEZONE;
    if (ece391_ioctl(fd, GET_TIME_CTL, &time)) return 2;

    ece391_printf("20%d %d/%d %s %d:%d:%d UTC%s%d\n", time.Year, time.Month,
                  time.Day_of_Month, day_of_week[time.Weekday], time.Hours, time.Minutes,
                  time.Seconds, TIMEZONE >= 0 ? "+" : "", TIMEZONE);
    ece391_close(fd);
    return 0;
}
