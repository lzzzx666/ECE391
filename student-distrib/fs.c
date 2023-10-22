#include "fs.h"
#include "page.h"

bootBlock_t *bootBlock;
dentry_t *dentry;
inode_t *inodes;
dataBlock_t *dataBlock;

int32_t filesys_init(uint32_t filesys_img)
{
    bootBlock = (bootBlock_t *)filesys_img;
    inodes = (inode_t *)(bootBlock + 1);
    dataBlock = (dataBlock_t *)(inodes + bootBlock->inodeNum);
    return 0;
}

int32_t read_dentry_by_name(const uint8_t *fname, dentry_t *dentry)
{
    int i;
    uint32_t result;
    for (i = 0; i < bootBlock->dentryNum; i++)
    {
        dentry_t *curDentry = &(bootBlock->dentries[i]);
        if (strncmp((int8_t *)fname, (int8_t *)curDentry->fileName, FILE_NAME_MAX) == 0)
        {
            result = read_dentry_by_index(i, dentry);
            return result;
        }
    }
    return READ_FAIL;
}

int32_t read_dentry_by_index(uint32_t index, dentry_t *dentry)
{
    if (index >= (bootBlock->dentryNum))
    {
        printf("fail to read dentry with invalid index! \n");
        return READ_FAIL;
    }
    dentry_t *curDentry = &(bootBlock->dentries[index]);
    dentry->fileType = curDentry->fileType;
    dentry->inodeIdx = curDentry->inodeIdx;
    strncpy((int8_t *)dentry->fileName, (int8_t *)curDentry->fileName, FILE_NAME_MAX);
    return READ_SUCCESS;
}

int32_t read_data(uint32_t inodeIdx, uint32_t offset, uint8_t *buf, uint32_t length)
{
    // printf("inode size: %d\n", inodes[inodeIdx].size);
    if (inodeIdx >= bootBlock->inodeNum)
    {
        printf("fail to read inode with invalid index!\n");
        printf("max index: %d, request index: %d \n", bootBlock->inodeNum, inodeIdx);
        return READ_FAIL;
    }
    if (length > inodes[inodeIdx].size - offset)
    {
        printf("fail to read data with invalid length! \n");
        printf("file size: %d, request length: %d, offset: %d \n", inodes[inodeIdx].size, length, offset);
        return READ_FAIL;
    }
    if (length == 0)
        return READ_SUCCESS;
    uint8_t cache[length];
    uint32_t startBlock = offset >> 12;
    uint32_t startOffset = (offset & 0x0FFF);
    uint32_t endPos = offset + length;
    uint32_t endBlock = endPos >> 12;
    uint32_t endOffset = (endPos & 0x0FFF);
    // printf("length: %d, max: %d \n", length, (BLOCK_SIZE - startOffset));
    // printf("start offset:%d\n", startOffset);
    // printf("start block:%d\n", startBlock);
    // printf("end offset:%d\n", endOffset);
    // printf("end block:%d\n", endBlock);
    // printf("end pos:%d\n", endPos);
    uint32_t cacheOffset = 0;
    uint32_t curBlock = startBlock;
    uint32_t size;
    uint32_t *dataBlockIdxArr = (uint32_t *)&(inodes[inodeIdx].dataBlocks);
    uint32_t curBlockIdx;
    while (curBlock <= endBlock)
    {
        curBlockIdx = dataBlockIdxArr[curBlock];
        // printf("curBlockIdx: %d\n", curBlockIdx);
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
    return READ_SUCCESS;
}

void test_fs()
{
    clear();
    uint32_t dentryNum = (uint32_t)(bootBlock->dentryNum);
    int i;
    for (i = 0; i < dentryNum; i++)
    {
        dentry_t dentry = (bootBlock->dentries[i]);
        // printf("%s\n", dentry.fileName);
    }
    dentry_t dentry;

    if (read_dentry_by_name((const uint8_t *)"verylargetextwithverylongname.txt", &dentry) == READ_SUCCESS)
        printf("%s\n", dentry.fileName);

    uint8_t buf[inodes[dentry.inodeIdx].size + 1];
    buf[inodes[dentry.inodeIdx].size] = '\0';
    // printf("file name: %s \n", dentry.fileName);
    // printf("file type: %d \n", dentry.fileType);
    // printf("inode index: %d \n", dentry.inodeIdx);

    read_data(dentry.inodeIdx, 0, buf, inodes[dentry.inodeIdx].size);
    printf("%s", buf);
}
