#ifndef SYSTEMCALL
#define SYSTEMCALL
#include "types.h"
#include "lib.h"
/*sys_halt*/
extern int32_t sys_halt(uint8_t status);
/*sys_execute*/
extern int32_t sys_execute(const uint8_t *command);
/*ys_read*/
extern int32_t sys_read(int32_t fd, void *buf, int32_t nbytes);
/*sys_write*/
extern int32_t sys_write(int32_t fd, const void *buf, int32_t nbytes);
/*sys_open*/
extern int32_t sys_open(const uint8_t *filename);
/*sys_clos*/
extern int32_t sys_close(int32_t fd);
/*sys_getargs*/
extern int32_t sys_getargs(uint8_t *buf, int32_t nbytes);
/*sys_vidmap*/
extern int32_t sys_vidmap(uint8_t **screen_start);
/*sys_set_handler*/
extern int32_t sys_set_handler(int32_t signum, void *handler);
/*sys_sigretur*/
extern int32_t sys_sigreturn(void);
#endif
