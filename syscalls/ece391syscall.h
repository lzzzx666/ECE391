#if !defined(ECE391SYSCALL_H)
#define ECE391SYSCALL_H

#include <stdint.h>

/*VGA ioctl numbers*/
#define IOCTL_TEXT_MODE 1
#define IOCTL_MODE_X 2
#define IOCTL_VMEM_MAP 3
#define IOCTL_SET_PAL 4

/*terminal ioctl number*/
#define IOCTL_SIMKB 1

/*file system ioctl numbers*/
#define IOCTL_FILE_TYPE 1
#define IOCTL_FILE_SIZE 2

/* All calls return >= 0 on success or -1 on failure. */

/*
 * Note that the system call for halt will have to make sure that only
 * the low byte of EBX (the status argument) is returned to the calling
 * task.  Negative returns from execute indicate that the desired program
 * could not be found.
 */
extern int32_t ece391_halt(uint8_t status);
extern int32_t ece391_execute(const uint8_t *command);
extern int32_t ece391_read(int32_t fd, void *buf, int32_t nbytes);
extern int32_t ece391_write(int32_t fd, const void *buf, int32_t nbytes);
extern int32_t ece391_open(const uint8_t *filename);
extern int32_t ece391_close(int32_t fd);
extern int32_t ece391_getargs(uint8_t *buf, int32_t nbytes);
extern int32_t ece391_vidmap(uint8_t **screen_start);
extern int32_t ece391_set_handler(int32_t signum, void *handler);
extern int32_t ece391_sigreturn(void);
extern int32_t ece391_ioctl(int32_t fd, int32_t request, void *buf);
enum signums
{
	DIV_ZERO = 0,
	SEGFAULT,
	INTERRUPT,
	ALARM,
	USER1,
	NUM_SIGNALS
};

#endif /* ECE391SYSCALL_H */
