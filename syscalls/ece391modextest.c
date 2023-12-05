#include <stdint.h>
#include "ece391support.h"
#include "ece391syscall.h"
#include "SGL.h"
BitMap_t bitMap;
int32_t main()
{
    uint8_t buf[128];
    int32_t VGAfd, rtcfd;
    int32_t size;
    int32_t ret_val = 8;
    int i, garbage;

    if (-1 == (VGAfd = ece391_open((uint8_t *)"VGA")))
        return 0;
    if (-1 == (rtcfd = ece391_open((uint8_t *)"rtc")))
        return 0;

    ret_val = ece391_write(rtcfd, &ret_val, 4);
    ece391_ioctl(VGAfd, 2, &garbage);
    for (i = 0; i < 10; i++)
    {
        ece391_read(VGAfd, &garbage, 1);
        ece391_read(rtcfd, &garbage, 4);
    }
    size = read_bitmap("cal2.bmp", &bitMap);
    plot_bitmap(VGAfd, size, &bitMap);
    

    ece391_close(VGAfd);
    ece391_close(rtcfd);
    ece391_printf("bmp size: %x\n",bitMap.header.bfSize);
    return 0;
}
