#ifndef MODEX_H
#define MODEX_H
#include "types.h"
#include "lib.h"
#include "page.h"

#define MODEX_VMEM_SIZE 65536

/*
 * macro used to target a specific video plane or planes when writing
 * to video memory in mode X; bits 8-11 in the mask_hi_bits enable writes
 * to planes 0-3, respectively
 */
#define SET_WRITE_MASK(mask_hi_bits)                                          \
    do                                                                        \
    {                                                                         \
        asm volatile("                                                     \
	movw $0x03C4,%%dx    	/* set write mask                    */;\
	movb $0x02,%b0                                                 ;\
	outw %w0,(%%dx)                                                 \
    " \
                     :                                                        \
                     : "a"((mask_hi_bits))                                    \
                     : "edx", "memory");                                      \
    } while (0)

int32_t modex_read(int32_t fd, void *buf, int32_t nbytes);
int32_t modex_write(int32_t fd, void *buf, int32_t nbytes);
int32_t modex_open(const uint8_t *filename);
int32_t modex_close(int32_t fd);
int32_t modex_ioctl(int32_t fd, int32_t request, void *buf);

/*
 * VGA_blank
 *   DESCRIPTION: Blank or unblank the VGA display.
 *   INPUTS: blank_bit -- set to 1 to blank, 0 to unblank
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void VGA_blank(int blank_bit);

/*
 * clear_screens
 *   DESCRIPTION: Fills the video memory with zeroes.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: fills all 256kB of VGA video memory with zeroes
 */
void clear_modex();

#endif
