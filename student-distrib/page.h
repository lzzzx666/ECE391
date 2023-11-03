#ifndef PAGE_H
#define PAGE_H
#include "types.h"
#include "systemcall.h"

/*the starting address of kernel code, which is 4MB*/
#define KERNEL_START_ADDR (4 * 1024 * 1024) // 4MB = 4 * 2^10 * 2^10 = 4*1024*1024
#define PAGE_DIR_ENTRY_NUM 1024
#define PAGE_TABLE_ENTRY_NUM 1024
#define INV_PD_IDX -1
#define INV_PT_IDX -1
#define ONE_PROGRAM_SIZE KERNEL_START_ADDR
#define PROGRAM_START_ADDRESS 2*ONE_PROGRAM_SIZE

// global variables to store the status of control registers
extern uint32_t cr0, cr2, cr3, cr4;

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
        uint32_t val; // The 32-bit value of the Page Directory Entry.
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
    union // using union to provide accessbility to the single bit
    {
        uint32_t val; // The 32-bit value of the Page Table Entry.
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

/**
 * CR4 (Control Register 4) structure.
 *
 * CR4 is a control register in x86 architecture that controls various processor
 * features and modes. This structure defines the individual bits and fields of CR4.
 */
typedef struct cr4 {
    union {
        uint32_t val;         // 32-bit value of the CR4 register.
        struct {
            uint8_t vme : 1;       // Virtual 8086 Mode Extensions.
            uint8_t pvi : 1;       // Protected-Mode Virtual Interrupts.
            uint8_t tsd : 1;       // Time Stamp Disable.
            uint8_t de : 1;        // Debugging Extensions.
            uint8_t pse : 1;       // Page Size Extensions.
            uint8_t pae : 1;       // Physical Address Extension.
            uint8_t mce : 1;       // Machine-Check Exception.
            uint8_t pge : 1;       // Page Global Enable.
            uint8_t pce : 1;       // Performance-Monitoring Counter Enable.
            uint8_t osfxsr : 1;    // Operating System Support for FXSAVE/FXRSTOR instructions.
            uint8_t osxmmexcpt : 1; // Operating System Support for Unmasked SIMD Floating-Point Exceptions.
            uint32_t reserverd : 21; // Reserved bits.
        } __attribute__((packed));
    };
} cr4_t;

/**
 * CR3 (Control Register 3) structure.
 *
 * CR3 is a control register in x86 architecture that holds the physical address of the
 * base of the page directory for a task. This structure defines the individual bits and fields
 * of CR3.
 */
typedef struct cr3 {
    union {
        uint32_t val;                 // 32-bit value of the CR3 register.
        struct {
            uint8_t reserved2 : 3;     // Reserved bits.
            uint8_t pwt : 1;           // Page-level Write-Through.
            uint8_t pcd : 1;           // Page-level Cache Disable.
            uint8_t reserved1 : 7;     // Reserved bits.
            uint32_t pageDirectoryBase : 20; // Physical base address of the page directory.
        } __attribute__((packed));
    };
} cr3_t;

/**
 * CR2 (Control Register 2) structure.
 *
 * CR2 is a control register in x86 architecture that holds the linear address of the last
 * page-table entry that caused a page fault. This structure simply contains a 32-bit value.
 */
typedef struct cr2 {
    uint32_t val; ///< 32-bit value of the CR2 register.
} cr2_t;

/**
 * CR0 (Control Register 0) structure.
 *
 * CR0 is a control register in x86 architecture that controls various processor operating
 * modes and features. This structure defines the individual bits and fields of CR0.
 */
typedef struct cr0 {
    union {
        uint32_t val;         // 32-bit value of the CR0 register.
        struct {
            uint8_t pe : 1;        // Protection Enable.
            uint8_t mp : 1;        // Monitor Coprocessor.
            uint8_t em : 1;        // Emulation.
            uint8_t ts : 1;        // Task Switched.
            uint8_t et : 1;        // Extension Type.
            uint8_t ne : 1;        // Numeric Error.
            uint16_t reserved1 : 10; // Reserved bits.
            uint8_t wp : 1;        // Write Protect.
            uint8_t reserved2 : 1; // Reserved bit.
            uint8_t am : 1;        // Alignment Mask.
            uint16_t reserved3 : 10; // Reserved bits.
            uint8_t nw : 1;        // Not Write-Through.
            uint8_t cd : 1;        // Cache Disable.
            uint8_t pg : 1;        // Paging.
        } __attribute__((packed));
    };
} cr0_t;

/**
 * CR (Control Registers) structure.
 *
 * This structure combines the control registers CR0, CR2, CR3, and CR4 into a single
 * data structure for easy management and access.
 */
typedef struct cr {
    cr0_t cr0; // Control Register 0.
    cr2_t cr2; // Control Register 2.
    cr3_t cr3; // Control Register 3.
    cr4_t cr4; // Control Register 4.
} cr_t;

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
extern int set_pte(PT_t *baseAddr, int idx, uint8_t us, uint32_t addr);

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
extern int set_pde(PD_t *baseAddr, int idx, uint8_t us, uint8_t g, uint8_t ps, uint32_t addr);

/**
 * Initialize the page tables and enable paging.
 *
 * This function initializes page tables and page directories, sets up the necessary page table
 * entries (PTEs) for video memory, and configures the control registers to enable paging.
 *
 * @return 0 on success.
 */
extern int page_init();

/**
 * @brief Update the Control Register CR3.
 *
 * This function updates the Control Register CR3 with the current Page Directory's physical address.
 * It involves calling the functions `get_cr` to obtain the CR3 value and `set_cr` to set the new CR3 value.
 * The update is necessary for enabling the changes in the page directory to take effect.
 */
extern void update_cr3();

/**
 * @brief Set up paging for a program in memory.
 *
 * This function configures paging to allow access to a program in memory.
 *
 * @param fd The program's file descriptor.
 *
 * @return 0 on success. This function sets up a Page Directory Entry (PDE) to map the program's memory into the virtual address space.
 */
extern int32_t set_paging(int32_t fd);

extern void set_cr();
extern void get_cr();

#endif
