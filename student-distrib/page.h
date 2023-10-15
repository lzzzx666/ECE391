// #include "types.h"

// #define PAGE_DIR_ENTRY_NUM 1024
// #define PAGE_TABLE_ENTRY_NUM 1024

// typedef struct PDE
// {
//     union
//     {
//         uint32_t val;
//         struct
//         {
//             uint8_t p : 1;
//             uint8_t rw : 1;
//             uint8_t us : 1;
//             uint8_t pwt : 1;
//             uint8_t pcd : 1;
//             uint8_t a : 1;
//             uint8_t reserved : 1;
//             uint8_t ps : 1;
//             uint8_t g : 1;
//             uint8_t avail : 3;
//             uint32_t pageTableBaseAddr : 20;
//         } __attribute__((packed));
//     };
// } PDE_t;

// typedef struct PTE
// {
//     union
//     {
//         uint32_t val;
//         struct
//         {
//             uint8_t p : 1;
//             uint8_t rw : 1;
//             uint8_t us : 1;
//             uint8_t pwt : 1;
//             uint8_t pcd : 1;
//             uint8_t a : 1;
//             uint8_t d : 1;
//             uint8_t pat : 1;
//             uint8_t g : 1;
//             uint8_t avail : 3;
//             uint32_t pageTableBaseAddr : 20;
//         } __attribute__((packed));
//     };
// } PTE_t;

// PDE_t pageDirectory[PAGE_DIR_ENTRY_NUM];
// PTE_t pageTable[PAGE_TABLE_ENTRY_NUM];

// extern void page_init();

// #define SET_PDE()
