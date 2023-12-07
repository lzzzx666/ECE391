#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"
#include "time.h"

const uint8_t* day_of_week[] = {0, "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
const uint32_t days_in_month[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

int main() {
    int32_t fd = ece391_open("rtc");
    if (fd == -1) return 1;
    time_t time;
    if (ece391_ioctl(fd, GET_TIME_CTL, &time)) return 2;

    time.Hours += TIMEZONE;
    if (time.Hours < 0) time.Day_of_Month--;
    if (time.Hours >= 24) time.Day_of_Month++;
    if (time.Day_of_Month < 1) time.Month--;
    if (time.Day_of_Month > days_in_month[time.Month]) time.Month++;
    if (time.Month < 1) time.Year--;
    if (time.Month > 12) time.Year++;

    ece391_printf("20%d %d/%d %s %d:%d:%d UTC%s%d\n", time.Year, time.Month,
                  time.Day_of_Month, day_of_week[time.Weekday], time.Hours, time.Minutes,
                  time.Seconds, TIMEZONE >= 0 ? "+" : "", TIMEZONE);
    ece391_close(fd);
    return 0;
}
