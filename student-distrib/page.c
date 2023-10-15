#include "page.h"
#include "lib.h"

/*the starting address of kernel code, which is 4MB*/
#define KERNEL_START_ADDR (4 * 1024 * 1024) // 4MB = 4 * 2^10 * 2^10 = 4*1024*1024

/*page table for the video memory (first entry in page directory), 4kb size per page*/
PT_t pageTable __attribute__((aligned(1024 * 4))); // 4KB = 4 * 2^10 = 4*1024

/*page directory,
pageDirectory[0] is the table for video memory, 4kb per page.
pageDirectory[1] is the table for kernel code, 4mb per page without redirection to pageTable.
*/
PD_t pageDirectory __attribute__((aligned(1024 * 4)));

/**
 * set_pte
 *
 * Sets the attributes and address for a specified Page Table Entry (PTE)
 * at the given index within a page table.
 *
 * @param baseAddr - Pointer to the base address of the page table.
 * @param idx - Index of the page table entry to be modified.
 * @param us - User/Supervisor flag (0 for supervisor, 1 for user).
 * @param addr - Physical address to be associated with the PTE.
 *
 * @return 0 on success. If the index is out of bounds, it returns INV_PT_IDX and
 * prints an error message.
 *
 * @note this function will not initialize the entry in the PD!!!
 */
int set_pte(PT_t *baseAddr, int idx, uint8_t us, uint32_t addr)
{
    if (idx >= PAGE_TABLE_ENTRY_NUM) // sanity check
    {
        printf("invalid page table index:%d", idx);
        return INV_PT_IDX;
    }
    /*default value*/
    (*baseAddr)[idx].p = 1;
    (*baseAddr)[idx].rw = 1;
    /*set value*/
    (*baseAddr)[idx].us = us;
    (*baseAddr)[idx].pageBaseAddr = addr;
    return 0;
}

/**
 * set_pde
 *
 * Sets the attributes and address for a specified Page Directory Entry (PDE)
 * at the given index within a page directory.
 *
 * @param baseAddr - Pointer to the base address of the page directory.
 * @param idx - Index of the page directory entry to be modified.
 * @param us - User/Supervisor flag (0 for supervisor, 1 for user).
 * @param g - Global page flag (0 for not global, 1 for global).
 * @param ps - Page size flag (0 for 4KB, 1 for 4MB page size).
 * @param addr - Physical address or page table base address to be associated with the PDE.
 *
 * @return 0 on success. If the index is out of bounds, it returns INV_PD_IDX and
 * prints an error message.
 */
int set_pde(PD_t *baseAddr, int idx, uint8_t us, uint8_t g, uint8_t ps, uint32_t addr)
{
    if (idx >= PAGE_DIR_ENTRY_NUM) // sanity check
    {
        printf("invalid page directory index:%d", idx);
        return INV_PD_IDX;
    }
    /*default value*/
    (*baseAddr)[idx].p = 1;
    (*baseAddr)[idx].rw = 1;
    /*set value*/
    (*baseAddr)[idx].us = us;
    (*baseAddr)[idx].g = g;
    (*baseAddr)[idx].ps = ps;
    (*baseAddr)[idx].pageTableBaseAddr = addr;
    return 0;
}


/**
 * Initialize the page tables and enable paging.
 *
 * This function initializes page tables and page directories, sets up the necessary page table
 * entries (PTEs) for video memory, and configures the control registers to enable paging.
 *
 * @return 0 on success.
 */
int page_init()
{
    int i;
    for (i = 0; i < PAGE_TABLE_ENTRY_NUM; i++)
    {
        pageTable[i].val = 0;
    }
    for (i = 0; i < PAGE_DIR_ENTRY_NUM; i++)
    {
        pageDirectory[i].val = 0;
    }

    set_pte(&pageTable, VIDEO >> 12, 0, VIDEO >> 12);
    set_pte(&pageTable, (VIDEO >> 12) + 1, 0, (VIDEO >> 12) + 1);
    set_pte(&pageTable, (VIDEO >> 12) + 2, 0, (VIDEO >> 12) + 2);
    set_pte(&pageTable, (VIDEO >> 12) + 3, 0, (VIDEO >> 12) + 3);
    set_pde(&pageDirectory, 0, 0, 0, 0, ((uint32_t)&pageTable) >> 12);
    set_pde(&pageDirectory, 1, 0, 1, 1, KERNEL_START_ADDR >> 12);
    (*set_cr)((uint32_t)(pageDirectory));
    return 0;
}
