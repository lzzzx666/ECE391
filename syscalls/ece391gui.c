#include <stdint.h>
#include "ece391sysnum.h"
#include "ece391syscall.h"
#include "ece391support.h"
#include "SGL.h"
#include "time.h"

BitMap_t bitMap;
cursorLoc_t cursor;
<<<<<<< HEAD
void execute(int VGAfd, char *name, int *garbage, int size)
{
    uint8_t buf[10];
    ece391_ioctl(VGAfd, IOCTL_TEXT_MODE, garbage);
    ece391_execute(name);
    if(ece391_strcmp(name,"piano")!=0)
    ece391_read(0,buf,1);
    ece391_ioctl(VGAfd, IOCTL_MODE_X, garbage);
    size = read_bitmap("desktop.bmp", &bitMap);
    plot_bitmap(VGAfd, size, &bitMap);
}


void itoa_align_copy(uint8_t val, int width, uint8_t fill, uint8_t *pos) {
    static uint8_t buf[20];
    ece391_itoa(val, buf, 10);
    int len = ece391_strlen(buf), i;
    for(i = 0; i < width; i++) {
        int idx = i + len -  width;
        pos[i] = idx < 0 ? fill : buf[idx];
    }
}
int32_t main()
{
    uint8_t buf[128];
    uint16_t mouseBuf[3];
    int32_t VGAfd, rtcfd, mousefd;
    int32_t size;
    int32_t ret_val = 64;
    int garbage;

    if (-1 == (VGAfd = ece391_open((uint8_t *)"VGA")))
        return 0;
    if (-1 == (rtcfd = ece391_open((uint8_t *)"rtc")))
        return 0;
    if (-1 == (mousefd = ece391_open((uint8_t *)"mouse")))
        return 0;

    ret_val = ece391_write(rtcfd, &ret_val, 4);

    ece391_ioctl(VGAfd, IOCTL_MODE_X, &garbage);
    size = read_bitmap("desktop.bmp", &bitMap);
    plot_bitmap(VGAfd, size, &bitMap);


    time_t time, prev;
    time.Timezone = TIMEZONE;
    uint8_t *time_buf = "2046/08/17 04:32:01 Sun";


    while (1)
    {
        ece391_read(rtcfd, &garbage, 4);
        ece391_read(mousefd, mouseBuf, 3);
        ece391_ioctl(VGAfd, IOCTL_SET_CURSOR, &(mouseBuf[1]));

        if ((mouseBuf[0]) & 1)
        {
            if ((mouseBuf[1] & 0xff) < 40)
            {
                switch ((mouseBuf[2] & 0xff) / 40)
                {
                case 0:
                    execute(VGAfd, (char *)"pingpong", &garbage, size);
                    break;
                case 1:
                    execute(VGAfd, (char *)"neofetch", &garbage, size);
                    break;
                case 2:
                    execute(VGAfd, (char *)"piano", &garbage, size);
                    break;
                case 3:
                    execute(VGAfd, (char *)"ls", &garbage, size);
                    break;
                case 4:
                    execute(VGAfd, (char *)"date", &garbage, size);
                    break;
                default:
                    break;
                }
            }
            else
            {
                continue;
            }

        if (ece391_ioctl(rtcfd, GET_TIME_CTL, &time)) return 2;

        if(time.Seconds != prev.Seconds) {
            prev = time;
            itoa_align_copy(time.Year, 2, '0', time_buf + 2);
            itoa_align_copy(time.Month, 2, '0', time_buf + 5);
            itoa_align_copy(time.Day_of_Month, 2, '0', time_buf + 8);
            itoa_align_copy(time.Hours, 2, ' ', time_buf + 11);
            itoa_align_copy(time.Minutes, 2, '0', time_buf + 14);
            itoa_align_copy(time.Seconds, 2, '0', time_buf + 17);
            ece391_strcpy(time_buf + 20, day_of_week[time.Weekday]);
            if (ece391_ioctl(VGAfd, /*IOCTL_DISP_TIME*/ 6, time_buf)) return 3;

        }
    }
    ece391_close(VGAfd);
    ece391_close(rtcfd);
    ece391_close(mousefd);
    ece391_printf("bmp size: %x\n", size);
    return 0;
}
