#include "vga.h"
#include "vga_data.h"
#define DEBUG 1
uint8_t VGA_lock = VGA_UNLOCKED;
int32_t vga_read(int32_t fd, void *buf, int32_t nbytes)
{
    int i;
    for (i = 0; i < MODE_X_VMEM_SIZE; i++)
    {
        ((uint8_t *)MODE_X_VMEM_ADDR)[i]++;
    }
    outw((MODE_X_VMEM_ADDR & 0xFF00) | 0x0C, 0x03D4);
    outw(((MODE_X_VMEM_ADDR & 0x00FF) << 8) | 0x0D, 0x03D4);
    return SYSCALL_SUCCESS;
}

int32_t vga_write(int32_t fd, void *buf, int32_t nbytes)
{
    if (fd > MAX_FD || buf == NULL)
        return SYSCALL_FAIL;
    int i, planeIdx;
    uint8_t *vmem = (uint8_t *)MODE_X_VMEM_ADDR;
    uint8_t *addr;
    for (i = 0; i < 320*200; i++)
    {
        planeIdx = i % 4;
        addr = MODE_X_VMEM_ADDR + (i >> 2) + planeIdx * PLANE_SIZE;
        *addr = ((uint8_t *)buf)[i];
    }
    // memcpy((void *)MODE_X_VMEM_ADDR, (const void *)buf, nbytes);
    outw((MODE_X_VMEM_ADDR & 0xFF00) | 0x0C, 0x03D4);
    outw(((MODE_X_VMEM_ADDR & 0x00FF) << 8) | 0x0D, 0x03D4);

    return SYSCALL_SUCCESS;
}

int32_t vga_open(const uint8_t *filename)
{
    if (VGA_lock == VGA_LOCKED)
        return SYSCALL_FAIL;
    save_CRTC_registers(text_CRTC);
    save_seq_regs(text_seq, &MOR);
    save_graphics_registers(text_graphics);
    save_attr_registers(text_attr);
    // save_palette(text_palette_RGB);
    return SYSCALL_SUCCESS;
}

int32_t vga_close(int32_t fd)
{
    VGA_blank(1);
    write_font_data();                      /* copy fonts to video mem */
    set_seq_regs_and_reset(text_seq, 0x67); /* sequencer registers     */
    set_CRTC_registers(text_CRTC);          /* CRT control registers   */
    set_attr_registers(text_attr);          /* attribute registers     */
    set_graphics_registers(text_graphics);  /* graphics registers      */
    fill_palette_text();                    /* palette colors          */
    VGA_blank(0);                           /* unblank the screen      */
    memset((void *)VIDEO, 0, 80 * 25 * 2);
    free_paging_directory((MODE_X_VMEM_ADDR >> 22) & 0x3FF);

#if DEBUG
    int i;
    printf("CRTC regs\n");
    for (i = 0; i < NUM_CRTC_REGS; i++)
    {
        printf("%#x    ", text_CRTC[i]);
        if (i % 4 == 3)
            printf("\n");
    }
    printf("\n");

    printf("seq regs\n");
    for (i = 0; i < NUM_SEQUENCER_REGS; i++)
    {
        printf("%#x    ", text_seq[i]);
        if (i % 4 == 3)
            printf("\n");
    }
    printf("\n");

    printf("graphics regs\n");
    for (i = 0; i < NUM_GRAPHICS_REGS; i++)
    {
        printf("%#x    ", text_graphics[i]);
        if (i % 4 == 3)
            printf("\n");
    }
    printf("\n");

    printf("attribute regs\n");
    for (i = 0; i < NUM_ATTR_REGS; i++)
    {
        printf("%#x    ", text_attr[i]);
        if (i % 4 == 3)
            printf("\n");
    }
    printf("\nMOR: %#x\n", MOR);
    printf("\n");
#endif

    return 0;
}

int32_t vga_ioctl(int32_t fd, int32_t request, void *buf)
{
    switch (request)
    {
    case IOCTL_TEXT_MODE:
        enable_text_mode();
        break;
    case IOCTL_MODE_X:
        enable_mode_x();
        break;
    default:
        break;
    }
    printf("trying to ioctl to modex\n");
    return 0;
}

void VGA_blank(int blankBit)
{
    /*
     * Move blanking bit into position for VGA sequencer register
     * (index 1).
     */
    blankBit = ((blankBit & 1) << 5);

    asm volatile(
        "movb $0x01,%%al         /* Set sequencer index to 1. */       ;"
        "movw $0x03C4,%%dx                                             ;"
        "outb %%al,(%%dx)                                              ;"
        "incw %%dx                                                     ;"
        "inb (%%dx),%%al         /* Read old value.           */       ;"
        "andb $0xDF,%%al         /* Calculate new value.      */       ;"
        "orl %0,%%eax                                                  ;"
        "outb %%al,(%%dx)        /* Write new value.          */       ;"
        "movw $0x03DA,%%dx       /* Enable display (0x20->P[0x3C0]) */ ;"
        "inb (%%dx),%%al         /* Set attr reg state to index. */    ;"
        "movw $0x03C0,%%dx       /* Write index 0x20 to enable. */     ;"
        "movb $0x20,%%al                                               ;"
        "outb %%al,(%%dx)                                               "
        :
        : "g"(blankBit)
        : "eax", "edx", "memory");
}

void clear_modex()
{
    /* Write to all four planes at once. */
    SET_WRITE_MASK(0x0F00);

    /* Set 64kB to zero (times four planes = 256kB). */
    memset((void *)MODE_X_VMEM_ADDR, 0, (uint32_t)MODE_X_VMEM_SIZE);
}

/*
 * set_seq_regs_and_reset
 *   DESCRIPTION: Set VGA sequencer registers and miscellaneous output
 *                register; array of registers should force a reset of
 *                the VGA sequencer, which is restored to normal operation
 *                after a brief delay.
 *   INPUTS: table -- table of sequencer register values to use
 *           val -- value to which miscellaneous output register should be set
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void set_seq_regs_and_reset(unsigned short table[NUM_SEQUENCER_REGS],
                            unsigned char val)
{
    /*
     * Dump table of values to sequencer registers.  Includes forced reset
     * as well as video blanking.
     */
    rep_outsw(0x03C4, table, NUM_SEQUENCER_REGS);

    /* Delay a bit... */
    {
        volatile int ii;
        for (ii = 0; ii < 10000; ii++)
            ;
    }

    /* wet VGA miscellaneous output register. */
    outw(val, 0x03C2);

    /* Turn sequencer on (array values above should always force reset). */
    outw(0x0300, 0x03C4);
}

/*
 * set_CRTC_registers
 *   DESCRIPTION: Set VGA cathode ray tube controller (CRTC) registers.
 *   INPUTS: table -- table of CRTC register values to use
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void set_CRTC_registers(unsigned short table[NUM_CRTC_REGS])
{
    /* clear protection bit to enable write access to first few registers */
    outw(0x0011, 0x03D4);
    rep_outsw(0x03D4, table, NUM_CRTC_REGS);
}

/*
 * set_attr_registers
 *   DESCRIPTION: Set VGA attribute registers.  Attribute registers use
 *                a single port and are thus written as a sequence of bytes
 *                rather than a sequence of words.
 *   INPUTS: table -- table of attribute register values to use
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void set_attr_registers(unsigned char table[NUM_ATTR_REGS * 2])
{
    /* Reset attribute register to write index next rather than data. */
    inb(0x03DA);
    rep_outsb(0x03C0, table, NUM_ATTR_REGS * 2);
}

void set_graphics_registers(unsigned short table[NUM_GRAPHICS_REGS])
{
    rep_outsw(0x03CE, table, NUM_GRAPHICS_REGS);
}

void save_seq_regs(unsigned short table[NUM_SEQUENCER_REGS], unsigned char *mor)
{
    int i;
    uint8_t val;
    inb(0x03C4);
    for (i = 0; i < NUM_SEQUENCER_REGS; i++)
    {
        outb(i, 0x03C4);
        val = inb(0x03C5);
        table[i] = val;
        table[i] = table[i] << 8;
        table[i] += i;
        val = 0;
    }
    *mor = inb(0x03CC);
    table[0] = 0x0100;
}

void save_CRTC_registers(unsigned short table[NUM_CRTC_REGS])
{
    int i;
    uint8_t val;
    inb(0x3D4);
    for (i = 0; i < NUM_CRTC_REGS; i++)
    {
        outb(i, 0x03D4);
        val = inb(0x03D5);
        table[i] = val;
        table[i] = table[i] << 8;
        table[i] += i;
        val = 0;
    }
}

void save_attr_registers(unsigned char table[NUM_ATTR_REGS * 2])
{
    uint8_t i, idx;
    uint8_t val;
    inb(0x03DA);
    for (i = 0x00; i < NUM_ATTR_REGS; i++)
    {
        idx = inb(0x03C0);
        outb((uint8_t)(0x20 | i), 0x03C0);
        val = inb(0x03C1);
        outb(val, 0x03C0);
        table[2 * i + 1] = val;
        table[2 * i] = i;
        val = 0;
    }
}

void save_graphics_registers(unsigned short table[NUM_GRAPHICS_REGS])
{
    int i;
    uint8_t val;
    inb(0x3CE);
    for (i = 0; i < NUM_GRAPHICS_REGS; i++)
    {
        outb(i, 0x03CE);
        val = inb(0x03CF);
        table[i] = val;
        table[i] = table[i] << 8;
        table[i] += i;
        val = 0;
    }
}

void save_palette(unsigned char palette_RGB[32][3])
{
    int i;
    inb(0x03C8);
    inb(0x03C9);
    outb(0x00, 0x03C8);
    for (i = 0; i < 32; i++)
    {
        palette_RGB[i][0] = inb(0x03C9);
        palette_RGB[i][1] = inb(0x03C9);
        palette_RGB[i][2] = inb(0x03C9);
    }
}

void fill_palette_text()
{
    /* Start writing at color 0. */
    outb(0x00, 0x03C8);

    /* Write all 32 colors from array. */
    rep_outsb(0x03C9, text_palette_RGB, 32 * 3);
}

void fill_palette_mode_x()
{
    /* Start writing at color 0. */
    outb(0x00, 0x03C8);

    /* Write all 64 colors from array. */
    rep_outsb(0x03C9, mode_x_palette_RGB, 64 * 3);
}

void write_font_data()
{
    int i;                /* loop index over characters                   */
    int j;                /* loop index over font bytes within characters */
    unsigned char *fonts; /* pointer into video memory                    */

    /* Prepare VGA to write font data into video memory. */
    outw(0x0402, 0x03C4);
    outw(0x0704, 0x03C4);
    outw(0x0005, 0x03CE);
    outw(0x0406, 0x03CE);
    outw(0x0204, 0x03CE);

    /* Copy font data from array into video memory. */
    for (i = 0, fonts = (unsigned char *)MODE_X_VMEM_ADDR; i < 256; i++)
    {
        for (j = 0; j < 16; j++)
            fonts[j] = font_data[i][j];
        fonts += 32; /* skip 16 bytes between characters */
    }

    /* Prepare VGA for text mode. */
    outw(0x0302, 0x03C4);
    outw(0x0304, 0x03C4);
    outw(0x1005, 0x03CE);
    outw(0x0E06, 0x03CE);
    outw(0x0004, 0x03CE);
}

void enable_text_mode()
{
    VGA_blank(1);                           /* blank the screen      */
    set_seq_regs_and_reset(text_seq, 0x63); /* sequencer registers   */
    set_CRTC_registers(text_CRTC);          /* CRT control registers */
    set_attr_registers(text_attr);          /* attribute registers   */
    set_graphics_registers(text_graphics);  /* graphics registers    */
    fill_palette_text();                    /* palette colors        */
    memset((void *)VIDEO, 0, 80 * 25 * 2);
    VGA_blank(0);
    return;
}

void enable_mode_x()
{
    set_mode_x_paging();
    VGA_blank(1);                             /* blank the screen      */
    set_seq_regs_and_reset(mode_X_seq, 0x63); /* sequencer registers   */
    set_CRTC_registers(mode_X_CRTC);          /* CRT control registers */
    set_attr_registers(mode_X_attr);          /* attribute registers   */
    set_graphics_registers(mode_X_graphics);  /* graphics registers    */
    fill_palette_mode_x();                    /* palette colors        */
    clear_modex();                            /* zero video memory     */
    VGA_blank(0);
    outw((MODE_X_VMEM_ADDR & 0xFF00) | 0x0C, 0x03D4);
    outw(((MODE_X_VMEM_ADDR & 0x00FF) << 8) | 0x0D, 0x03D4);
    return;
}
