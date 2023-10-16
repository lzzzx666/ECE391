#include "systemcall.h"

/**
 * sys_halt
 * INPUT: temporarily unkown
 * OUTPUT: print the system call message
 */
int32_t sys_halt(uint8_t status)
{
    printf("sys_halt!");
    return 0;
}
/**
 * sys_execute
 * INPUT: temporarily unkown
 * OUTPUT: print the system call message
 */
int32_t sys_execute(const uint8_t *command)
{
    printf("sys_execute!");
    return 0;
}
/**
 * sys_read
 * INPUT: temporarily unkown
 * OUTPUT: print the system call message
 */
int32_t sys_read(int32_t fd, void *buf, int32_t nbytes)
{
    printf("sys_read!");
    return 0;
}
/**
 * sys_write
 * INPUT: temporarily unkown
 * OUTPUT: print the system call message
 */
int32_t sys_write(int32_t fd, const void *buf, int32_t nbytes)
{
    printf("sys_write!");
    return 0;
}
/**
 * sys_open
 * INPUT: temporarily unkown
 * OUTPUT: print the system call message
 */
int32_t sys_open(const uint8_t *filename)
{
    printf("sys_oepn!");
    return 0;
}
/**
 * sys_close
 * INPUT: temporarily unkown
 * OUTPUT: print the system call message
 */
int32_t sys_close(int32_t fd)
{
    printf("sys_close!");
    return 0;
}
/**
 * sys_getargs
 * INPUT: temporarily unkown
 * OUTPUT: print the system call message
 */
int32_t sys_getargs(uint8_t *buf, int32_t nbytes)
{
    printf("sys_getargs!");
    return 0;
}
/**
 * sys_vidmap
 * INPUT: temporarily unkown
 * OUTPUT: print the system call message
 */
int32_t sys_vidmap(uint8_t **screen_start)
{
    printf("sys_vidmap!");
    return 0;
}
/**
 * sys_sys_set_handler
 * INPUT: temporarily unkown
 * OUTPUT: print the system call message
 */
int32_t sys_set_handler(int32_t signum, void *handler)
{
    printf("sys_set_handler!");
    return 0;
}
/**
 * sys_sigreturn
 * INPUT: temporarily unkown
 * OUTPUT: print the system call message
 */
int32_t sys_sigreturn(void)
{
    printf("sys_sigreturn!");
    return 0;
}
