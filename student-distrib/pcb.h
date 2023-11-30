#ifndef PCB_H
#define PCB_H
#include "types.h"
#include "systemcall.h"
#include "terminal.h"
#include "rtc.h"
#include "fs.h"

/*macros used in pcb module*/
#define MAX_TASK 8            //I choose 8 as the max task numver
#define TASK_NONEXIST 0         //this indicates the task does not exist
#define KERNAL_BOTTOM 0x800000 // This is the bottom of the kernal stack, locate at 8MB
#define TASK_STACK_SIZE 0x2000 // This is the 8KB task stack
#define TERMINAL 3              //3 is the index of terminal functions int the function table
#define MAX_FILE_NUM 8          //one process can open 8 files at the same time(including stdin and stdout)
#define UNINITIALIZED -2

/*define some function pointer types for open close read and wtite*/
typedef int32_t (*open_func)(const uint8_t *filename);
typedef int32_t (*close_func)(int32_t fd);
typedef int32_t (*read_func)(int32_t fd, ...);
typedef int32_t (*write_func)(uint32_t inode, ...);

/*these are the function pointer table*/
extern open_func open_o[];
extern close_func close_o[];
extern read_func read_o[];
extern write_func write_o[];
extern int32_t sche_array[];
extern int32_t sche_index;


/**
 * file_operation_t
 * the file operation struction contains four operation for the file:open,close
 * read and write.
 */
typedef struct file_operation
{
    open_func open;
    close_func close;
    read_func read;
    write_func write;
} file_operation_t;

/**
 * file_object_t
 * the file object struction is created when the file is open. It records some 
 * information about the file and it is stored in the pcb structure.
 */
typedef struct file_object
{
    file_operation_t f_operation;
    int32_t inode;
    uint32_t f_position;
    int32_t exist;

} file_object_t;

/**
 * pcb_t
 * the pcb struction is used to manage the process. It contains the information
 * about a specific process, such as all files it opens and something else.
 */
typedef struct pcb
{
    file_object_t file_obj_table[MAX_FILE_NUM];
    int32_t f_number;
    int32_t pid;
    int32_t parent_pid;
    int32_t parent_esp;
    int32_t parent_ebp;
    int8_t arguments[MAX_BUF];
} pcb_t;

/*it records the active process id*/
extern int32_t current_pid;

/*it stores the pcbs of all processes*/
extern pcb_t *pcb_array[];

/*it is used to indicate whether a pcb exists*/
extern int8_t pcb_bitmap;

/*create a new pcb*/
int32_t create_pcb();

/*delete the active pcb*/
int32_t delete_pcb();

/*----------------------------------helper function-----------------------------------------*/

/*it will initialize the necessary information about the new created pcb*/
extern void initialize_new_pcb(pcb_t *pcb, int32_t pid);

/*it will initialize the necessary information about the file object*/
extern void initialize_file_object(file_object_t *file_object, dentry_t dentry);

/*it will initialize the stdin/stdout for the new created pcb*/
extern void initialize_stdin_stdout(pcb_t *pcb);

/*it will assign operations for the new file object*/
extern void assign_operation(file_object_t *file_object, int32_t file_type);

/*update the current_pid variable based on esp register*/
extern void update_current_pid();

/*when open a file, put the file in the file array of the pcb*/
extern int32_t put_file_to_pcb(file_object_t *file_object);

/**
 * @brief Get the Process Control Block (PCB) of the current process.
 *
 * This function returns a pointer to the Process Control Block (PCB) of the currently executing process.
 * It first updates the current process ID (PID) using the function `update_current_pid`.
 *
 * @return A pointer to the PCB of the current process.
 */
extern pcb_t* get_current_pcb();

#endif
