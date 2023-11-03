#include "pcb.h"
/*those are function tables, used for initializing file operations*/
open_func open_o[4] = {rtc_open, directory_open, file_open, terminal_open};
close_func close_o[4] = {rtc_close, directory_close, file_close, terminal_close};
read_func read_o[4] = {(read_func)rtc_read, (read_func)directory_read,
                       (read_func)file_read, (read_func)terminal_read};
write_func write_o[4] = {(write_func)rtc_write, (write_func)directory_write,
                         (write_func)file_write, (write_func)terminal_write};

/*it stores the pcbs of all processes*/
pcb_t *pcb_array[MAX_TASK];

/*it is used to indicate whether a pcb exists*/
int8_t pcb_bitmap = 0x00;

/*it records the active process id*/

int32_t current_pid;

/**
 * create_pcb
 * it will find a empty position in the pcb_array and create a new pcb ,initialize it,
 * and then fill it in the array.
 * input:none
 * @return the pid for the new pcb on success or -1 on fail
 */
int32_t create_pcb()
{
    /*the index for the new pcb*/
    int32_t pcb_index = 0;

    /*the pointer of the new pcb*/
    void *new_pcb = NULL;

    /*get a empty pid*/
    while (1)
    {   
        //7-pcb_index is the corresponding bit in the bitmap
        if (((pcb_bitmap >> (7 - pcb_index)) & 0x1) == TASK_NONEXIST || pcb_index >= MAX_TASK)
        {
            break;
        }
        pcb_index++;
    }

    /*check if the pcb array is full*/
    if (pcb_index >= MAX_TASK)
    {
        return -1;
    }

    /*initialize the new task*/
    new_pcb = (void *)(KERNAL_BOTTOM - TASK_STACK_SIZE * (pcb_index + 1));
    initialize_new_pcb((pcb_t *)new_pcb, pcb_index);

    /*initialize the stdin and stdout*/
    initialize_stdin_stdout(new_pcb);

    /*add the new pcb to the array*/
    pcb_array[pcb_index] = new_pcb;
    pcb_bitmap = pcb_bitmap | (0x1 << (7 - pcb_index));//7-pcb_index is the corresponding bit in the bitmap

    return pcb_index;
}

/**
 * delete_pcb
 * it will delete the active pcb in the pcb array
 * input:none
 * @return 0 on success
 */
int32_t delete_pcb()
{
    /*update the current pid based on the esp(the active process)*/
    update_current_pid();

    /*clear the pcb array and the bitmap*/
    pcb_array[current_pid] = NULL;
    pcb_bitmap = pcb_bitmap & ~(0x1 << (7 - current_pid));//7-current_pid is the corresponding bit in the bitmap
    return 0;
}

/*-----------------------------------helper function-----------------------------------------------------*/

/**
 * initialize_new_pcb
 * it will set necessary parameters for the new pcb structure, like pid, f_number
 * ,all information is listed in pcb_h
 * @param pcb - the new created pcb to be initialized
 * @param pid - the pid for the new created pcb
 * output:none
 */
void initialize_new_pcb(pcb_t *pcb, int32_t pid)
{
    /*the iterator*/
    int32_t i;

    /*update the pid*/
    update_current_pid();

    /*set the pid and file number for the pcb*/
    pcb->pid = pid;
    pcb->f_number = 0;

    /*store the parent pcb information*/
    if (((pcb_bitmap >> (7 - current_pid)) & 1))//7-current_pid is the corresponding bit in the bitmap
    {
        pcb->parent_pid = current_pid;
    }
    else
    {
        pcb->parent_pid = NULL;
        pcb->parent_esp = NULL;
    }

    /*clear the file object table of the pcb*/
    for (i = 0; i < MAX_FD; i++)
    {
        pcb->file_obj_table[i].exist = 0;
    }
}

/**
 * initialize_file_object
 * it will set necessary parameters for the new file object, like inode, file operation,
 * all information is listed in pcb.h
 * @param file_object - the new created file object to be initialized
 * @param dentry - the dentry for the corresponding file
 * output:none
 */
void initialize_file_object(file_object_t *file_object, dentry_t dentry)
{
    /*initialize all parameters*/
    if (dentry.fileType <= 3) //3 is used to check if the dentry is valid
    {
        file_object->exist = 1;
        file_object->f_position = 0;
        file_object->inode = dentry.inodeIdx;
        assign_operation(file_object, dentry.fileType);
    }
    else
    {
        file_object->exist = 0;
    }
}

/**
 * nitialize_stdin_stdout
 * it will initialize stdin/out for the new program
 * @param pcb - the new created pcb(or new program)
 * output:none
 */
void initialize_stdin_stdout(pcb_t *pcb)
{
    /*initialize the stdin*/
    pcb->file_obj_table[0].exist = 1;
    pcb->file_obj_table[0].f_position = 0;
    pcb->file_obj_table[0].inode = 0;
    assign_operation(&(pcb->file_obj_table)[0], TERMINAL);

    /*initialize the stdout*/
    pcb->file_obj_table[1].exist = 1;
    pcb->file_obj_table[1].f_position = 0;
    pcb->file_obj_table[1].inode = 0;
    assign_operation(&(pcb->file_obj_table)[1], TERMINAL);

    /*stdin/out are counted as two files*/
    pcb->f_number = 2; //2 is the stdin and stdout
}

/**
 * assign_operation
 * it will assign file operations for a file based on the file type
 * @param file_object - the file object to be assigned
 * @param file_type - the file type for the file
 * output:none
 */
void assign_operation(file_object_t *file_object, int32_t file_type)
{

    /*assign open,close,read and write operations based on the file type*/
    (*file_object).f_operation.open = open_o[file_type];
    (*file_object).f_operation.close = close_o[file_type];
    (*file_object).f_operation.read = read_o[file_type];
    (*file_object).f_operation.write = write_o[file_type];
}

/**
 * put_file_to_pcb
 * it will assign the new open file(file object) to the active pcb
 * @param file_object - the file object to be assigned
 * output:none
 */
int32_t put_file_to_pcb(file_object_t *file_object)
{
    /*the empty fd*/
    int32_t fd;

    /*current pcb*/
    pcb_t *pcb = get_current_pcb();

    /*find the empty entry*/
    for (fd = 0; fd < MAX_FD; fd++)
    {
        if (!pcb->file_obj_table[fd].exist)
            break;
    }

    /*put it to the file object table*/
    pcb->file_obj_table[fd] = *file_object;
    pcb->f_number++;
    return fd;
}

/**
 * get_current_task
 * it will get the active task(the pid for the task)
 * @param cur_pid - the global variable that stores the active pid
 * output:none
 */
void update_current_pid()
{
    /*temporarily used variable in this function*/
    int32_t temp_pid;

    /*get the address of the pcb */
    asm volatile(
        "movl %%esp,%%eax\n\t"
        "andl $0xffffe000,%%eax\n\t" //because the size of pcb+stack is 8kb, we use 0xffffe000 to get the pid
        "movl %%eax,%0"
        : "=r"(temp_pid)
        :
        : "%eax");

    /*calculate the pid based on the pcb*/
    temp_pid = (KERNAL_BOTTOM - temp_pid - TASK_STACK_SIZE) / TASK_STACK_SIZE;
    current_pid = temp_pid;
}

/**
 * get_current_pcb
 * it will get the active task(the pcb for the task)
 * input:none
 * output:none
 */
pcb_t *get_current_pcb()
{
    update_current_pid();
    return pcb_array[current_pid];
}
