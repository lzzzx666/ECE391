/*simple graphical lib*/
#ifndef SGL_H
#define SGL_H
#include <stdint.h>
#include "ece391syscall.h"
#include "ece391support.h"
#include "ece391sysnum.h"

typedef struct BitMapFileHeader
{
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
} BitMapFileHeader_t;

typedef struct BitMapInfoHeader
{
    uint32_t biSize;
    int32_t biWidth;
    int32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t biXPelsPerMeter;
    int32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
} BitMapInfoHeader_t;

typedef struct RGB
{
    uint8_t B;
    uint8_t G;
    uint8_t R;
    uint8_t reserved;
} RGB_t;

typedef struct BitMap
{
    BitMapFileHeader_t header;
    BitMapInfoHeader_t info;
    RGB_t palette[256];
    uint8_t pixels[320 * 200];
} BitMap_t;

int32_t read_bitmap(const uint8_t *fname, BitMap_t *bitMap)
{
    int32_t fd;
    int32_t bitMapSize;
    if (-1 == (fd = ece391_open(fname)))
        return -1;
    if (-1 == ece391_ioctl(fd, IOCTL_FILE_SIZE, (void *)&bitMapSize))
        return -1;
    ece391_read(fd, bitMap, bitMapSize);
    ece391_close(fd);
    return bitMapSize;
}

int32_t plot_bitmap(int32_t VGAfd, int32_t bitMapSize, BitMap_t *bitMap)
{
    void *garbage;
    uint8_t *pixels = bitMap->pixels;
    ece391_ioctl(VGAfd, IOCTL_MODE_X, garbage);
    ece391_write(VGAfd, pixels, bitMapSize);
}

#endif
