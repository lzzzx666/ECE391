#include <stdint.h>
#include "ece391sysnum.h"
#include "ece391syscall.h"
#include "ece391support.h"
#include "SGL.h"

BitMap_t bitMap;
cursorLoc_t cursor;
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
int32_t main()
{
    uint8_t buf[128];
    uint16_t mouseBuf[3];
    int32_t VGAfd, rtcfd, mousefd;
    int32_t size;
    int32_t ret_val = 64;
    int i, garbage;

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
        }
    }
    ece391_close(VGAfd);
    ece391_close(rtcfd);
    ece391_close(mousefd);
    ece391_printf("bmp size: %x\n", size);
    return 0;
}
