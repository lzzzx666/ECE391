#ifndef VGA_H
#define VGA_H
#include "types.h"
#include "lib.h"
#include "page.h"

#define MODE_X_VMEM_SIZE 65536

#define NUM_SEQUENCER_REGS 5
#define NUM_CRTC_REGS 25
#define NUM_GRAPHICS_REGS 9
#define NUM_ATTR_REGS 22

#define IOCTL_TEXT_MODE 1
#define IOCTL_MODE_X 2

#define MODE_X_HIGHT 320
#define MODE_X_WIDTH 200
#define SCREEN_SIZE (MODE_X_HIGHT * MODE_X_WIDTH)
#define PLANE_SIZE (SCREEN_SIZE/4)

#define VGA_LOCKED 1
#define VGA_UNLOCKED 0

/*
 * macro used to target a specific video plane or planes when writing
 * to video memory in mode X; bits 8-11 in the mask_hi_bits enable writes
 * to planes 0-3, respectively
 */
#define SET_WRITE_MASK(mask_hi_bits)                                         \
    do                                                                       \
    {                                                                        \
        asm volatile("                                                     \
	movw $0x03C4,%%dx    	/* set write mask                    */;\
	movb $0x02,%b0                                                 ;\
	outw %w0,(%%dx)                                                 \
    " \
                     :                                                       \
                     : "a"((mask_hi_bits))                                   \
                     : "edx", "memory");                                     \
    } while (0)

int32_t vga_read(int32_t fd, void *buf, int32_t nbytes);
int32_t vga_write(int32_t fd, void *buf, int32_t nbytes);
int32_t vga_open(const uint8_t *filename);
int32_t vga_close(int32_t fd);
int32_t vga_ioctl(int32_t fd, int32_t request, void *buf);

void VGA_blank(int blank_bit);
void clear_modex();
void set_seq_regs_and_reset(unsigned short table[NUM_SEQUENCER_REGS],
                            unsigned char val);
void set_CRTC_registers(unsigned short table[NUM_CRTC_REGS]);
void set_attr_registers(unsigned char table[NUM_ATTR_REGS * 2]);
void set_graphics_registers(unsigned short table[NUM_GRAPHICS_REGS]);
void fill_palette_mode_x();
void save_seq_regs(unsigned short table[NUM_SEQUENCER_REGS], unsigned char *mor);
void save_CRTC_registers(unsigned short table[NUM_CRTC_REGS]);
void save_attr_registers(unsigned char table[NUM_ATTR_REGS * 2]);
void save_graphics_registers(unsigned short table[NUM_GRAPHICS_REGS]);
void fill_palette_text();
void write_font_data();
void enable_text_mode();
void enable_mode_x();

#endif
