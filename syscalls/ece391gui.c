#include <stdint.h>
#include "ece391sysnum.h"
#include "ece391syscall.h"
#include "ece391support.h"
#include "SGL.h"

BitMap_t bitMap;
cursorLoc_t cursor;
int32_t main()
{
    uint8_t buf[128];
    int32_t VGAfd, rtcfd;
    int32_t size;
    int32_t ret_val = 8;
    int i, garbage;
    uint8_t x, y;
    if (-1 == (VGAfd = ece391_open((uint8_t *)"VGA")))
        return 0;
    if (-1 == (rtcfd = ece391_open((uint8_t *)"rtc")))
        return 0;
    ret_val = ece391_write(rtcfd, &ret_val, 4);

    ece391_ioctl(VGAfd, 2, &garbage);
    size = read_bitmap("alma.bmp", &bitMap);
    plot_bitmap(VGAfd, size, &bitMap);
    x = y = 0;
    while (1)
    {
        cursor.x = 50 + (x++) % 50;
        cursor.y = 50 + (y++) % 50;
        ece391_ioctl(VGAfd, IOCTL_SET_CURSOR, &cursor);
        ece391_read(rtcfd, &garbage, 4);
    }
    ece391_close(VGAfd);
    ece391_close(rtcfd);
    ece391_printf("bmp size: %x\n", size);
    return 0;
}
