#ifndef FS_H
#define FS_H
#include "types.h"
#include "lib.h"
#define BLOCK_SIZE 4 * 1024 // block size in byte
#define FILE_NAME_MAX 32    // max length of file name
#define DENTRY_MAX 63       // max number of dentries
#define DATA_BLOCK_MAX ((BLOCK_SIZE / 4) - 1)
#define TERIMNAL_WIDTH 80 // max number of characters could be shown in one line

#define FS_SUCCEED 0
#define FS_FAIL -1

// file types
#define MOUSE 6
#define BEEPER 5
#define VGA 4
#define TERMINAL 3
#define REGULAR_FILE 2
#define DIRECTORY 1
#define RTC 0


// ioctl number
#define IOCTL_FILE_TYPE 1
#define IOCTL_FILE_SIZE 2
#define IOCTL_FILE_PTR 3

/** define global variables storing the file system information*/
uint32_t dentryNum;    // number of dentries
uint32_t inodeNum;     // number of inodes
uint32_t dataBlockNum; // number of data blocks

/**
 * Directory Entry structure (dentry_t).
 *
 * This structure represents a directory entry, which is used to store information
 * about a file or directory within a file system. It includes fields for the
 * file name, file type, inode index, and reserved space.
 */
typedef struct dentry
{
    uint8_t fileName[FILE_NAME_MAX]; // Array to store the file name.
    uint32_t fileType;               // Type of the file (e.g., regular file, directory).
    uint32_t inodeIdx;               // Index of the associated inode.
    uint32_t reserved[6];            // Reserved space for future use or alignment.
} dentry_t;

/**
 * Boot Block structure (bootBlock_t).
 *
 * The Boot Block structure represents essential information about a file system, including
 * the number of directory entries, inodes, data blocks, and an array of directory entries.
 * It also includes reserved space for future use or alignment.
 */
typedef struct bootBlock
{
    uint32_t dentryNum;            // Number of directory entries in the file system.
    uint32_t inodeNum;             // Number of inodes in the file system.
    uint32_t dataBlockNum;         // Number of data blocks in the file system.
    uint32_t reserved[13];         // Reserved space for future use or alignment.
    dentry_t dentries[DENTRY_MAX]; // Array of directory entries.
} bootBlock_t;

/**
 * Inode structure (inode_t).
 *
 * An Inode structure represents a file's metadata in a file system. It includes
 * information about the file's size in bytes and an array to store data block indices.
 */
typedef struct inode
{
    uint32_t size;                       // Length of the file in bytes.
    uint32_t dataBlocks[DATA_BLOCK_MAX]; // Array of data block indices.
} inode_t;

/**
 * Data Block structure (dataBlock_t).
 *
 * A Data Block structure represents a block of data within a file system. It contains
 * an array of bytes to store the actual data, and the size of this array is determined
 * by the BLOCK_SIZE constant.
 */
typedef struct dataBlock
{
    uint8_t data[BLOCK_SIZE]; // Array of bytes to store data, typically with a size of BLOCK_SIZE.
} dataBlock_t;

// file system base function

extern int32_t filesys_init(uint32_t filesys_img);
extern int32_t read_dentry_by_name(const uint8_t *fname, dentry_t *dentry);
extern int32_t read_dentry_by_index(uint32_t index, dentry_t *dentry);
extern int32_t read_data(uint32_t inode, uint32_t offset, uint8_t *buf, uint32_t length);

// fs system call interfaces for file operations

extern int32_t file_open(const uint8_t *fname);
extern int32_t file_close(int32_t fd);
extern int32_t file_read(int32_t fd, void *buf, uint32_t nbytes);
extern int32_t file_write(int32_t fd, const void *buf, int32_t nbytes);
extern int32_t file_ioctl(int32_t fd, int32_t request, void *buf);

// fs system call interfaces for direcotry operations

extern int32_t directory_open(const uint8_t *fname);
extern int32_t directory_close(int32_t fd);
extern int32_t directory_read(int32_t fd, uint8_t *buf, int32_t padding);
extern int32_t directory_write(int32_t fd, const void *buf, int32_t nbytes);
extern int32_t directory_ioctl(int32_t fd, int32_t request, void *buf);

// tests utilities

extern int32_t file_read_test(const int8_t *fname);
extern int32_t directory_read_test();

#endif
