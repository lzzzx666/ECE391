#ifndef SYSTEMCALL
#define SYSTEMCALL

/*macros are defined here*/
#define MAX_FD 8
#define OPEN 0
#define CLOSE 1
#define READ 2
#define WRITE 3

#define MAX_FILE_NAME 32
#define MAX_BUF 128

#define PROGRAM_IMAGE 0x08048000
#define PROGRAM_IMAGE_END 0x08400000
#define PROGRAM_IMAGE_SIZE PROGRAM_IMAGE_END - PROGRAM_IMAGE

#define EXECUTAVLE_MAGIC_NUMBER_SIZE 4

#define SYSCALL_SUCCESS 0
#define SYSCALL_FAIL -1
#define PADDING 0

#include "types.h"
#include "lib.h"
#include "pcb.h"
#include "x86_desc.h"
#include "fs.h"

/*sys_halt*/
extern int32_t halt(uint8_t status);
/*sys_execute*/
extern int32_t execute(const uint8_t *command);
/*ys_read*/
extern int32_t read(int32_t fd, void *buf, int32_t nbytes);
/*sys_write*/
extern int32_t write(int32_t fd, const void *buf, int32_t nbytes);
/*sys_open*/
extern int32_t open(const uint8_t *filename);
/*sys_clos*/
extern int32_t close(int32_t fd);
/*sys_getargs*/
extern int32_t getargs(uint8_t *buf, int32_t nbytes);
/*sys_vidmap*/
extern int32_t vidmap(uint8_t **screen_start);
/*sys_set_handler*/
extern int32_t set_handler(int32_t signum, void *handler);
/*sys_sigretur*/
extern int32_t sigreturn(void);

/**/
void to_user_mode(int32_t eip, int32_t eflags, int32_t esp, int32_t pid);
/**/

#endif
