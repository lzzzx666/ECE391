#ifndef COLORS_H
#define COLORS_H

#define CLR_BLACK   0
#define CLR_BLUE    1
#define CLR_GREEN   2
#define CLR_CYAN    3
#define CLR_RED     4
#define CLR_MAGENTA 5
#define CLR_BROWN   6
#define CLR_GREY    7
#define CLR_FLICKER     1
#define CLR_NONFLICKER   0

#define CLR_CONTROL_BYTE 0x1B
#define CLR "\x1B"
#define COLOR(bg, flicker, fg) ((uint8_t)(((bg) << 4) | ((flicker) << 3) | (fg)))
#define CLR_DEFAULT CLR"\x07"
#define CLR_DEFAULT_BYTE 0x07

#endif  // COLORS_H
