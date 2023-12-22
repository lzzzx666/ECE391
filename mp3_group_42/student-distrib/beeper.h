#ifndef BEEPER_H
#define BEEPER_H

#include "beeper.h"

#define BEEPER_FILETYPE 5

#include "lib.h"

int32_t beeper_open(const uint8_t *fname);

int32_t beeper_close(int32_t fd);

int32_t beeper_write(int32_t fd, const void *buf, int32_t nbytes);

int32_t beeper_read(int32_t fd, void *buf, int32_t nbytes);

int32_t beeper_ioctl(int32_t fd, int32_t request, void *buf);

#endif
