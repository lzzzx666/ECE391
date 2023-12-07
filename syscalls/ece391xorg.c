#include <stdint.h>

#include "SGL.h"
#include "ece391support.h"
#include "ece391syscall.h"
#include "xorg.h"

BitMap_t bitMap;
window_t windows[MAX_NUM_WINDOWS];

uint8_t draw_default(uint32_t w, uint32_t h, uint32_t x, uint32_t y) {
    return 21;
}

void windows_init() {
    int32_t i;
    for(i = 0; i < MAX_NUM_WINDOWS; i++) {
        windows[i].display = 0;
        windows[i].left = windows[i].top = i * 5;
        windows[i].width = windows[i].height = 150 - i * 10;
        windows[i].layer = i;
        windows[i].draw = draw_default;
    }
}



void update() {
    // static 
    uint32_t i;
    // for(i = 0; i < MAX_NUM_WINDOWS; i++) {
    //     if(!windows[i].display) continue;
        
    // }
    for(i = 0 ;i < 255; i++) bitMap.pixels[i] = i;
    for(i = 0 ;i < 255; i++)  bitMap.palette[i].R = 255;
    // bitMap.pixels[4] = 123;
    // bitMap.palette[123] = (RGB_t){0, 255, 0};
}

int32_t main() {

    uint8_t fd;
    if (!(fd = ece391_open((uint8_t *)"vga13h"))) return 1;
    ece391_fdputs(1, "open success");
    return 0;

    uint8_t buf[128];
    int32_t VGAfd, rtcfd;
    int32_t size;
    int32_t ret_val = 8;

    if (-1 == (VGAfd = ece391_open((uint8_t *)"VGA"))) return 0;
    if (-1 == (rtcfd = ece391_open((uint8_t *)"rtc"))) return 0;

    windows_init();

    while (1) {
        update();
        plot_bitmap(VGAfd, size, &bitMap);
    }

    ece391_close(VGAfd);
    ece391_close(rtcfd);
    return 0;
}
