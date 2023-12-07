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

    if (-1 == (VGAfd = ece391_open((uint8_t *)"VGA")))
        return 0;

    ret_val = ece391_write(rtcfd, &ret_val, 4);
    ece391_ioctl(VGAfd, 2, &garbage);

    size = read_bitmap("alma.bmp", &bitMap);
    plot_bitmap(VGAfd, size, &bitMap);

    cursor.x = 10;
    cursor.y = 10;
    ece391_ioctl(VGAfd, IOCTL_SET_CURSOR, &cursor);
    while (1)
        ;
    ece391_close(VGAfd);
    ece391_close(rtcfd);
    ece391_printf("bmp size: %x\n", size);
    return 0;
}
