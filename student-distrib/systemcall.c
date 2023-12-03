#include "systemcall.h"
#include "page.h"
#include "pcb.h"
#include "keyboard.h"

#define EXCEPTION_TEST 0
#define ADDR_128MB 0x8000000
#define DEBUG 0
#define ADDR_4MB 0x400000
/*those are numbers used for executable file*/
const int8_t executableMagicStr[4] = {0x7f, 0x45, 0x4c, 0x46};

/*the return value for execute*/
int32_t retVal;

/*the arguments*/
uint8_t argv[MAX_TERMINAL_SIZE];

/**
 * @brief Halt the current process with a given status code.
 * This function halts the execution of the current process and performs various cleanup tasks.
 *
 * @param status The status code indicating the reason for halting.
 *
 * @return The status code provided as input, which is returned upon process termination.
 */
int32_t halt(uint8_t status)
{
    int i;
    pcb_t *cur_pcb = get_current_pcb();
    int32_t parent_ebp = cur_pcb->parent_ebp;
    int32_t parent_esp = cur_pcb->parent_esp;
    // If the current process is the initial shell (PID 0), restart the shell.
    if (current_pid < TERMINAL_NUMBER)
    {
        delete_pcb();
        execute((uint8_t*)"shell");
        // printf("This terminal becomes inactive.\n"); //now the terminal don't have inactive status
        // if (active_terminal == 1)
        // {
        //     active_terminal = 0;
        //     for (i = 0; i < TERMINAL_NUMBER; i++)
        //     {
        //         sche_array[i] = TERMINAL_UNINIT;
        //     }
        // }
        // else
        // {
        //     sche_array[current_terminal] = TERMINAL_CLOSE;
        //     active_terminal--;
        // }
        // schedule();
    }

    // Close all file descriptors associated with the current process.
    for (i = 0; i < MAX_FILE_NUM; i++)
    {
        if (cur_pcb->file_obj_table[i].exist)
        {
            // Call the close function associated with the file operation.
            (cur_pcb->file_obj_table[i].f_operation.close)(i);
            cur_pcb->file_obj_table[i].exist = 0; // Mark the file descriptor as closed.
        }
    }
    set_paging(cur_pcb->parent_pid); // Set paging for the parent process.
    delete_pcb();                    // Clean up the current PCB.

    tss.ss0 = KERNEL_DS;                                                  // Set the kernel stack segment.
    tss.esp0 = KERNAL_BOTTOM - cur_pcb->parent_pid * TASK_STACK_SIZE - 4; // Set the kernel stack pointer.

    retVal = status; // Set the return value for the process.

    // Perform a context switch by modifying the stack and instruction pointers.
    asm volatile(
        "movl %0, %%ebp \n\t"
        "movl %1, %%esp \n\t"
        "leave          \n\t"
        "ret"
        : /* no output */
        : "r"(parent_ebp),
          "r"(parent_esp)
        : "ebp", "esp");

    return status; // Return the status code, indicating the reason for process termination.
}

/**
 * @brief Execute a specified executable file.
 * This function executes the specified executable file by loading it into memory,
 * setting up the process control block (PCB), and transitioning to user mode.
 *
 * @param command The name of the executable file and its arguments.
 *
 * @return Return status:
 * - 0 on successful execution.
 * - -1 if an error occurs, such as file not found, file type mismatch, or other issues.
 */
int32_t execute(const uint8_t *command)
{
    int32_t i;
    uint8_t name_buf[MAX_FILE_NAME] = {'\0'};
    uint8_t test_buf[EXECUTABLE_MAGIC_NUMBER_SIZE];
    dentry_t dentry;
    int32_t pcb_index;
    // int32_t argvLen;
    pcb_t *cur_pcb = NULL;
    pcb_t *new_pcb = NULL;
    int32_t eip, eflags, esp;

    // Check if it is the first process and obtain the current PCB.
    update_current_pid();
    if (current_pid < 0 || current_pid >= MAX_TASK || (pcb_bitmap >> ((7 - current_pid) & 0x1)) == TASK_NONEXIST)
    {
        cur_pcb = NULL;
    }
    cur_pcb = get_current_pcb();

    // Sanity check for the command argument.
    if (command == NULL)
    {
        return -1;
    }

    // Get the file name and check if it is too long.
    i = 0;
    while (command[i] != ' ' && command[i] != '\0')
    {
        name_buf[i] = command[i];
        i++;
        if (i >= MAX_FILE_NAME)
        {
            return -1; // File name is too long.
        }
    } // now i points to the first space char
    memset(argv, '\0', MAX_BUF);
    while (i < strlen((int8_t *)command) && command[i] != '\0' && command[i] == ' ')
    {
        i++;
    } // now i points to the first non-space char after exe name
    int32_t j = 0;
    while (i < strlen((int8_t *)command) && command[i] != '\0')
    {
        argv[j++] = command[i++]; // copy the content between [arg1, arg2]
    }
    // Check if the file exists.
    if (read_dentry_by_name(name_buf, &dentry) == FS_FAIL)
    {
        return -1; // File not found.
    }

    // Check if it is a regular file.
    if (dentry.fileType != REGULAR_FILE)
    {
        return -1; // Not a regular file.
    }

    // Check if the file can be executed (magic number validation).
    if (read_data(dentry.inodeIdx, 0, test_buf, EXECUTABLE_MAGIC_NUMBER_SIZE) == FS_FAIL || strncmp((const int8_t *)test_buf, executableMagicStr, EXECUTABLE_MAGIC_NUMBER_SIZE) != 0)
    {
        return -1; // File cannot be executed.
    }

    // Get other arguments from the command.

    // Check if a new PCB can be created.
    if(strncmp((int8_t*)name_buf,(int8_t*)"shell",MAX_FILE_NAME)==0){
            pcb_index = create_pcb(1); //1 indicates that it is shell
    }else{
        pcb_index = create_pcb(0); //0 indicates it is not a shell
    }

    if (pcb_index == -1)
    {
        return -1; // PCB creation failed.
    }

    // Store arguments in the new PCB.
    new_pcb = pcb_array[pcb_index];
    memcpy(&(new_pcb->arguments), argv, MAX_BUF);

    // Start paging for the new process.
    set_paging(pcb_index);

    // Load the executable into memory.
    read_data(dentry.inodeIdx, 0, (uint8_t *)PROGRAM_IMAGE, PROGRAM_IMAGE_SIZE);

    // Get the entry point (EIP) from the executable.
    read_data(dentry.inodeIdx, 24, (uint8_t *)&eip, 4);
    current_pid = pcb_index;
    esp = PROGRAM_IMAGE_END - 4;

    // Get the flags (EFLAGS) and transition to user mode.
    asm volatile(
        "pushfl\n\t"
        "popl %0\n\t"
        :
        : "r"(eflags));

    to_user_mode(eip, eflags, esp, pcb_index); // Transition to user mode.

    return retVal; // Return the execution status.
}

/**
 * @brief Switch the processor to user mode.
 * This function changes the processor's state to user mode by modifying the
 * Task State Segment (TSS) and executing an interrupt return (IRET) instruction.
 * It sets the code segment (CS), data segment (SS), stack pointer (ESP), and
 * stack segment (SS0) to user mode values, allowing the execution of user-level code.
 *
 * @param eip The instruction pointer value for the user mode code.
 * @param eflags The flags register value for the user mode.
 * @param esp The user mode stack pointer.
 * @param pid The Process ID (PID) of the user process.
 */
void to_user_mode(int32_t eip, int32_t eflags, int32_t esp, int32_t pid)
{
    /* Change TSS */
    int32_t cs = USER_CS;
    int32_t ss = USER_DS;
    uint32_t cur_esp;
    uint32_t cur_ebp;

    tss.ss0 = KERNEL_DS;                                  // Set the stack segment for kernel mode.
    tss.esp0 = KERNAL_BOTTOM - pid * TASK_STACK_SIZE - 4; // Set the kernel stack pointer.

    // Get the current stack and base pointers.
    asm volatile("mov %%esp, %0"
                 : "=r"(cur_esp));
    asm volatile("mov %%ebp, %0"
                 : "=r"(cur_ebp));

    // Update the parent stack and base pointers in the PCB.
    pcb_array[current_pid]->parent_esp = cur_esp;
    pcb_array[current_pid]->parent_ebp = cur_ebp;
    sti();
    // Switch to user mode by pushing the necessary values onto the stack and executing IRET.
    asm volatile(
        "pushl %3\n\t"
        "pushl %2\n\t"
        "pushfl\n\t"
        "popl %%eax\n\t"
        "orl $0x200,%%eax\n\t"
        "pushl %%eax\n\t"
        "pushl %1\n\t"
        "pushl %0\n\t"
        "iret" ::"r"(eip),
        "r"(cs), "r"(esp), "r"(ss)
        : "memory");
}

/**
 * @brief Read data from a file descriptor.
 *
 * This function reads data from a file descriptor into the provided buffer.
 *
 * @param fd The file descriptor index.
 * @param buf A pointer to the buffer where the data will be stored.
 * @param nbytes The number of bytes to read.
 *
 * @return The number of bytes read on success, or SYSCALL_FAIL on failure.
 */
int32_t read(int32_t fd, void *buf, int32_t nbytes)
{
    pcb_t *cur_pcb = get_current_pcb();
    int32_t read_bytes;

    // Check if attempting to read from stdout (fd == 1).
    if (fd == 1)
    {
        return SYSCALL_FAIL;
    }

    // Sanity check for input parameters.
    if (nbytes <= 0 || fd >= MAX_FD || buf == NULL || cur_pcb->file_obj_table[fd].exist == 0 || fd < 0)
    {
#if DEBUG
        printf("Can't read!\n");
#endif
        return SYSCALL_FAIL;
    }

    // Read data using the appropriate file operation.
    read_bytes = (cur_pcb->file_obj_table[fd].f_operation.read)(fd, (int32_t *)buf, nbytes);
    cur_pcb->file_obj_table[fd].f_position += read_bytes;

    return read_bytes; // Return the number of bytes read on success.
}

/**
 * @brief Write data to a file descriptor.
 * This function writes data from a buffer to a file descriptor.
 *
 * @param fd The file descriptor index.
 * @param buf A pointer to the buffer containing the data to be written.
 * @param nbytes The number of bytes to write.
 *
 * @return The number of bytes written on success, or SYSCALL_FAIL on failure.
 */
int32_t write(int32_t fd, const void *buf, int32_t nbytes)
{
    pcb_t *cur_pcb = get_current_pcb();
    int32_t write_bytes;

    // Check if attempting to write to stdin (fd == 0).
    if (fd == 0)
    {
        return SYSCALL_FAIL;
    }

    // Sanity check for input parameters.
    if (nbytes <= 0 || fd >= MAX_FD || buf == NULL || cur_pcb->file_obj_table[fd].exist == 0 || fd < 0)
    {
#if DEBUG
        printf("Can't write!\n");
#endif
        return SYSCALL_FAIL;
    }

    // Write data using the appropriate file operation.
    write_bytes = (cur_pcb->file_obj_table[fd].f_operation.write)(fd, buf, nbytes);

    if (write_bytes == FS_FAIL)
    {
#if DEBUG

        printf("Can't write!\n");
#endif
        return SYSCALL_FAIL;
    }

    return write_bytes; // Return the number of bytes written on success.
}

/**
 * @brief Open a file for reading or writing.
 * This function opens a file with the specified name and associates it with a file descriptor.
 *
 * @param filename The name of the file to be opened.
 *
 * @return The file descriptor (fd) associated with the opened file on success,
 *         or SYSCALL_FAIL on failure.
 */
int32_t open(const uint8_t *filename)
{
    int32_t fd;
    dentry_t dentry;
    file_object_t current_file;
    pcb_t *cur_pcb = get_current_pcb();

    // Find the file in the file system.
    if (read_dentry_by_name(filename, &dentry) == FS_FAIL)
    {
        return SYSCALL_FAIL;
    }

    // Sanity check for opening the file.
    if (file_open(filename) == FS_FAIL)
    {
#if DEBUG
        printf("Can't find this file!\n");
#endif
        return SYSCALL_FAIL;
    }

    // Check if the maximum number of open files has been reached.
    if (cur_pcb->f_number >= MAX_FD)
    {
#if DEBUG
        printf("Can't open more file!\n");
#endif
        return SYSCALL_FAIL;
    }

    // Initialize the file object with the file information.
    initialize_file_object(&current_file, dentry);

    // Check for initialization failure.
    if (current_file.exist == 0)
    {
        return SYSCALL_FAIL;
    }

    // Assign this file to the process control block (PCB).
    fd = put_file_to_pcb(&current_file);

    // Open the file using the associated file operation.
    cur_pcb->file_obj_table[fd].f_operation.open(filename);

    return fd; // Return the file descriptor on success.
}

/**
 * @brief Close a file descriptor.
 *
 * This function closes a file descriptor, releasing associated resources and marking it as available for reuse.
 *
 * @param fd The file descriptor to be closed.
 *
 * @return SYSCALL_SUCCESS on successful closure, or SYSCALL_FAIL on failure. Possible failure conditions include:
 * - Attempting to close standard input (fd < 2).
 * - Attempting to close an invalid or unopened file descriptor.
 */
int32_t close(int32_t fd)
{
    pcb_t *cur_pcb = get_current_pcb();

    // Sanity check for closing the file descriptor.
    if (fd < 2 || fd >= MAX_FD)
    { // We can't close terminal (standard input/output).
        return SYSCALL_FAIL;
    }

    // Check if the file descriptor is already closed.
    if (cur_pcb->file_obj_table[fd].exist == 0)
    {
        return SYSCALL_FAIL;
    }

    // Close the file descriptor.
    cur_pcb->file_obj_table[fd].exist = 0;
    cur_pcb->f_number--;
    cur_pcb->file_obj_table[fd].f_operation.close(fd);

    return SYSCALL_SUCCESS; // Return SYSCALL_SUCCESS on successful closure.
}

int32_t ioctl(int32_t fd, int32_t request, void *buf)
{
    pcb_t *curPcb = get_current_pcb();
    if (fd > MAX_FD || fd < 0)
        return SYSCALL_FAIL;
    curPcb->file_obj_table[fd].f_operation.ioctl(fd, request, buf);
    return SYSCALL_SUCCESS;
}

/*-----------these functions are not used now, please ignore them------------------------------*/
/**
 * sys_getargs
 * INPUT: temporarily unkown
 * OUTPUT: print the system call message
 */
#if EXCEPTION_TEST
int32_t getargs(uint8_t *buf, int32_t nbytes)
{
    int32_t num = *(int32_t *)NULL;
    return 0;
}
#else
int32_t getargs(uint8_t *buf, int32_t nbytes)
{
    pcb_t *curPcb = get_current_pcb();
    if (buf == NULL || curPcb->arguments[0] == '\0' || nbytes <= 0)
    {
        return -1;
        // sanity check
    }
    // if nbytes bigger than buf length, we just copy the first max_buf content
    if (nbytes >= MAX_BUF)
        memcpy(buf, curPcb->arguments, MAX_BUF);
    else
        memcpy(buf, curPcb->arguments, nbytes);
    return 0;
}
#endif

/**
 * sys_vidmap
 * INPUT: temporarily unkown
 * OUTPUT: print the system call message
 */
int32_t vidmap(uint8_t **screen_start)
{

    if ((unsigned int)screen_start <= ADDR_128MB || (unsigned int)screen_start >= ADDR_128MB + ADDR_4MB || screen_start == NULL)
        return -1;
    return set_vidmap_paging(screen_start);
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
