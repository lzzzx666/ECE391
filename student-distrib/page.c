#include "page.h"
#include "lib.h"

/*the starting address of kernel code, which is 4MB*/
#define KERNEL_START_ADDR (4 * 1024 * 1024) // 4MB = 4 * 2^10 * 2^10 = 4*1024*1024
#define ADDR_single4MB 0x400000
#define VID_ADDRESS (0x8000000 + 3 * ADDR_single4MB)

/*page table for the video memory (first entry in page directory), 4kb size per page*/
PT_t pageTable __attribute__((aligned(1024 * 4))); // 4KB = 4 * 2^10 = 4*1024
PT_t video_pageTable __attribute__((aligned(1024 * 4)));
/*page directory,
pageDirectory[0] is the table for video memory, 4kb per page.
pageDirectory[1] is the table for kernel code, 4mb per page without redirection to pageTable.
*/
PD_t pageDirectory __attribute__((aligned(1024 * 4)));

int set_pte(PT_t *baseAddr, int idx, uint8_t us, uint32_t addr)
{
    if (idx >= PAGE_TABLE_ENTRY_NUM) // sanity check
    {
        printf("invalid page table index:%d", idx);
        return INV_PT_IDX;
    }
    /*default value, present and rw enabled*/
    (*baseAddr)[idx].p = 1;
    (*baseAddr)[idx].rw = 1;
    /*set value*/
    (*baseAddr)[idx].us = us;
    (*baseAddr)[idx].pageBaseAddr = addr;
    return 0;
}

int set_pde(PD_t *baseAddr, int idx, uint8_t us, uint8_t g, uint8_t ps, uint32_t addr)
{
    if (idx >= PAGE_DIR_ENTRY_NUM) // sanity check
    {
        printf("invalid page directory index:%d", idx);
        return INV_PD_IDX;
    }
    /*default value, present and rw enabled*/
    (*baseAddr)[idx].p = 1;
    (*baseAddr)[idx].rw = 1;
    /*set value*/
    (*baseAddr)[idx].us = us;
    (*baseAddr)[idx].g = g;
    (*baseAddr)[idx].ps = ps;
    (*baseAddr)[idx].pageTableBaseAddr = addr;
    return 0;
}

int page_init()
{
    int i; // loop counter
    /*initialize page table and page directory*/
    for (i = 0; i < PAGE_TABLE_ENTRY_NUM; i++)
    {
        pageTable[i].val = 0;
    }
    for (i = 0; i < PAGE_DIR_ENTRY_NUM; i++)
    {
        pageDirectory[i].val = 0;
    }

    /*set page table for page table. To avoid access to video memory
    failed, set the map to its original address*/
    set_pte(&pageTable, VIDEO >> 12, 0, VIDEO >> 12);
    set_pte(&pageTable,VIDEO_TERMINAL1>>12,0,VIDEO_TERMINAL1>>12);
    set_pte(&pageTable,VIDEO_TERMINAL2>>12,0,VIDEO_TERMINAL2>>12);
    set_pte(&pageTable,VIDEO_TERMINAL3>>12,0,VIDEO_TERMINAL3>>12);

    /*set page directory entry 0 and 1. 0 for page table
     occupied by video memory, 1 for kernel code    */
    set_pde(&pageDirectory, 0, 0, 0, 0, ((uint32_t)&pageTable) >> 12); // 0 entry shold be user and 4kb
    set_pde(&pageDirectory, 1, 0, 1, 1, KERNEL_START_ADDR >> 12);      // 1st entry should be supervisior and 4mb

    /*enable paging function*/
    get_cr();                                         // get the control registers values into corresponding global variables
    ((cr4_t *)&cr4)->pse = 1;                         // turn on pse feature (allow 4MB page)
    ((cr3_t *)&cr3)->val = (uint32_t)(pageDirectory); // set cr3 to the pageDirectory address
    ((cr0_t *)&cr0)->pg = 1;                          // turn on the paging feature
    set_cr();                                         // set the control registers to the corresponding value

    return 0;
}

void update_cr3()
{
    get_cr(); // Get the current CR3 value.
    set_cr(); // Set the new CR3 value.
}

int32_t set_paging(int32_t fd)
{
    int32_t program_address = fd * ONE_PROGRAM_SIZE + PROGRAM_START_ADDRESS;

    // Set up the Page Directory Entry (PDE) to map the program's memory.
    set_pde(&pageDirectory, PROGRAM_IMAGE >> 22, 1, 0, 1, program_address >> 12);

    // Update the control register CR3.
    update_cr3();

    return 0; // Return 0 on success.
}

int32_t set_vidmap_paging(uint8_t **screen_start)
{
    set_pde(&pageDirectory, (VID_ADDRESS >> 22) & 0x3FF, 1, 0, 0, (((uint32_t)&video_pageTable) >> 12));
    update_cr3();
    set_pte(&video_pageTable, (VID_ADDRESS >> 12) & 0x3FF, 1, VIDEO >> 12);
    *screen_start = (uint8_t *)(VID_ADDRESS);
    return 0;
}
void update_vidmap(int32_t tid)
{
    if(tid<0 || tid>=TERMINAL_NUMBER) return;

    /*The case that the program is running in the current terminal*/
    if (current_terminal == tid)
    {
       set_pte(&video_pageTable,( VID_ADDRESS >> 12) &0x3ff, 1, VIDEO>>12);
    }
    /*The case that the program is running in the background terminal*/
    else
    {
        set_pte(&video_pageTable, (VID_ADDRESS >> 12)&0x3ff , 1, ((uint32_t)(main_terminal[tid].video_mem_backup))>>12);
    }
    update_cr3();
}
