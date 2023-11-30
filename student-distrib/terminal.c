#include "terminal.h"
#include "lib.h"
#include "systemcall.h"

terminal_t main_terminal[TERMINAL_NUMBER];
terminal_t prev_terminal[TERMINAL_NUMBER];
const uint8_t* video_mem[TERMINAL_NUMBER]={VIDEO_TERMINAL1,VIDEO_TERMINAL2,VIDEO_TERMINAL3};
int current_terminal = 0;

const uint8_t *shared_user_vid_mem = (char*)VIDEO;

// void initialize_terminal()
// initializes the main terminal with default values.
// input:none.
// return:none
// side effect:init the main_terminal and enable the cursor
void initialize_terminal(int32_t terminal_num)
{
    terminal_t *terminal = &main_terminal[terminal_num];
    terminal->count = terminal->enter_pressed = 0;
    terminal->cursor_x = terminal->cursor_y = 0;
    memset((void *)terminal->terminal_buf, '\0', MAX_TERMINAL_SIZE);
    enable_cursor(14, 15); // set cursor shape
    update_cursor(0, 0);   // set cursor position
    // @@
    terminal->video_mem_backup = video_mem[terminal_num];
}
// int32_t terminal_close(int32_t fd)
//.Closes the terminal.
// input:none.
// return:0
// side effect:Closes the terminal
int32_t terminal_close(int32_t fd)
{
    terminal_t *terminal = &main_terminal[current_terminal];
    terminal_t *prev = &prev_terminal[current_terminal];
    memset((void *)terminal->terminal_buf, '\0', MAX_TERMINAL_SIZE);
    terminal->count = 0;
    prev->count = 0;
    terminal->enter_pressed = 0;
    return 0;
}
// int32_t terminal_open(int32_t fd)
// open the terminal.
// input:none.
// return:0
// side effect:open the terminal
int32_t terminal_open(const uint8_t *filename)
{
    return 0;
}

// int32_t terminal_read(int32_t fd, void *buf, int32_t nbytes)
// Description:Reads from the previous terminal buffer to the current terminal buffer
// until an enter key is pressed or a newline character (\n) is encountered.
// Side Effects:
// - Updates the value of main_terminal.enter_pressed to 0.
// - Populates the given buffer buf with data read from the previous terminal buffer.
// Input:
// - int32_t fd: File descriptor (not used in this implementation).
// - void *buf: Pointer to the buffer to store the read data.
// - int32_t nbytes: Maximum number of bytes to read.
// Return:
// - int32_t: Total number of bytes read or -1 on error.
int32_t terminal_read(int32_t fd, void *buf, int32_t nbytes)
{
    terminal_t *terminal = &main_terminal[current_terminal];
    terminal_t *prev = &prev_terminal[current_terminal];
    if ((!buf) || (nbytes <= 0))
        return -1; // sanity check
    int i = 0, ret_count = 0;
    terminal->enter_pressed = 0;
    while (!terminal->enter_pressed);
    for (i = 0; i < nbytes && i < READ_MAX_SIZE && prev->terminal_buf[i] != '\0'; i++)
    {
        ((char *)buf)[i] = prev->terminal_buf[i]; // read from previous buffer to the terminal buffer
        ret_count++;
        if (prev->terminal_buf[i] == '\n')
            break; // when meeting \n, return
    }
    ((char *)buf)[ret_count] = '\0';
    return ret_count; // return total byte we read
}
// int32_t terminal_write(int32_t fd, void *buf, int32_t nbytes)
// Description:
// Writes to the terminal from the given buffer up to a specified
// number of bytes or until a null character (\0) is encountered.
// Side Effects:
// - Outputs the buffer content to the terminal.
// Input:
// - int32_t fd: File descriptor (not used in this implementation).
// - void *buf: Pointer to the buffer containing the data to be written.
// - int32_t nbytes: Maximum number of bytes to write.
// Return:
// - int32_t: Total number of bytes written or -1 on error.
int32_t terminal_write(int32_t fd, void *buf, int32_t nbytes)
{
    if ((!buf) || (nbytes <= 0))
        return -1; // sanity check
    int i = 0, ret_count = 0;
    for (i = 0; i < nbytes; i++)
    {
        if (((char *)buf)[i] == '\0')
            continue;
        ret_count++;
        putc(((char *)buf)[i]);
    }
    return ret_count; // return total byte we write
}
// void terminal_clear()
// Description:
// - Clears the terminal and restores some of its default state.
// Side Effects:
// - Clears the terminal screen.
// - Resets main_terminal.count and main_terminal.enter_pressed to 0.
// - Clears the terminal buffer.
// Input:
// - None.
// Return:
// - Void.
void terminal_clear()
{
    terminal_t *terminal = &main_terminal[current_terminal];
    clear();
    terminal->count = terminal->enter_pressed = 0; // restore some state
    memset((void *)terminal->terminal_buf, '\0', MAX_TERMINAL_SIZE);
}

// @@
int32_t switch_terminal(int32_t terminal_num) {
    if(terminal_num < 0 || terminal_num >= TERMINAL_NUMBER) return -1; // invalid `terminal_num`
    if(shared_user_vid_mem == NULL) return -1;  // vidmap error
    if(terminal_num == current_terminal) return 0;  // no action needed
    
    memcpy(main_terminal[current_terminal].video_mem_backup, shared_user_vid_mem, VIDEOMEM_SIZE);   // backup video mem of old terminal
    memcpy(shared_user_vid_mem, main_terminal[terminal_num].video_mem_backup, VIDEOMEM_SIZE);   // restore video mem backup form new terminal
    current_terminal = terminal_num;    // update `current_terminal`
    update_cursor(main_terminal[current_terminal].cursor_x, main_terminal[current_terminal].cursor_y);  // update curosor position in new terminal
    return 1;
}
