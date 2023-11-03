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

#define EXECUTAVLE_MAGIC_NUMBER_SIZE 4 //there are 4 bytes to identify executable file

#define SYSCALL_SUCCESS 0  //those are some macros that used for convenience
#define SYSCALL_FAIL -1
#define PADDING 0

#include "types.h"
#include "lib.h"
#include "pcb.h"
#include "x86_desc.h"
#include "fs.h"

/*halt a program*/
extern int32_t halt(uint8_t status);
/*execute a program*/
extern int32_t execute(const uint8_t *command);
/*read a file to the buffer*/
extern int32_t read(int32_t fd, void *buf, int32_t nbytes);
/*write the buf to the file*/
extern int32_t write(int32_t fd, const void *buf, int32_t nbytes);
/*open a file and put it in the file table for the current process*/
extern int32_t open(const uint8_t *filename);
/*close a file and remove it from the file table*/
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

/*------------------------helper functions-------------------------------------------------------------*/
/*it will go from the kernel to the user program*/
void to_user_mode(int32_t eip, int32_t eflags, int32_t esp, int32_t pid);

#endif
