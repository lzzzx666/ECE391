#include "beeper.h"

#include "lib.h"

int32_t beeper_open(const uint8_t *fname) { return 0; }

static void no_sound() {
    uint8_t tmp = inb(0x61) & 0xFC;
    outb(tmp, 0x61);
}

int32_t beeper_close(int32_t fd) {
    no_sound();
    return 0;
}

static void play_sound(uint32_t nFrequence) {
    uint32_t Div;
    uint8_t tmp;
    // Set the PIT to the desired frequency
    Div = 1193180 / nFrequence;
    outb(0xb6, 0x43);
    outb((uint8_t)(Div), 0x42);
    outb((uint8_t)(Div >> 8), 0x42);
    // And play the sound using the PC speaker
    tmp = inb(0x61);
    if (tmp != (tmp | 3)) {
        outb(tmp | 3, 0x61);
    }
}

int32_t beeper_write(int32_t fd, const void *buf, int32_t nbytes) {
    if (nbytes != sizeof(uint32_t)) return -1;
    uint32_t freq = *((const uint32_t *)buf);
    if (!freq)
        no_sound();
    else
        play_sound(freq);
    return 0;
}

int32_t beeper_read(int32_t fd, void *buf, int32_t nbytes) { return 0; }

int32_t beeper_ioctl(int32_t fd, int32_t request, void *buf) { return 0; }
