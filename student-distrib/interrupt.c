#include "interrupt.h"
#include "x86_desc.h"

void idt_init()
{
    int i;
    for (i = 0; i < 15; i++)
    {
        SET_IDT_ENTRY(idt[i], 0xFFFFFFFF);
    }
}
