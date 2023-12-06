#include "modex.h"

int32_t modex_read(int32_t fd, void *buf, int32_t nbytes)
{

    printf("trying to read to modex\n");
    return 0;
}

int32_t modex_write(int32_t fd, void *buf, int32_t nbytes)
{
    printf("trying to write to modex\n");
    return 0;
}

int32_t modex_open(const uint8_t *filename)
{

    printf("trying to open modex\n");
    return 0;
}

int32_t modex_close(int32_t fd)
{
    printf("trying to write to modex\n");
    return 0;
}

int32_t modex_ioctl(int32_t fd, int32_t request, void* buf)
{
    printf("trying to ioctl to modex\n");
    return 0;
}

void VGA_blank(int blank_bit)
{
    /*
     * Move blanking bit into position for VGA sequencer register
     * (index 1).
     */
    blank_bit = ((blank_bit & 1) << 5);

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
        : "g"(blank_bit)
        : "eax", "edx", "memory");
}

void clear_modex()
{
    /* Write to all four planes at once. */
    SET_WRITE_MASK(0x0F00);

    /* Set 64kB to zero (times four planes = 256kB). */
    memset((void*)MODEX_VMEM_ADDR, (int32_t)0, (uint32_t)MODEX_VMEM_SIZE);
}

