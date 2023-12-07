#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"
#include "keyboard.h"

#define NUM_KEYS 12

uint8_t keys[NUM_KEYS] = {SCANCODE_Q,
                          SCANCODE_W,
                          SCANCODE_E,
                          SCANCODE_R,
                          SCANCODE_T,
                          SCANCODE_Y,
                          SCANCODE_U,
                          SCANCODE_I,
                          SCANCODE_O,
                          SCANCODE_P,
                          SCANCODE_OPEN_BRACKET,
                          SCANCODE_CLOSE_BRACKET};

uint32_t freqs[NUM_KEYS] = {
    262,  // C4 (Do)
    277,  // C#4 (Do#)
    294,  // D4 (Re)
    311,  // D#4 (Re#)
    330,  // E4 (Mi)
    350,  // F4 (Fa)
    370,  // F#4 (Fa#)
    392,  // G4 (Sol)
    415,  // G#4 (Sol#)
    440,  // A4 (La)
    466,  // A#4 (La#)
    494,  // B4 (Si)
};

int main() {
    int32_t fd = ece391_open("beeper");
    if (fd == -1) return 1;
    // if (ece391_ioctl(1, 2, (void*)0)) return 2;

    uint8_t key;
    int32_t current, i;
    uint32_t freq;
    uint8_t buf[100];
    while (1) {
        if (ece391_ioctl(1, 2, &key)) return 2;
        // ece391_itoa(key, buf, 10);
        // ece391_fdputs(1, buf);
        for (i = 0; i < NUM_KEYS; i++) {
            if (key == keys[i] && key != current) {
                current = i;
                freq = freqs[i];
                if (ece391_write(fd, &freq, sizeof(freq))) return 2;
            } else if (key == BREAK(keys[i])) {
                current = -1;
                freq = 0;
                if (ece391_write(fd, &freq, sizeof(freq))) return 2;
            } else if (key == SCANCODE_ESC) {
                ece391_fdputs(1, "\nquit\n");
                ece391_close(fd);
                return 0;
            }
        }
    }

    return 0;
}
