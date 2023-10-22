#include "systemcall.h"

// int32_t pid_array[8];
// file_obj file_obj_table[8];
int32_t* terminal_o[4] = {terminal_open, terminal_close, terminal_read, terminal_write};
int32_t* rtc_o[4]      = {rtc_open, rtc_close, rtc_read, rtc_write};
int32_t* file_o[4]     = {file_open, file_close, file_read, file_write};
int32_t* dir_o[4]      = {dir_open, dir_close, dir_read, dir_write};
/**
 * sys_halt
 * INPUT: temporarily unkown
 * OUTPUT: print the system call message
 */
int32_t halt(uint8_t status)
{
    printf("sys_halt!");
    return 0;
}
/**
 * sys_execute
 * INPUT: temporarily unkown
 * OUTPUT: print the system call message
 */
int32_t execute(const uint8_t *command)
{  
    int32_t i;
    char current_char;
    char name_buf[MAX_FILE_NAME]={'\0'};
    char args[MAX_BUF-MAX_FILE_NAME]={'0'};
    char test_buf[10];
    dentry_t dentry; 
    int32_t pcb_index;
    pcb_t* cur_pcb=pcb_array[current_pid];
    pcb_t* new_pcb=NULL;
    /*sanity check*/
    if(command==NULL){
        return -1;
    }
    /*get the file name*/
    while(current_char=command[i]!='\0'){
        name_buf[i++]=current_char;
    }
    /*check if file name is so long*/
    if(i>=MAX_FILE_NAME){
        return -1;
    }
    /*check if the file exists*/
    if(read_dentry_by_name(name_buf,&dentry)==-1){
        return -1;
    }
    /*check if it is a regular file*/
    if(dentry.filetype!=REGULAR_FILE){
        return -1;
    }
    /*check if the file can be executed*/
    if(read_data(dentry.inode_num,0,test_buf,4)==0){
        return -1;
    }
    /*get other arguments*/
    for(i=MAX_FILE_NAME;i<MAX_BUF;i++){
        args[i-MAX_FILE_NAME]=command[i];
    }
    // /*find the pid*/
    // for(i=0;i<MAX_TASK;i++){
    //     if(pcb_bitmap>>(7-i)&0x1){
    //         break;
    //     }
    //     /*check if any pid is available*/
    //     if(i==MAX_TASK-1){
    //         return -1;
    //     }
    // }
    /*check if a new pcb can be created*/
    if(pcb_index=screate_pcb()==-1){
        return -1;
    }
    /*store arguments*/
    new_pcb=pcb_array[pcb_index];
    for(i=MAX_FILE_NAME;i<MAX_BUF;i++){
        new_pcb->arguments[i]= args[i];
    }
    /*start paging*/
    set_paging(pcb_index);

    /*load into memory*/
    read_data(dentry.inode_num,0,PROGRAM_IMAGE,PROGRAM_IMAGE_SIZE);

    /*change tss*/
    tss.ss0=KERNEL_DS;
    tss.esp0=KERNAL_BOTTOM-pcb_index*TASK_STACK_SIZE;



    printf("sys_execute!");
    return 0;
}
/**/
void to_user_mode(){
    
}
/**
 * sys_read
 * INPUT: temporarily unkown
 * OUTPUT: print the system call message
 */
int32_t read(int32_t fd, void *buf, int32_t nbytes)
{
    pcb_t* cur_pcb = pcb_array[current_pid];
    int32_t file_position;
    int32_t read_bytes;

    /*sanity check*/
    if(nbytes<=0 || fd>=MAX_FD || buf==NULL || cur_pcb->file_obj_table[fd].exist==0 || fd<0){
        printf("Can't read!\n")
        return -1;
    }

    /*read data*/
    file_position=cur_pcb->file_obj_table[fd].f_position;
    read_bytes=cur_pcb->file_obj_table[fd].f_operation[READ](fd,file_position,buf,nbytes);
    cur_pcb->file_obj_table[fd].f_position+=read_bytes;
    
    printf("sys_read!");
    return 0;
}
/**
 * sys_write
 * INPUT: temporarily unkown
 * OUTPUT: print the system call message
 */
int32_t write(int32_t fd, const void *buf, int32_t nbytes)
{
    pcb_t* cur_pcb = pcb_array[current_pid];
    int32_t file_position;
    int32_t write_bytes;

    /*sanity check*/
    if(nbytes<=0 || fd>=MAX_FD || buf==NULL || cur_pcb->file_obj_table[fd].exist==0 || fd<0){
        printf("Can't write!\n")
        return -1;
    }

    /*write data*/
    file_position=cur_pcb->file_obj_table[fd].f_position;
    write_bytes=cur_pcb->file_obj_table[fd].f_operation[WRITE](fd,file_position,buf,nbytes);
    cur_pcb->file_obj_table[fd].f_position+=write_bytes;
    
    printf("sys_write!");
    return 0;
}
/**
 * sys_open
 * INPUT: temporarily unkown
 * OUTPUT: print the system call message
 */
int32_t open(const uint8_t *filename)
{
    int32_t fd;
    dentry_t dentry;
    file_object_t current_file;
    pcb_t* cur_pcb = pcb_array[current_pid];
    int32_t fnumber = cur_pcb->f_number;
    int32_t f_index;

    /*sanity check*/
    if(fopen(filename)==NULL){
        printf("Fail to open file!\n");
        return -1;
    }
    if(fnumber>=MAX_FD){
        printf("Can't open more file!\n")
        return -1;
    }

    /*initialize pcb*/
    read_entry_by_name(filename,&dentry);
    cur_pcb->f_number++;

    /*initialize the file object*/
    current_file.inode=dentry.inode_num;
    current_file.exist=1;
    current_file.f_position=0;
    switch(dentry.filetype){
        case REGULAR_FILE:
            current_file.f_operation=file_o;
            break;
        case DIRECTORY:
            current_file.f_operation=dir_o;
            break;
        case RTC_DEVICE:
            current_file.f_operation=rtc_o;
            break;
        default:break;
    }
    /*find the empty entry*/
    for(f_index=0;f_index<MAX_FD;f_index++){
        if(!cur_pcb->file_obj_table[f_index].exist)
        break;
    }
    /*assign the file*/
    cur_pcb->file_obj_table[f_index]=current_file;

    /*open the file*/
    cur_pcb->file_obj_table[f_index].f_operation[OPEN](filename);

    return f_index;

    printf("sys_open!");
    return 0;
}
/**
 * sys_close
 * INPUT: temporarily unkown
 * OUTPUT: print the system call message
 */
int32_t close(int32_t fd)
{
    pcb_t* cur_pcb = pcb_array[current_pid];
    if(fd<2 || fd>=MAX_FD){
        return -1;
    }
    if(cur_pcb->file_obj_table[fd].exist=0){
        return -1;
    }
    /*close the file*/
    cur_pcb->file_obj_table[fd].f_operation[CLOSE](fd);

    cur_pcb->file_obj_table[fd].exist=0;
    cur_pcb->f_number--;

    printf("sys_close!");
    return 0;
}
/**
 * sys_getargs
 * INPUT: temporarily unkown
 * OUTPUT: print the system call message
 */
int32_t getargs(uint8_t *buf, int32_t nbytes)
{
    printf("sys_getargs!");
    return 0;
}
/**
 * sys_vidmap
 * INPUT: temporarily unkown
 * OUTPUT: print the system call message
 */
int32_t vidmap(uint8_t **screen_start)
{
    printf("sys_vidmap!");
    return 0;
}
/**
 * sys_sys_set_handler
 * INPUT: temporarily unkown
 * OUTPUT: print the system call message
 */
int32_t set_handler(int32_t signum, void *handler)
{
    printf("sys_set_handler!");
    return 0;
}
/**
 * sys_sigreturn
 * INPUT: temporarily unkown
 * OUTPUT: print the system call message
 */
int32_t sigreturn(void)
{
    printf("sys_sigreturn!");
    return 0;
}
