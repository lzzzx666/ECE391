#include "types.h"

#define PAGE_DIR_ENTRY_NUM 1024
#define PAGE_TABLE_ENTRY_NUM 1024
#define INV_PD_IDX -1
#define INV_PT_IDX -1

typedef struct PDE
{
    union
    {
        uint32_t val;
        struct
        {
            uint8_t p : 1;
            uint8_t rw : 1;
            uint8_t us : 1;
            uint8_t pwt : 1;
            uint8_t pcd : 1;
            uint8_t a : 1;
            uint8_t reserved : 1;
            uint8_t ps : 1;
            uint8_t g : 1;
            uint8_t avail : 3;
            uint32_t pageTableBaseAddr : 20;
        };
    } __attribute__((packed));
} PDE_t;

typedef struct PTE
{
    union
    {
        uint32_t val;
        struct
        {
            uint8_t p : 1;
            uint8_t rw : 1;
            uint8_t us : 1;
            uint8_t pwt : 1;
            uint8_t pcd : 1;
            uint8_t a : 1;
            uint8_t d : 1;
            uint8_t pat : 1;
            uint8_t g : 1;
            uint8_t avail : 3;
            uint32_t pageBaseAddr : 20;
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

typedef PDE_t PD_t[PAGE_DIR_ENTRY_NUM];
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
