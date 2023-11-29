#ifndef SYSTEMCALL
#define SYSTEMCALL

/*macros are defined here*/
#define MAX_FD 8 //maximum file number for a process
#define OPEN 0  //those 0,1,2,3 are index in the operation table
#define CLOSE 1
#define READ 2
#define WRITE 3

#define MAX_FILE_NAME 32 //32 is the maximum size of file name
#define MAX_BUF 128     //128 is the maximum size of the terminal buffer

#define PROGRAM_IMAGE 0x08048000    //0x08048000 is the start place of the program image
#define PROGRAM_IMAGE_END 0x08400000    //the end of the program page
#define PROGRAM_IMAGE_SIZE PROGRAM_IMAGE_END - PROGRAM_IMAGE //the maximum size of the the program image

#define EXECUTABLE_MAGIC_NUMBER_SIZE 4

#define SYSCALL_SUCCESS 0  //those are some macros that used for convenience
#define SYSCALL_FAIL -1
#define PADDING 0

#include "types.h"
#include "lib.h"
#include "pcb.h"
#include "x86_desc.h"
#include "fs.h"
/*the return value*/
extern int32_t retVal;

/**
 * @brief Halt the current process with a given status code.
 * This function halts the execution of the current process and performs various cleanup tasks.
 *
 * @param status The status code indicating the reason for halting.
 *
 * @return The status code provided as input, which is returned upon process termination.
 */
extern int32_t halt(uint8_t status);

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
extern int32_t execute(const uint8_t *command);

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
extern int32_t read(int32_t fd, void *buf, int32_t nbytes);

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
extern int32_t write(int32_t fd, const void *buf, int32_t nbytes);

/**
 * @brief Open a file for reading or writing.
 * This function opens a file with the specified name and associates it with a file descriptor.
 *
 * @param filename The name of the file to be opened.
 *
 * @return The file descriptor (fd) associated with the opened file on success,
 *         or SYSCALL_FAIL on failure.
 */
extern int32_t open(const uint8_t *filename);

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
extern int32_t close(int32_t fd);

/*------------------------those functions are not used now------------------------------------------------*/
/*sys_getargs*/
extern int32_t getargs(uint8_t *buf, int32_t nbytes);

/*sys_vidmap*/
extern int32_t vidmap(uint8_t **screen_start);

/*sys_set_handler*/
extern int32_t set_handler(int32_t signum, void *handler);

/*sys_sigretur*/
extern int32_t sigreturn(void);

extern int32_t simulate_keyboard(void);

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
void to_user_mode(int32_t eip, int32_t eflags, int32_t esp, int32_t pid);


#endif
