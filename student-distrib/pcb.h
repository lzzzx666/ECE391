#ifndef PCB_H
#define PCB_H
#include "types.h"
#include "systemcall.h"
#include "terminal.h"
#include "rtc.h"
#include "fs.h"
#define MAX_TASK 8
#define TASK_NONEXIST 0
#define KERNAL_BOTTOM 0x800000 // This is the bottom of the kernal stack, locate at 8MB
#define TASK_STACK_SIZE 0x2000 // This is the 8KB task stack
#define TERMINAL 3

typedef int32_t (*open_func)(const uint8_t *filename);
typedef int32_t (*close_func)(int32_t fd);
typedef int32_t (*read_func)(uint8_t *filename, uint8_t *buf, uint32_t length);
typedef int32_t (*write_func)(uint32_t inode, uint8_t *buf, uint32_t length);

extern open_func open_o[];
extern close_func close_o[];
extern read_func read_o[];
extern write_func write_o[];

typedef struct file_operation
{
    open_func open;
    close_func close;
    read_func read;
    write_func write;
} file_operation_t;

typedef struct file_object
{
    file_operation_t f_operation;
    int32_t inode;
    uint32_t f_position;
    int32_t exist;
    int32_t file_length;
    int8_t *filename;

} file_object_t;

typedef struct pcb
{
    file_object_t file_obj_table[8];
    int32_t f_number;
    int32_t pid;
    int32_t parent_pid;
    int32_t parent_esp;
    int8_t arguments[MAX_BUF - MAX_FILE_NAME];
} pcb_t;

extern int32_t current_pid;
extern pcb_t *pcb_array[];
extern int8_t pcb_bitmap;
int32_t create_pcb();
int32_t delete_pcb();

/*--------------------------------------------------------------------------------------*/
/*helper function*/

extern void initialize_new_pcb(pcb_t *pcb, int32_t pid);

extern void initialize_file_object(file_object_t *file_object, dentry_t dentry);

extern void initialize_stdin_stdout(pcb_t *pcb);

extern void assign_operation(file_object_t *file_object, int32_t file_type);

extern void get_current_task(int32_t *cur_pid);

extern int32_t put_file_to_pcb(file_object_t *file_object);

#endif
