#ifndef PCB_H
#define PCB_H
#include "types.h"
#include "systemcall.h"
#define  MAX_TASK 8
#define TASK_NONEXIST 0
#define KERNAL_BOTTOM 0x800000 //This is the bottom of the kernal stack, locate at 8MB
#define TASK_STACK_SIZE 0x2000  //This is the 8KB task stack
typedef struct pcb
{
    file_object_t file_obj_table[8];
    int32_t f_number;
    int32_t signal_info;    //unused
    int32_t pid;
    int32_t parent_pid;
    int32_t parent_esp;
    int8_t arguments[MAX_BUF-MAX_FILE_NAME];
}pcb_t;

typedef struct file_object
{
    int32_t* f_operation;
    int32_t inode;
    int32_t f_position; 
    int32_t exist;      

}file_object_t;

extern int32_t current_pid;
extern pcb_t* pcb_array[];
extern int8_t pcb_bitmap;
int32_t create_pcb();
int32_t delete_pcb();
void get_current_task(int32_t* cur_pid);
#endif