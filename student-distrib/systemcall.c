#include "systemcall.h"
#include "page.h"
#include "pcb.h"
#include "keyboard.h"
#define DEBUG 0
const int8_t executableMagicStr[4] = {0x7f, 0x45, 0x4c, 0x46};
int32_t retVal;
/**
 * sys_halt
 * INPUT: temporarily unkown
 * OUTPUT: print the system call message
 */

int32_t
halt(uint8_t status)
{
    int i;
    pcb_t *cur_pcb = get_current_pcb();
    if (current_pid == 0)
    {
        delete_pcb();
        execute((const uint8_t *)"shell");
    }

    /* Close all file descriptors */
    for (i = 0; i < MAX_FILE_NUM; i++)
    {
        if (cur_pcb->file_obj_table[i].exist)
        {
            (cur_pcb->file_obj_table[i].f_operation.close)(i);
            cur_pcb->file_obj_table[i].exist = 0;
        }
    }

    // /* Set paging for parent process */
    set_paging(cur_pcb->parent_pid);
    tss.ss0 = KERNEL_DS;
    tss.esp0 = KERNAL_BOTTOM - cur_pcb->parent_pid * TASK_STACK_SIZE - 4;
    delete_pcb();
    retVal = status;
    asm volatile("movl %0, %%ebp \n\t"
                 "movl %1, %%esp \n\t"
                 "leave          \n\t"
                 "ret"
                 : /* no output */
                 : "r"(cur_pcb->parent_ebp),
                   "r"(cur_pcb->parent_esp)
                 : "ebp", "esp");
    return status;
}

/**
 * execute
 *  Based on the command, it will execute the corresponding executable file
 * INPUT: commond: the executable file
 * OUTPUT: ?
 */
int32_t execute(const uint8_t *command)
{
    int32_t i;
    uint8_t name_buf[MAX_FILE_NAME] = {'\0'};
    uint8_t args[MAX_BUF - MAX_FILE_NAME] = {'\0'};
    uint8_t test_buf[EXECUTAVLE_MAGIC_NUMBER_SIZE];
    dentry_t dentry;
    int32_t pcb_index;
    pcb_t *cur_pcb = NULL;
    pcb_t *new_pcb = NULL;
    int32_t eip, eflags, esp;

    /*check if it is the first process*/
    update_current_pid();
    if (current_pid < 0 || current_pid >= MAX_TASK)
    {
        cur_pcb = NULL;
    }
    else if (pcb_bitmap >> ((7 - current_pid) & 0x1) == TASK_NONEXIST)
    {
        cur_pcb = NULL;
    }
    cur_pcb = get_current_pcb();
    /*sanity check*/
    if (command == NULL)
    {
        return -1;
    }

    /*get the file name*/
    i = 0;
    while (command[i] != '\0')
    {
        name_buf[i] = command[i];
        i++;
        /*check if file name is so long*/
        if (i >= MAX_FILE_NAME)
        {
            return -1;
        }
    }

    /*check if the file exists*/
    if (read_dentry_by_name(name_buf, &dentry) == FS_FAIL)
    {
        return -1;
    }
    /*check if it is a regular file*/
    if (dentry.fileType != REGULAR_FILE)
    {
        return -1;
    }
    /*check if the file can be executed*/
    if (read_data(dentry.inodeIdx, 0, test_buf, 4) == FS_FAIL)
    {
        return -1;
    }
    if (strncmp((const int8_t *)test_buf, executableMagicStr, 4) != 0)
    {
        return -1;
    }
    /*get other arguments*/
    for (i = MAX_FILE_NAME; i < MAX_BUF; i++)
    {
        args[i - MAX_FILE_NAME] = command[i];
    }
    /*check if a new pcb can be created*/
    pcb_index = create_pcb();
    if (pcb_index == -1)
    {
        return -1;
    }

    /*store arguments*/
    new_pcb = pcb_array[pcb_index];
    for (i = MAX_FILE_NAME; i < MAX_BUF; i++)
    {
        new_pcb->arguments[i] = args[i];
    }
    /*start paging*/
    set_paging(pcb_index);

    /*load into memory*/
    read_data(dentry.inodeIdx, 0, (uint8_t *)PROGRAM_IMAGE, PROGRAM_IMAGE_SIZE);

    /*get eip*/
    read_data(dentry.inodeIdx, 24, (uint8_t *)&eip, 4);
    current_pid = pcb_index;
    esp = PROGRAM_IMAGE_END - 4;
    asm volatile(
        "pushfl\n\t"
        "popl %0\n\t"
        :
        : "r"(eflags));

    /*go to user space*/
    to_user_mode(eip, eflags, esp, pcb_index);
    return retVal;
}
/**/
void to_user_mode(int32_t eip, int32_t eflags, int32_t esp, int32_t pid)
{

    /*change tss*/
    int32_t cs = USER_CS;
    int32_t ss = USER_DS;
    uint32_t cur_esp;
    uint32_t cur_ebp;
    tss.ss0 = KERNEL_DS;
    tss.esp0 = KERNAL_BOTTOM - pid * TASK_STACK_SIZE - 4;
    asm volatile("mov %%esp, %0"
                 : "=r"(cur_esp));
    asm volatile("mov %%ebp, %0"
                 : "=r"(cur_ebp));
    pcb_array[current_pid]->parent_esp = cur_esp;
    pcb_array[current_pid]->parent_ebp = cur_ebp;
    asm volatile(
        "pushl %3\n\t"
        "pushl %2\n\t"
        "pushfl\n\t"
        "pushl %1\n\t"
        "pushl %0\n\t"
        "iret" ::"r"(eip),
        "r"(cs), "r"(esp), "r"(ss)
        : "memory");
}
/**
 * sys_read
 * INPUT: temporarily unkown
 * OUTPUT: print the system call message
 */
int32_t read(int32_t fd, void *buf, int32_t nbytes)
{
    pcb_t *cur_pcb = get_current_pcb();
    int32_t read_bytes;
    if (fd == 1)
    {
        return SYSCALL_FAIL;
    }
    /*sanity check*/
    if (nbytes <= 0 || fd >= MAX_FD || buf == NULL || cur_pcb->file_obj_table[fd].exist == 0 || fd < 0)
    {
#if DEBUG
        printf("Can't read!\n");
#endif
        return SYSCALL_FAIL;
    }

    /*read data*/

    read_bytes = (cur_pcb->file_obj_table[fd].f_operation.read)(fd, (int32_t *)buf, nbytes);
    cur_pcb->file_obj_table[fd].f_position += read_bytes;
    return read_bytes;
}

/**
 * sys_write
 * INPUT: temporarily unkown
 * OUTPUT: print the system call message
 */
int32_t write(int32_t fd, const void *buf, int32_t nbytes)
{

    pcb_t *cur_pcb = get_current_pcb();
    int32_t write_bytes;
    if (fd == 0)
    {
        return SYSCALL_FAIL;
    }
    /*sanity check*/

    if (nbytes <= 0 || fd >= MAX_FD || buf == NULL || cur_pcb->file_obj_table[fd].exist == 0 || fd < 0)
    {
#if DEBUG
        printf("Can't write!\n");
#endif
        return SYSCALL_FAIL;
    }

    /*write data*/
    write_bytes = (cur_pcb->file_obj_table[fd].f_operation.write)(fd, buf, nbytes);

    if (write_bytes == FS_FAIL)
    {
#if DEBUG

        printf("Can't write!\n");
#endif
        return SYSCALL_FAIL;
    }

    return write_bytes;
}

/**
 * sys_open
 * INPUT: temporarily unkown
 * OUTPUT: print the system call message
 */
int32_t open(const uint8_t *filename)
{

    int32_t fd;
    dentry_t dentry;
    file_object_t current_file;
    pcb_t *cur_pcb = get_current_pcb();

    /*find the file in the file system*/
    if (read_dentry_by_name(filename, &dentry) == FS_FAIL)
    {
        return SYSCALL_FAIL;
    }

    /*sanity check*/
    if (file_open(filename) == FS_FAIL)
    {
#if DEBUG
        printf("Can't find this file!\n");
#endif
        return SYSCALL_FAIL;
    }
    if (cur_pcb->f_number >= MAX_FD)
    {
#if DEBUG
        printf("Can't open more file!\n");
#endif
        return SYSCALL_FAIL;
    }

    /*initialize the file object*/
    initialize_file_object(&current_file, dentry);

    /*the case when initialization fail*/
    if (current_file.exist == 0)
    {
        return SYSCALL_FAIL;
    }

    /*assign this file to pcb*/
    fd = put_file_to_pcb(&current_file);

    /*open the file*/
    cur_pcb->file_obj_table[fd].f_operation.open(filename);

    return fd;
}

/**
 * sys_close
 * INPUT: temporarily unkown
 * OUTPUT: print the system call message
 */
int32_t close(int32_t fd)
{
    pcb_t *cur_pcb = get_current_pcb();

    /*get the active pcb*/
    cur_pcb = pcb_array[current_pid];

    /*sanity check*/
    if (fd < 2 || fd >= MAX_FD)
    { // we can't close terminal

        return SYSCALL_FAIL;
    }
    if (cur_pcb->file_obj_table[fd].exist == 0)
    {
        return SYSCALL_FAIL;
    }

    /*close the file*/
    cur_pcb->file_obj_table[fd].exist = 0;
    cur_pcb->f_number--;
    cur_pcb->file_obj_table[fd].f_operation.close(fd);

    return SYSCALL_SUCCESS;
}

/**
 * sys_getargs
 * INPUT: temporarily unkown
 * OUTPUT: print the system call message
 */
int32_t getargs(uint8_t *buf, int32_t nbytes)
{

    printf("sys_getargs!\n");
    return 0;
}

/**
 * sys_vidmap
 * INPUT: temporarily unkown
 * OUTPUT: print the system call message
 */
int32_t vidmap(uint8_t **screen_start)
{
    printf("sys_vidmap!\n");
    return 0;
}

/**
 * sys_sys_set_handler
 * INPUT: temporarily unkown
 * OUTPUT: print the system call message
 */
int32_t set_handler(int32_t signum, void *handler)
{
    printf("sys_set_handler!\n");
    return 0;
}

/**
 * sys_sigreturn
 * INPUT: temporarily unkown
 * OUTPUT: print the system call message
 */
int32_t sigreturn(void)
{
    printf("sys_sigreturn!\n");
    return 0;
}
