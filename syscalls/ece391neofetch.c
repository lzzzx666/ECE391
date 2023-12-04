#include <stdint.h>

#include "../student-distrib/colors.h"
#include "ece391support.h"
#include "ece391syscall.h"

#define w 24
#define h 14
#define dh 3
#define dw 6
#define W 28
#define H 18
#define N 3

void putchar(uint8_t c)
{
    static uint8_t *str = (uint8_t *)" ";
    str[0] = c;
    ece391_fdputs(1, str);
}

int main()
{
    static uint8_t map[W][H], color[W][H];
    const uint32_t offx[] = {4, 3, 2}, offy[] = {2, 3, 4};
    const uint8_t ch[] = {0xb0, 0xb1, 0xdb}, clr[] = {0x07, 0x0f, 0x0c};
    uint32_t n, i, j;
    for (j = 0; j < H; j++)
        for (i = 0; i < W; i++)
            map[i][j] = ' ', color[i][j] = CLR_DEFAULT_BYTE;
    for (n = 0; n < N; n++)
    {
        for (i = 0; i < w; i++)
        {
            for (j = 0; j < h; j++)
            {
                if ((j < dh || j >= h - dh) || !(i < dw || i >= w - dw))
                {
                    map[offx[n] + i][offy[n] + j] = ch[n];
                    color[offx[n] + i][offy[n] + j] = clr[n];
                }
            }
        }
    }
    for (j = 0; j < H; j++)
    {
        for (i = 0; i < W; i++)
        {
            ece391_fdputs(1, (const uint8_t *)CLR);
            putchar(color[i][j]);
            putchar(map[i][j]);
        }
        ece391_fdputs(1, (const uint8_t *)"\n");
    }

    ece391_fdputs(1, CLR "\x0A");
    ece391_fdputs(
        1,
        "\n\xdb\xdb\xdb\xdb\xdb\xdb\xbb   \xdb\xdb\xdb\xdb\xdb\xbb   \xdb\xdb\xbb  "
        "\xdb\xdb\xdb\xdb\xdb\xdb\xbb  "
        "\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xbb\n\xc8\xcd\xcd\xcd\xcd\xdb\xdb\xbb "
        "\xdb\xdb\xc9\xcd\xcd\xdb\xdb\xbb \xdb\xdb\xdb\xba "
        "\xdb\xdb\xc9\xcd\xcd\xcd\xdb\xdb\xbb \xdb\xdb\xc9\xcd\xcd\xcd\xcd\xbc\n "
        "\xdb\xdb\xdb\xdb\xdb\xc9\xbc \xc8\xdb\xdb\xdb\xdb\xdb\xdb\xba \xc8\xdb\xdb\xba "
        "\xdb\xdb\xba   \xdb\xdb\xba \xdb\xdb\xdb\xdb\xdb\xdb\xdb\xbb\n "
        "\xc8\xcd\xcd\xcd\xdb\xdb\xbb  \xc8\xcd\xcd\xcd\xdb\xdb\xba  \xdb\xdb\xba "
        "\xdb\xdb\xba   \xdb\xdb\xba "
        "\xc8\xcd\xcd\xcd\xcd\xdb\xdb\xba\n\xdb\xdb\xdb\xdb\xdb\xdb\xc9\xbc  "
        "\xdb\xdb\xdb\xdb\xdb\xc9\xbc  \xdb\xdb\xba \xc8\xdb\xdb\xdb\xdb\xdb\xdb\xc9\xbc "
        "\xdb\xdb\xdb\xdb\xdb\xdb\xdb\xba\n\xc8\xcd\xcd\xcd\xcd\xcd\xbc   "
        "\xc8\xcd\xcd\xcd\xcd\xbc   \xc8\xcd\xbc  \xc8\xcd\xcd\xcd\xcd\xcd\xbc  "
        "\xc8\xcd\xcd\xcd\xcd\xcd\xcd\xbc\n");
    ece391_fdputs(1, (const uint8_t *)CLR_DEFAULT);
    return 0;
}
