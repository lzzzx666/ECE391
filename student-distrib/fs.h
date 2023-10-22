#ifndef FS_H
#define FS_H
#include "types.h"
#include "lib.h"
#define BLOCK_SIZE 4 * 1024 // block size in byte
#define FILE_NAME_MAX 32
#define DENTRY_MAX 63
#define DATA_BLOCK_MAX ((BLOCK_SIZE / 4) - 1)

#define READ_SUCCESS 0
#define READ_FAIL -1

#define REGULAR_FILE 2
#define DIRECTORY 1
#define RTC_DEVICE 0

typedef struct dentry
{
    uint8_t fileName[FILE_NAME_MAX];
    uint32_t fileType;
    uint32_t inodeIdx;
    uint32_t reserved[6];
} dentry_t;

typedef struct bootBlock
{
    uint32_t dentryNum;
    uint32_t inodeNum;
    uint32_t dataBlockNum;
    uint32_t reserved[13];
    dentry_t dentries[DENTRY_MAX];
} bootBlock_t;

typedef struct inode
{
    uint32_t size; // length in Bytes
    uint32_t dataBlocks[DATA_BLOCK_MAX];
} inode_t;

typedef struct dataBlock
{
    uint8_t data[BLOCK_SIZE];
} dataBlock_t;

extern int32_t read_dentry_by_name(const uint8_t *fname, dentry_t *dentry);
extern int32_t read_dentry_by_index(uint32_t index, dentry_t *dentry);
extern int32_t read_data(uint32_t inode, uint32_t offset, uint8_t *buf, uint32_t length);
extern int32_t filesys_init(uint32_t filesys_img);

extern void test_fs();
#endif
