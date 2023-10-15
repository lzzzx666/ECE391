#include "page.h"

void page_init()
{

    // asm volatile(
    //     "leal  pageDirectory, %%eax;"
    //     "movl  %%eax, %%cr3;"
    //     "movl  %%cr4, %%eax;"
    //     "movl  $0x00000010, %%eax;"
    //     "movl  %%eax, %%cr4;"
    //     "movl  %%cr0, %%eax;"
    //     "movl  $0x80000000, %%eax;"
    //     "movl  %%eax, %%cr0;"
    //     :
    //     : "r"(&pageDirectory)
    //     : "%eax");
    int i;
    for (i = 0; i < 1024; i++)
    {
        pageDirectory[i].val = 0;
        pageTable[i].val = 0;
    }

    // asm volatile(
    //     "pushl %eax\n"
    //     "movl %cr0, %eax\n"       // Load CR0 into EAX
    //     "orl $0x80000000, %eax\n" // Set the PG (Paging) bit
    //     "movl %eax, %cr0\n"       // Write EAX back to CR0
    //     "popl %eax\n"
    // );
}
