#include "keyboard.h"
#include "rtc.h"
#include "terminal.h"
#include "lib.h"
#include "systemcall.h"

static int shift_pressed = 0; // 0 stands for not pushing
static int capslock_pressed = 0;
static int alt_pressed = 0;
static int ctrl_pressed = 0;
char keyboard_buffer[KEY_BOARD_BUF_SIZE];
extern terminal_t main_terminal, prev_terminal;

char scan_code_set[NUM_SCANCODES] = {
    '\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\0', '\0',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\0', '\0', 'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', '\0', '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm', ',', '.', '/', '\0', '*', '\0', ' ', '\0'};
char scan_code_set_shift[NUM_SCANCODES] =
    {
        '\0', '\0', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\0', '\0',
        'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\0', '\0', 'A', 'S',
        'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', '\0', '|', 'Z', 'X', 'C', 'V',
        'B', 'N', 'M', '<', '>', '?', '\0', '*', '\0', ' ', '\0'};
char scan_code_set_caps[NUM_SCANCODES] =
    {
        '\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\0', '\0',
        'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\0', '\0', 'A', 'S',
        'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`', '\0', '\\', 'Z', 'X', 'C', 'V',
        'B', 'N', 'M', ',', '.', '/', '\0', '*', '\0', ' ', '\0'};
char scan_code_set_capsandshift[NUM_SCANCODES] =
    {
        '\0', '\0', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\0', '\0',
        'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '{', '}', '\0', '\0', 'a', 's',
        'd', 'f', 'g', 'h', 'j', 'k', 'l', ':', '"', '~', '\0', '\\', 'z', 'x', 'c', 'v',
        'b', 'n', 'm', '<', '>', '?', '\0', '*', '\0', ' ', '\0'};

// Description: Initializes the keyboard by enabling its corresponding interrupt request line (IRQ).
// Parameters:None.
// Return Value:None.
// Side Effects: The interrupt request (IRQ) for the keyboard is enabled, allowing the system to handle keyboard interrupts

void init_keyboard()
{
    enable_irq(KEYBOARD_IRQ);
}

// Description: Handles keyboard interrupts by reading a scan code from the keyboard data port and then, based on the scan code
// Parameters:None.
// Return Value:None.
// Side Effects: Reads a byte from the KEYBOARD_DATA_PORT.
// If the scan code corresponds to an alphanumeric character (a-z, 0-9), the character will be printed to the screen.

void keyboard_handler()
{
    cli();
    unsigned char scan_code = inb(KEYBOARD_DATA_PORT);
    char ascii;
    uint8_t user_interrupt = 0;
    pcb_t *cur_pcb;
    switch (scan_code)
    {
    case TAB:
        break;
    case BACKSPACE:
        if (main_terminal.count > 0)
        {
            main_terminal.terminal_buf[--main_terminal.count] = '\0'; // overwrite the original content with '\0'
            backspace();                                              // change screen x and y
        }
        break;
    case ENTER:
        main_terminal.terminal_buf[main_terminal.count++] = '\n';   // add a \n at the end
        main_terminal.enter_pressed = 1;                            // notify the main_terminal
        prev_terminal = main_terminal;                              // store the previous terminal
        main_terminal.terminal_buf[main_terminal.count = 0] = '\0'; // restore count
        memset((void *)main_terminal.terminal_buf, '\0', MAX_TERMINAL_SIZE);
        putc('\n');
        // terminal_read(0, keyboard_buffer, main_terminal.count);
        break;
    case CAPS_LOCK:
        capslock_pressed = 1 - capslock_pressed;
        break;
    case LSHIFT_MAKE:
        shift_pressed = 1;
        break;
    case LSHIFT_BREAK:
        shift_pressed = 0;
        break;
    case RSHIFT_MAKE:
        shift_pressed = 1;
        break;
    case RSHIFT_BREAK:
        shift_pressed = 0;
        break;
    case CTRL_MAKE:
        ctrl_pressed = 1;
        break;
    case CTRL_BREAK:
        ctrl_pressed = 0;
        break;
    case ALT_MAKE:
        alt_pressed = 1;
        break;
    case ALT_BREAK:
        alt_pressed = 0;
        break;
    case F1:
        break;
    case F2:
        break;
    case F3:
        break;
    default:
        if (scan_code >= NUM_SCANCODES)
            break;
        if (shift_pressed && capslock_pressed) // four different scan_code set
            ascii = scan_code_set_capsandshift[scan_code];
        else if (shift_pressed)
            ascii = scan_code_set_shift[scan_code];
        else if (capslock_pressed)
            ascii = scan_code_set_caps[scan_code];
        else
            ascii = scan_code_set[scan_code];

        if (ascii == '\0')
            break;
        else if (ctrl_pressed && (ascii == 'L' || ascii == 'l')) // clear the screen
        {
            terminal_clear();
            break;
        }
        else if (ctrl_pressed && (ascii == 'C' || ascii == 'c')) // clear the screen
        {
            user_interrupt = 1;
            break;
        }
        else
        {
            if (main_terminal.count < READ_MAX_SIZE - 1)
            {
                main_terminal.terminal_buf[main_terminal.count++] = ascii; // default condition
                putc(ascii);
            }
        }
    }

    send_eoi(KEYBOARD_IRQ);

    sti();
    if (user_interrupt)
    {
        cur_pcb = get_current_pcb();
        if (cur_pcb->pid != 0)
        {
            halt(1);
        }
    }
}

