#include "idt.h"

void uni() {printf("exception!\n"); while(1);}

void set_IDT() {
    int i;
    for(i = 0; i < 0x14; i++) {
        SET_IDT_INTR_ENTRY(idt[i], uni, KERNEL_DS, 3);
    }

    lidt(idt_desc_ptr);
}