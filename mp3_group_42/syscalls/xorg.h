#ifndef XORG_H
#define XORG_H

#include <stdint.h>

#define BORDER_COLOR 4
#define BACKGROUND_C 0
#define MAX_NUM_WINDOWS 10
#define BORDER_WIDTH 1
#define HEADER_HEIGHT 5

typedef uint8_t (*draw_func)(uint32_t, uint32_t, uint32_t, uint32_t);

typedef struct window
{
    int32_t layer;
    uint8_t display;
    uint8_t active;

    uint32_t height;
    uint32_t width;
    uint32_t left;
    uint32_t top;

    draw_func *draw;

} window_t;



#endif
