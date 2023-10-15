#include "types.h"

#define PAGE_DIR_ENTRY_NUM 1024
#define PAGE_TABLE_ENTRY_NUM 1024
#define INV_PD_IDX -1
#define INV_PT_IDX -1

/**
 * Page Directory Entry (PDE) structure.
 *
 * This struct represents a Page Directory Entry (PDE) used in memory management
 * and paging. It provides a set of bitfields for configuring various attributes
 * and the base address of a page table.
 */
typedef struct PDE
{
    union // using union to provide accessbility to the single bit
    {
        uint32_t val;                        // The 32-bit value of the Page Directory Entry.
        struct
        {
            uint8_t p : 1;                   // Present bit.
            uint8_t rw : 1;                  // Read/Write bit.
            uint8_t us : 1;                  // User/Supervisor bit.
            uint8_t pwt : 1;                 // Page-level Write-Through bit.
            uint8_t pcd : 1;                 // Page-level Cache Disable bit.
            uint8_t a : 1;                   // Accessed bit.
            uint8_t reserved : 1;            // Reserved bit.
            uint8_t ps : 1;                  // Page Size bit.
            uint8_t g : 1;                   // Global Page bit.
            uint8_t avail : 3;               // Available for use or reserved.
            uint32_t pageTableBaseAddr : 20; // Page Table Base Address (20 bits).
        };
    } __attribute__((packed));
} PDE_t;

/**
 * Page Table Entry (PTE) structure.
 *
 * This struct represents a Page Table Entry (PTE) used in memory management
 * and paging. It provides a set of bitfields for configuring various attributes
 * and the base address of a page.
 */
typedef struct PTE
{
    union                               // using union to provide accessbility to the single bit
    {
        uint32_t val;                   // The 32-bit value of the Page Table Entry.
        struct
        {
            uint8_t p : 1;              // Present bit.
            uint8_t rw : 1;             // Read/Write bit.
            uint8_t us : 1;             // User/Supervisor bit.
            uint8_t pwt : 1;            // Page-level Write-Through bit.
            uint8_t pcd : 1;            // Page-level Cache Disable bit.
            uint8_t a : 1;              // Accessed bit.
            uint8_t d : 1;              // Dirty bit.
            uint8_t pat : 1;            // Page Attribute Table bit.
            uint8_t g : 1;              // Global Page bit.
            uint8_t avail : 3;          // Available for use or reserved.
            uint32_t pageBaseAddr : 20; // Page Base Address (20 bits).
        } __attribute__((packed));
    };
} PTE_t;

typedef struct cr4
{
    union
    {
        uint32_t val;
        struct
        {
            uint8_t vme : 1;
            uint8_t pvi : 1;
            uint8_t tsd : 1;
            uint8_t de : 1;
            uint8_t pse : 1;
            uint8_t pae : 1;
            uint8_t mce : 1;
            uint8_t pge : 1;
            uint8_t pce : 1;
            uint8_t osfxsr : 1;
            uint8_t osxmmexcpt : 1;
            uint32_t reserverd : 21;

        } __attribute__((packed));
    };
} cr4_t;

typedef struct cr3
{
    union
    {
        uint32_t val;
        struct
        {
            uint8_t reserved2 : 3;
            uint8_t pwt : 1;
            uint8_t pcd : 1;
            uint8_t reserved1 : 7;
            uint32_t pageDirectoryBase : 20;

        } __attribute__((packed));
    };
} cr3_t;

typedef struct cr2
{
    uint32_t val;
} cr2_t;

typedef struct cr0
{
    union
    {
        uint32_t val;
        struct
        {
            uint8_t pe : 1;
            uint8_t mp : 1;
            uint8_t em : 1;
            uint8_t ts : 1;
            uint8_t et : 1;
            uint8_t ne : 1;
            uint16_t reserved1 : 10;
            uint8_t wp : 1;
            uint8_t reserved2 : 1;
            uint8_t am : 1;
            uint16_t reserved3 : 10;
            uint8_t nw : 1;
            uint8_t cd : 1;
            uint8_t pg : 1;
        } __attribute__((packed));
    };
} cr0_t;

/**
 * Page Directory (PD) type.
 *
 * This typedef represents an array of Page Directory Entries (PDEs) and is used
 * for managing page directories in memory management and paging.
 */
typedef PDE_t PD_t[PAGE_DIR_ENTRY_NUM];

/**
 * Page Table (PT) type.
 *
 * This typedef represents an array of Page Table Entries (PTEs) and is used
 * for managing page tables in memory management and paging.
 */
typedef PTE_t PT_t[PAGE_TABLE_ENTRY_NUM];

extern int set_pte(PT_t *baseAddr, int idx, uint8_t us, uint32_t addr);
extern int set_pde(PD_t *baseAddr, int idx, uint8_t us, uint8_t g, uint8_t ps, uint32_t addr);
extern void set_cr0(uint32_t regVal);
extern void set_cr2(uint32_t regVal);
extern void set_cr3(uint32_t regVal);
extern void set_cr4(uint32_t regVal);
extern void set_cr(uint32_t addr);
extern uint32_t get_cr0();
extern uint32_t get_cr2();
extern uint32_t get_cr3();
extern uint32_t get_cr4();
extern int page_init();

// #define SET_PDE()
