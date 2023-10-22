#include "terminal.h"
#include "lib.h"

terminal_t main_terminal, prev_terminal;

void initialize_terminal()
{
    main_terminal.count = main_terminal.enter_pressed = 0;
    main_terminal.cursor_x = main_terminal.cursor_y = 0;
    memset((void*)main_terminal.terminal_buf, '\0', MAX_TERMINAL_SIZE);
    enable_cursor(14,15);
    update_cursor(0, 0);
}

int32_t terminal_close(int32_t fd)
{
    return 0;
}

int32_t terminal_open(const char *filename)
{
    return 0;
}


int32_t terminal_read(int32_t fd, void *buf, int32_t nbytes)
{
    if((!buf) || (nbytes <= 0)) return -1;
    int i = 0, ret_count = 0;
    main_terminal.enter_pressed = 0;
    while(!main_terminal.enter_pressed) {}
    for (i = 0; i < nbytes && i < READ_MAX_SIZE && prev_terminal.terminal_buf[i] != '\0'; i++)
    {
        ((char *)buf)[i] = prev_terminal.terminal_buf[i];
        ret_count++;
        if(prev_terminal.terminal_buf[i] == '\n')   break;
    }
    ((char *)buf)[ret_count] = '\0';
    //clear the terminal buffer for next use
   // main_terminal.count = main_terminal.enter_pressed = 0;
  //  memset((void*)main_terminal.terminal_buf, '\0', MAX_TERMINAL_SIZE);
    return ret_count;
}

int32_t terminal_write(int32_t fd, void *buf, int32_t nbytes)
{
    if((!buf) || (nbytes <= 0)) return -1;
    int i = 0, ret_count = 0;
    for(i = 0; i < nbytes; i++)
    {
        if(((char*)buf)[i] == '\0') break;
        ret_count++;
        putc(((char*)buf)[i]);
    }
    return ret_count;
}

void terminal_clear()
{
    clear();
    main_terminal.count = main_terminal.enter_pressed = 0;
    memset((void*)main_terminal.terminal_buf, '\0', MAX_TERMINAL_SIZE);
}

