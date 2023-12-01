#include "terminal.h"
#include "lib.h"
#include "systemcall.h"

#define DEBUG
#include "debug.h"

terminal_t main_terminal[TERMINAL_NUMBER];
// terminal_t prev_terminal[TERMINAL_NUMBER];
uint8_t* video_mem[TERMINAL_NUMBER]={(uint8_t *)VIDEO_TERMINAL1,(uint8_t *)VIDEO_TERMINAL2,(uint8_t *)VIDEO_TERMINAL3};
volatile int32_t current_terminal = 0;

uint8_t *shared_user_vid_mem = (uint8_t*)VIDEO;

// void initialize_terminal()
// initializes the main terminal with default values.
// input:none.
// return:none
// side effect:init the main_terminal and enable the cursor
void initialize_terminal(int32_t terminal_num)
{
    terminal_t *terminal = &main_terminal[terminal_num];
    terminal->count = terminal->enter_pressed = terminal->tab_pressed = 0;
    terminal->cursor_x = terminal->cursor_y = 0;
    memset((void *)terminal->terminal_buf, '\0', MAX_TERMINAL_SIZE);
    enable_cursor(14, 15); // set cursor shape
    update_cursor(0, 0);   // set cursor position
    // @@
    terminal->video_mem_backup = video_mem[terminal_num];
    terminal->simulateKeyboard = 0;
}
// int32_t terminal_close(int32_t fd)
//.Closes the terminal.
// input:none.
// return:0
// side effect:Closes the terminal
int32_t terminal_close(int32_t fd)
{
    terminal_t *terminal = &main_terminal[current_terminal];
    // terminal_t *prev = &prev_terminal[current_terminal];
    memset((void *)terminal->terminal_buf, '\0', MAX_TERMINAL_SIZE);
    terminal->count = 0;
    // prev->count = 0;
    terminal->enter_pressed = 0;
    terminal->simulateKeyboard = 0;
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

    while(current_terminal != sche_index);
    terminal_t *terminal = &main_terminal[current_terminal];
    // terminal_t *prev = &prev_terminal[current_terminal];
    if ((!buf) || (nbytes <= 0))
        return -1; // sanity check
    int i = 0, ret_count = 0;
    terminal->enter_pressed = terminal->tab_pressed = 0;
    terminal->up_pressed = 0;
    terminal->down_pressed = 0;
    while (!(terminal->enter_pressed || terminal->up_pressed || terminal->down_pressed || terminal->tab_pressed))
        ;
    if (terminal->up_pressed)
    {
        while (terminal->count > 0)
            backspace();
        strcpy(buf, "^[[A");
        return 4;
    }
    else if (terminal->down_pressed)
    {
        while (terminal->count > 0)
            backspace();
        strcpy(buf, "^[[B");
        return 4;
    }

    for (i = 0; i < nbytes && i < READ_MAX_SIZE && terminal->terminal_buf[i] != '\0'; i++)
    {
        ((char *)buf)[i] = terminal->terminal_buf[i]; // read from previous buffer to the terminal buffer
        ret_count++;
        if ((terminal->terminal_buf[i] == '\n') || (terminal->terminal_buf[i] == '\t'))
            break; // when meeting \n, return
    }

    if (terminal->tab_pressed)
    {
        while (terminal->count > 0)
            backspace();
    }

    ((char *)buf)[ret_count] = '\0';
    terminal->count = 0;
    memset((void *)terminal->terminal_buf, '\0', MAX_TERMINAL_SIZE);

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
    uint8_t curCnt = main_terminal[current_terminal].count;
    if ((!buf) || (nbytes <= 0))
    {
        main_terminal[current_terminal].simulateKeyboard = 0;
        return -1; // sanity check
    }

    int i = 0, ret_count = 0;
    if (main_terminal[current_terminal].simulateKeyboard == 0)
    {
        for (i = 0; i < nbytes; i++)
        {
            if (((char *)buf)[i] == '\0')
                continue;
            ret_count++;
            putc(((char *)buf)[i]);
        }
    }
    else
    {
        ret_count = 0;
        for (i = curCnt; i < 127; i++)
        {
            if (ret_count > nbytes || ((char *)buf)[ret_count] == '\0')
                break;
            main_terminal[current_terminal].terminal_buf[i] = ((char *)buf)[ret_count];
            putc(((char *)buf)[ret_count]);
            ret_count++;
        }
        main_terminal[current_terminal].count += ret_count;
        main_terminal[current_terminal].terminal_buf[main_terminal[current_terminal].count] = '\0';
    }
    main_terminal[current_terminal].simulateKeyboard = 0;
    return ret_count; // return total byte we write
}

// TODO comment & switch terminal & simulate keyboard
int32_t terminal_ioctl(int32_t fd, int32_t request, void *buf)
{
    switch (request)
    {
    case SIMKB:
        main_terminal[current_terminal].simulateKeyboard = 1;
        int32_t len = strlen((const int8_t *)buf);
        terminal_write(fd, buf, len);
        break;
    default:
        break;
    }
    return 0;
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
int32_t switch_terminal(int32_t terminal_num)
{
    if (terminal_num < 0 || terminal_num >= TERMINAL_NUMBER)
        return -1; // invalid `terminal_num`
    if (shared_user_vid_mem == NULL)
        return -1; // vidmap error
    if (terminal_num == current_terminal)
        return 0; // no action needed

    memcpy(main_terminal[current_terminal].video_mem_backup, shared_user_vid_mem, VIDEOMEM_SIZE);      // backup video mem of old terminal
    memcpy(shared_user_vid_mem, main_terminal[terminal_num].video_mem_backup, VIDEOMEM_SIZE);          // restore video mem backup form new terminal
    current_terminal = terminal_num;                                                                   // update `current_terminal`
    update_cursor(main_terminal[current_terminal].cursor_x, main_terminal[current_terminal].cursor_y); // update curosor position in new terminal

    return 1;
}
