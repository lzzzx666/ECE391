#include "fs.h"
#include "page.h"

#define DEBUG 0

/*file scope variables to store the pointers to bootBlock, inodes and dataBlock*/
bootBlock_t *bootBlock;
inode_t *inodes;
dataBlock_t *dataBlock;

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
    bootBlock = (bootBlock_t *)filesys_img;     //get boot block pointer
    inodes = (inode_t *)(bootBlock + 1);        //get inodes pointer
    dataBlock = (dataBlock_t *)(inodes + bootBlock->inodeNum);      //get data block pointer
    // get file system information
    dentryNum = bootBlock->dentryNum;
    inodeNum = bootBlock->inodeNum;
    dataBlockNum = bootBlock->inodeNum;
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
    uint32_t result;
    for (i = 0; i < bootBlock->dentryNum; i++)      //loop through all dentries to find target
    {
        dentry_t *curDentry = &(bootBlock->dentries[i]);
        if (strncmp((int8_t *)fname, (int8_t *)curDentry->fileName, FILE_NAME_MAX) == 0)    //compare file name
        {
            result = read_dentry_by_index(i, dentry);   //if names match, then get the dentry
            return result;
        }
    }
    printf("no such file with name: %s \n", fname);
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
    if (index >= (bootBlock->dentryNum))        //sanity check
    {
        printf("fail to read dentry with invalid index! \n");
        return FS_FAIL;
    }
    dentry_t *curDentry = &(bootBlock->dentries[index]);
    dentry->fileType = curDentry->fileType;
    dentry->inodeIdx = curDentry->inodeIdx;
    strncpy((int8_t *)dentry->fileName, (int8_t *)curDentry->fileName, FILE_NAME_MAX);
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

    if (inodeIdx >= bootBlock->inodeNum)    //sanity check
    {
        printf("fail to read inode with invalid index!\n");
        printf("max index: %d, request index: %d \n", bootBlock->inodeNum, inodeIdx);
        return FS_FAIL;
    }

    if (length > inodes[inodeIdx].size - offset)    //sanity check
    {
        printf("fail to read data with invalid length! \n");
        printf("file size: %d, request length: %d, offset: %d \n", inodes[inodeIdx].size, length, offset);
        return FS_FAIL;
    }

    if (length == 0)    //sanity check
        return FS_SUCCEED;

    uint8_t cache[length];
    uint32_t startBlock = offset >> 12;
    uint32_t startOffset = (offset & 0x0FFF);
    uint32_t endPos = offset + length;
    uint32_t endBlock = endPos >> 12;
    uint32_t endOffset = (endPos & 0x0FFF);

#if DEBUG
    printf("inode size: %d\n", inodes[inodeIdx].size);
    printf("length: %d, max: %d \n", length, (BLOCK_SIZE - startOffset));
    printf("start offset:%d\n", startOffset);
    printf("start block:%d\n", startBlock);
    printf("end offset:%d\n", endOffset);
    printf("end block:%d\n", endBlock);
    printf("end pos:%d\n", endPos);
#endif

    uint32_t cacheOffset = 0;
    uint32_t curBlock = startBlock;
    uint32_t size;
    uint32_t *dataBlockIdxArr = (uint32_t *)&(inodes[inodeIdx].dataBlocks);
    uint32_t curBlockIdx;
    while (curBlock <= endBlock)
    {
        curBlockIdx = dataBlockIdxArr[curBlock];
        if (curBlockIdx >= dataBlockNum)    //sanity check
        {
            printf("fail to access datablock with invalid index!");
            return FS_FAIL;
        }
#if DEBUG
        printf("curBlockIdx: %d\n", curBlockIdx);
#endif
        if (curBlock == startBlock)
        {
            size = length < (BLOCK_SIZE - startOffset) ? length : (BLOCK_SIZE - startOffset);
            memcpy(&(cache[cacheOffset]), &(dataBlock[curBlockIdx].data[startOffset]), size);
            cacheOffset += size;
        }
        else if (curBlock == endBlock)
        {
            if (endOffset == 0)
                break;
            size = endOffset;
            memcpy(&(cache[cacheOffset]), &(dataBlock[curBlockIdx].data[0]), size);
        }
        else
        {
            memcpy(&(cache[cacheOffset]), &(dataBlock[curBlockIdx].data[0]), BLOCK_SIZE);
            cacheOffset += BLOCK_SIZE;
        }
        curBlock++;
    }
    memcpy(buf, &cache, length);
    return FS_SUCCEED;
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
int32_t fopen(const uint8_t *fname)
{
    dentry_t dentry;
    int32_t result;
    result = read_dentry_by_name(fname, &dentry);
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
int32_t fclose(int32_t fd)
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
int32_t fread(const uint8_t* fname, void *buf, uint32_t nbytes)
{
    dentry_t dentry;
    uint32_t fileSize;
    uint32_t readBytes;
    if (read_dentry_by_name(fname, &dentry) == FS_FAIL)     //sanity check
        return FS_FAIL;
    fileSize = inodes[dentry.inodeIdx].size;
    readBytes = fileSize < nbytes ? fileSize : nbytes;
    if (read_data(dentry.inodeIdx, 0, buf, readBytes) == FS_FAIL)     //sanity check
        return FS_FAIL;
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
int32_t fwrite(int32_t fd, const void *buf, int32_t nbytes)
{
    return FS_FAIL;
}

/*** USELESS FOR CKPT2 ***/
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
    return FS_SUCCEED;
}

/*** USELESS FOR CKPT2 ***/
int32_t directory_close(int32_t fd)
{
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
int32_t directory_read(uint32_t idx, uint8_t *buf)
{
    uint32_t size = 0;
    uint32_t nameLen;
    dentry_t dentry;
    if (read_dentry_by_index(idx, &dentry) == FS_FAIL)     //sanity check
        return FS_FAIL;
    nameLen = strlen((int8_t *)dentry.fileName);
    size = nameLen < FILE_NAME_MAX ? nameLen : FILE_NAME_MAX;
    memcpy(buf, dentry.fileName, nameLen);
    return (int32_t)size;
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
    return FS_FAIL;
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
    int i, j, k;
    uint32_t fileType;
    uint32_t fileSize;
    int32_t fileNameLen;
    dentry_t dentry;
    uint8_t fileName[FILE_NAME_MAX];
    for (i = 0; i < dentryNum; i++)
    {
        memset(fileName, 0, FILE_NAME_MAX);
        dentry = (bootBlock->dentries[i]);
        fileType = dentry.fileType;
        fileSize = inodes[dentry.inodeIdx].size;
        fileNameLen = directory_read(i, fileName);
        printf("file_name: ");
        j = 32 - fileNameLen;
        for (; j > 0; j--)
        {
            printf(" ");
        }
        printc(fileName, fileNameLen);
        printf(", file_type: ");
        printf("%d", dentry.fileType);
        printf(", file_size: ");
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
    if (read_dentry_by_name((const uint8_t *)fname, &dentry) == FS_FAIL)     //sanity check
        return FS_FAIL;
    fileSize = inodes[dentry.inodeIdx].size;
    uint8_t buf[fileSize];
    if (fread((const uint8_t *)fname, buf, fileSize) == FS_FAIL)     //sanity check
        return FS_FAIL;
    clear();
    printc(buf, fileSize);
    printf("\n");
    return FS_SUCCEED;
}

