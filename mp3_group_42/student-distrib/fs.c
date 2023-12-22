#include "fs.h"
#include "page.h"
#include "pcb.h"
#include "systemcall.h"
#include "beeper.h"
#define DEBUG 0

/*file scope variables to store the pointers to bootBlock, inodes and dataBlock*/
bootBlock_t *bootBlock;
inode_t *inodesArr;
dataBlock_t *dataBlock;
uint32_t directoryIdx;

/**
 * int32_t filesys_init(uint32_t filesys_img)
 *
 * This function initializes the file system by processing a provided file system image.
 * It sets up various data structures, including the boot block, inodes, data blocks, and metadata.
 *
 * @param filesys_img - A pointer to the file system image.
 * @return must return FS_SUCCEED. File system initialization is forced to be successful
 */
int32_t filesys_init(uint32_t filesys_img)
{
    dentry_t *vga;
    bootBlock = (bootBlock_t *)filesys_img;                       // get boot block pointer
    inodesArr = (inode_t *)(bootBlock + 1);                       // get inodes pointer
    dataBlock = (dataBlock_t *)(inodesArr + bootBlock->inodeNum); // get data block pointer
    /*get file system information*/
    dentryNum = ++bootBlock->dentryNum;
    inodeNum = bootBlock->inodeNum;
    dataBlockNum = bootBlock->dataBlockNum;
    vga = &(bootBlock->dentries[dentryNum - 1]);
    strcpy((int8_t *)vga->fileName, (const int8_t *)"VGA");
    vga->fileType = VGA;

    dentry_t *beeper;
    dentryNum = ++bootBlock->dentryNum;
    inodeNum = bootBlock->inodeNum;
    dataBlockNum = bootBlock->dataBlockNum;
    beeper = &(bootBlock->dentries[dentryNum - 1]);
    strcpy((int8_t *)beeper->fileName, (const int8_t *)"beeper");
    beeper->fileType = BEEPER_FILETYPE;

    dentry_t *mouse;
    dentryNum = ++bootBlock->dentryNum;
    inodeNum = bootBlock->inodeNum;
    dataBlockNum = bootBlock->dataBlockNum;
    mouse = &(bootBlock->dentries[dentryNum - 1]);
    strcpy((int8_t *)mouse->fileName, (const int8_t *)"mouse");
    mouse->fileType = MOUSE;

    return FS_SUCCEED;
}

/**
 * int32_t read_dentry_by_name(const uint8_t *fname, dentry_t *dentry)
 *
 * This function searches for a directory entry with a specified filename in the file system's
 * directory entries. If a matching entry is found, it populates the provided 'dentry' structure
 * with the corresponding metadata information.
 *
 * @param fname - The filename to search for.
 * @param dentry - A pointer to a 'dentry_t' structure to store the directory entry information.
 * @return FS_SUCCEED if the directory entry is found and successfully retrieved, FS_FAIL otherwise.
 */
int32_t read_dentry_by_name(const uint8_t *fname, dentry_t *dentry)
{
    int i;
    uint32_t inputLen = strlen((const int8_t *)fname);
    uint8_t buf[FILE_NAME_MAX + 1];
    uint32_t result;
    buf[FILE_NAME_MAX] = '\0';
    for (i = 0; i < bootBlock->dentryNum; i++) // loop through all dentries to find target
    {
        memset(buf, 0, FILE_NAME_MAX + 1);
        dentry_t *curDentry = &(bootBlock->dentries[i]);
        memcpy(buf, curDentry->fileName, FILE_NAME_MAX);
        uint32_t nameLen = strlen((const int8_t *)buf);
        if ((inputLen == nameLen) && (strncmp((int8_t *)fname, (int8_t *)buf, nameLen) == 0)) // compare file name
        {
            result = read_dentry_by_index(i, dentry); // if names match, then get the dentry
            return result;
        }
    }
#if DEBUG
    printf("no such file with name: %s \n", fname);
#endif
    return FS_FAIL;
}

/**
 * int32_t read_dentry_by_index(uint32_t index, dentry_t *dentry)
 *
 * This function retrieves directory entry information based on the provided index.
 * It populates the provided 'dentry' structure with the metadata information of the directory entry.
 *
 * @param index - The index of the directory entry to retrieve.
 * @param dentry - A pointer to a 'dentry_t' structure to store the directory entry information.
 * @return FS_SUCCEED if the directory entry is found and successfully retrieved, FS_FAIL otherwise.
 */
int32_t read_dentry_by_index(uint32_t index, dentry_t *dentry)
{
    if (index >= (bootBlock->dentryNum)) // sanity check
    {
#if DEBUG
        printf("fail to read dentry with invalid index! \n");
#endif
        return FS_FAIL;
    }
    // populate dentry
    dentry_t *curDentry = &(bootBlock->dentries[index]);
    dentry->fileType = curDentry->fileType;
    dentry->inodeIdx = curDentry->inodeIdx;
    strncpy((int8_t *)dentry->fileName, (int8_t *)curDentry->fileName, FILE_NAME_MAX); // copy file name
    return FS_SUCCEED;
}

/**
 * int32_t read_data(uint32_t inodeIdx, uint32_t offset, uint8_t *buf, uint32_t length)
 *
 * This function reads data from a file's inode, starting at the specified offset,
 * and copies it into the provided buffer. The length parameter determines how much data to read.
 *
 * @param inodeIdx - The index of the inode representing the file.
 * @param offset - The starting offset from which to read the data.
 * @param buf - A pointer to the buffer where the data will be copied.
 * @param length - The length of data to read.
 * @return FS_SUCCEED if the data is successfully read and copied, FS_FAIL otherwise.
 */
int32_t read_data(uint32_t inodeIdx, uint32_t offset, uint8_t *buf, uint32_t length)
{

    if (inodeIdx >= bootBlock->inodeNum) // sanity check
    {
#if DEBUG
        printf("fail to read inode with invalid index!\n");
        printf("max index: %d, request index: %d \n", bootBlock->inodeNum, inodeIdx);
#endif
        return FS_FAIL;
    }

    if (length == 0 || offset >= inodesArr[inodeIdx].size) // sanity check
        return FS_SUCCEED;
    uint32_t readLen = length < (inodesArr[inodeIdx].size - offset) ? length : (inodesArr[inodeIdx].size - offset);
    // uint8_t data[readLen];                    // data buffer to store the data
    uint32_t startBlock = offset >> 12;       // equivalent ot offset / 4096
    uint32_t startOffset = (offset & 0x0FFF); // equivalent ot offset % 4096
    uint32_t endPos = offset + readLen;       // relative end position
    uint32_t endBlock = endPos >> 12;         // equivalent ot endPos / 4096
    uint32_t endOffset = (endPos & 0x0FFF);   // equivalent ot endPos % 4096

#if DEBUG // all of the information that may be help to debug
    printf("inode size: %d\n", inodesArr[inodeIdx].size);
    printf("length: %d, max: %d \n", length, (BLOCK_SIZE - startOffset));
    printf("start offset:%d\n", startOffset);
    printf("start block:%d\n", startBlock);
    printf("end offset:%d\n", endOffset);
    printf("end block:%d\n", endBlock);
    printf("end pos:%d\n", endPos);
#endif

    // initialize local variables
    uint32_t dataOffset = 0;
    uint32_t curBlock = startBlock;
    uint32_t size;
    uint32_t *dataBlockIdxArr = (uint32_t *)&(inodesArr[inodeIdx].dataBlocks);
    uint32_t curBlockIdx;
    memset(buf, 0, readLen);
    /*loop through data blocks to get the data and store into data*/
    while (curBlock <= endBlock)
    {
        curBlockIdx = dataBlockIdxArr[curBlock];
        if (curBlockIdx >= dataBlockNum) // sanity check
        {
#if DEBUG
            printf("fail to access datablock with invalid index!");
#endif
            return FS_FAIL;
        }
#if DEBUG
        printf("curBlockIdx: %d\n", curBlockIdx);
#endif
        if (curBlock == startBlock) // if it is the start block, only copy back part of it
        {
            size = readLen < (BLOCK_SIZE - startOffset) ? readLen : (BLOCK_SIZE - startOffset);
            memcpy(&(buf[dataOffset]), &(dataBlock[curBlockIdx].data[startOffset]), size);
            dataOffset += size;
        }
        else if (curBlock == endBlock) // if it is the end block, only copy front part of it
        {
            if (endOffset == 0)
                break;
            size = endOffset;
            memcpy(&(buf[dataOffset]), &(dataBlock[curBlockIdx].data[0]), size);
        }
        else // if it is the middle data block, copy all of it
        {
            memcpy(&(buf[dataOffset]), &(dataBlock[curBlockIdx].data[0]), BLOCK_SIZE);
            dataOffset += BLOCK_SIZE;
        }
        curBlock++;
    }
    // memset(buf, 0, sizeof(buf));
    // memcpy(buf, &data, readLen); // copy data from data into buffer
    return readLen;
}

/*** USELESS FOR CKPT2 ***/
/**
 * int32_t fopen(const uint8_t *fname)
 *
 * This function attempts to open a file by name and retrieve its directory entry information.
 *
 * @param fname - The name of the file to open.
 * @return FS_SUCCEED if the file is successfully opened and its directory entry is retrieved,
 *         FS_FAIL if the file is not found or an error occurs.
 */
int32_t file_open(const uint8_t *fname)
{
    dentry_t dentry;
    int32_t result;
    result = read_dentry_by_name(fname, &dentry); // check whether file could be open
    return result;
}

/*** USELESS FOR CKPT2 ***/
/**
 * int32_t fopen(const uint8_t *fname)
 *
 * This function is used to close a file identified by its file descriptor (fd).
 *
 * @param fd - The file descriptor of the file to close.
 * @return FS_SUCCEED if the file is successfully closed, an error code otherwise.
 */
int32_t file_close(int32_t fd)
{
    return FS_SUCCEED;
}

/**
 * int32_t fread(const uint8_t* fname, void *buf, uint32_t nbytes)
 *
 * This function reads data from a file identified by its name and copies it into the provided buffer.
 *
 * @param fname - The name of the file to read.
 * @param buf - A pointer to the buffer where the data will be copied.
 * @param nbytes - The number of bytes to read.
 * @return The number of bytes read from the file and copied into the buffer, or FS_FAIL if an error occurs.
 */
int32_t file_read(int32_t fd, void *buf, uint32_t nbytes)
{
    uint32_t fileSize;
    uint32_t readBytes;
    pcb_t *pcbPtr;
    pcbPtr = get_current_pcb();
    uint32_t inodeIdx = pcbPtr->file_obj_table[fd].inode;
    uint32_t offset = pcbPtr->file_obj_table[fd].f_position;
    fileSize = inodesArr[inodeIdx].size;
    if (offset >= fileSize)
    {
        return 0;
    }
    readBytes = read_data(inodeIdx, offset, buf, nbytes); // get the max bytes could be read or need to be read
    if (readBytes == FS_FAIL)                             // sanity check and read data
        return 0;
    return readBytes;
}

/*** USELESS FOR CKPT2 ***/
/**
 * int32_t fwrite(int32_t fd, const void *buf, int32_t nbytes)
 *
 * This function attempts to write data to a file identified by its file descriptor (fd).
 *
 * @param fd - The file descriptor of the file to which data will be written.
 * @param buf - A pointer to the data buffer containing the data to be written.
 * @param nbytes - The number of bytes to write.
 * @return FS_FAIL as the function is currently USELESS
 */
int32_t file_write(int32_t fd, const void *buf, int32_t nbytes)
{
#if DEBUG
    printf("This file system is read only!");
#endif
    return FS_FAIL;
}

// TODO comment
int32_t file_ioctl(int32_t fd, int32_t request, void *buf)
{
    pcb_t *curPcb = get_current_pcb();
    int32_t givenIdx = curPcb->file_obj_table[fd].inode;
    int32_t i;
    uint32_t dataBlockIdx = inodesArr[givenIdx].dataBlocks[0];
    switch (request)
    {
    case IOCTL_FILE_TYPE:
        for (i = 0; i < bootBlock->dentryNum; i++)
        {
            if (bootBlock->dentries[i].inodeIdx == givenIdx)
            {
                *((int32_t *)buf) = bootBlock->dentries[i].fileType;
                return SYSCALL_SUCCESS;
            }
        }
        return SYSCALL_FAIL;
    case IOCTL_FILE_SIZE:
        *((int32_t *)buf) = inodesArr[givenIdx].size;
        return SYSCALL_SUCCESS;
    case IOCTL_FILE_PTR:

        *((uint32_t *)buf) =(uint32_t)( &(dataBlock[dataBlockIdx]));
        return SYSCALL_SUCCESS;
    default:
        break;
    }
    return FS_FAIL;
}

/**
 * int32_t directory_open(const uint8_t *fname)
 *
 * This function opens a directory by name. Currently USELESS
 *
 * @param fname - The name of the directory to open.
 * @return FS_FAIL as the function is currently USELESS
 */
int32_t directory_open(const uint8_t *fname)
{
    directoryIdx = 0;
    return FS_SUCCEED;
}

/*** USELESS FOR CKPT2 ***/
/**
 * int32_t directory_close(int32_t fd)
 *
 * This function is called to close a directory that was previously opened.
 * It performs any necessary cleanup related to the directory file descriptor (fd).
 *
 * @param fd The file descriptor associated with the directory.
 * @return An integer indicating the status of the operation. FS_SUCCEED typically
 *         means the directory was successfully closed.
 */
int32_t directory_close(int32_t fd)
{
    directoryIdx = 0;
    return FS_SUCCEED;
}

/**
 * int32_t directory_read(uint32_t idx, uint8_t *buf)
 *
 * This function reads the name of a directory entry identified by its index and copies it into the provided buffer.
 *
 * @param idx - The index of the directory entry to read.
 * @param buf - A pointer to the buffer where the directory entry's name will be copied.
 * @return The length of the directory entry's name copied to the buffer, or FS_FAIL if an error occurs.
 */
int32_t directory_read(int32_t fd, uint8_t *buf, int32_t nbytes)
{
    dentry_t dentry;
    uint8_t data[MAX_FILE_NAME + 1];
    int32_t readLen;
    if (directoryIdx >= bootBlock->dentryNum)
        return 0;
    data[MAX_FILE_NAME] = '\0';
    if (read_dentry_by_index(directoryIdx, &dentry) == FS_FAIL)
        return 0;
    memcpy(data, dentry.fileName, FILE_NAME_MAX);
    readLen = strlen((const int8_t *)data);
    memcpy(buf, data, readLen);
    directoryIdx++;
    return readLen;
}

/*** USELESS FOR CKPT2 ***/
/**
 * int32_t directory_write(int32_t fd, const void *buf, int32_t nbytes)
 *
 * This function is intended for writing data to a directory entry, but it is currently USELESS
 * and returns an error code.
 *
 * @param fd - The file descriptor of the directory entry.
 * @param buf - A pointer to the data buffer containing the data to be written.
 * @param nbytes - The number of bytes to write.
 * @return FS_FAIL as the function is USELESS(read only)
 */
int32_t directory_write(int32_t fd, const void *buf, int32_t nbytes)
{
#if DEBUG
    printf("This file system is read only!");
#endif
    return FS_FAIL;
}

// TODO comment
int32_t directory_ioctl(int32_t fd, int32_t request, void *buf)
{
    pcb_t *curPcb = get_current_pcb();
    int32_t givenIdx = curPcb->file_obj_table[fd].inode;
    int32_t i;
    switch (request)
    {
    case IOCTL_FILE_TYPE:

        for (i = 0; i < bootBlock->dentryNum; i++)
        {
            if (bootBlock->dentries[i].inodeIdx == givenIdx)
            {
                *((int32_t *)buf) = bootBlock->dentries[i].fileType;
                return SYSCALL_SUCCESS;
            }
        }
        return SYSCALL_FAIL;
    case IOCTL_FILE_SIZE:

        *((int32_t *)buf) = inodesArr[givenIdx].size;
        return SYSCALL_SUCCESS;
    default:
        break;
    }
    return SYSCALL_FAIL;
}

/**
 * int32_t directory_read_test()
 *
 * This function reads and displays the contents of the directory, including file names, file types,
 * and file sizes. It clears the console screen and formats the output for readability.
 *
 * @return FS_SUCCEED if the directory contents are successfully read and displayed, FS_FAIL if any error occurs.
 */
int32_t directory_read_test()
{
    clear();
    /*
    clear the screen before print the files
    initialization
    */
    int i, j, k;
    uint32_t fileType;
    uint32_t fileSize;
    int32_t fileNameLen;
    dentry_t dentry;
    uint8_t fileName[FILE_NAME_MAX];
    for (i = 0; i < dentryNum; i++) // loop to get alll file information
    {
        memset(fileName, 0, FILE_NAME_MAX); // initialize data
        dentry = (bootBlock->dentries[i]);
        fileType = dentry.fileType;
        fileSize = inodesArr[dentry.inodeIdx].size;
        read_dentry_by_index(i, &dentry); // get file name
        fileNameLen = strlen((const int8_t *)dentry.fileName) < FILE_NAME_MAX ? strlen((const int8_t *)dentry.fileName) : FILE_NAME_MAX;

        /*print entry*/
        printf("file_name: ");
        j = 32 - fileNameLen;
        for (; j > 0; j--)
        {
            printf(" ");
        }
        putc_rep(dentry.fileName, fileNameLen);
        printf(", file_type: ");
        printf("%d", dentry.fileType);
        printf(", file_size: ");
        // right alignment
        j = !fileSize;
        for (k = fileSize; k; j++, k /= 10)
            ;
        for (; j < 9; j++)
        {
            printf(" ");
        }
        printf("%d\n", fileSize);
    }
    return FS_SUCCEED;
    // int32_t fd, cnt;
    // uint8_t buf[33];
    // current_pid = 0;
    // create_pcb();
    // if (-1 == (fd = open((const uint8_t *)".")))
    // {
    //     printf("directory open failed\n");
    //     return 2;
    // }

    // while (0 != (cnt = read(fd, buf, 32)))
    // {
    //     if (-1 == cnt)
    //     {
    //         printf("directory entry read failed\n");
    //         return 3;
    //     }
    //     buf[cnt] = '\n';
    //     if (-1 == write(1, buf, cnt + 1))
    //         return 3;
    // }
    // delete_pcb();
    // return 0;
}

/**
 * int32_t file_read_test(const int8_t *fname)
 *
 * This function reads the contents of a file specified by its name, prints the content to the console,
 * and clears the console screen.
 *
 * @param fname - The name of the file to read and print.
 * @return FS_SUCCEED if the file is successfully read and printed, FS_FAIL if any error occurs.
 */
int32_t file_read_test(const int8_t *fname)
{

    dentry_t dentry;
    uint32_t fileSize;
    int32_t readLen;
    if (read_dentry_by_name((const uint8_t *)fname, &dentry) == FS_FAIL) // sanity check
        return FS_FAIL;
    fileSize = inodesArr[dentry.inodeIdx].size;
    uint8_t buf[fileSize];
    readLen = read_data(dentry.inodeIdx, 0, buf, fileSize);
    if (readLen == FS_FAIL) // sanity check and read file
        return FS_FAIL;

    clear();
    putc_rep(buf, readLen); // print all data including 0x00
    printf("\n");
    return FS_SUCCEED;
}
