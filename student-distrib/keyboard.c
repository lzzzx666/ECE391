#include "keyboard.h"
#include "rtc.h"

char scan_code_set[NUM_SCANCODES] = {
   '\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\0', '\0',
	'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\0', '\0', 'a', 's',
	'd', 'f', 'g', 'h', 'j', 'k', 'l' , ';', '\'', '`', '\0', '\\', 'z', 'x', 'c', 'v', 
	'b', 'n', 'm',',', '.', '/', '\0', '*', '\0', ' ', '\0'
};
char scan_code_set_shift[NUM_SCANCODES] = 
{
    '\0', '\0', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\0', '\0',
	'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\0', '\0', 'A', 'S',
	'D', 'F', 'G', 'H', 'J', 'K', 'L' , ':', '"', '~', '\0', '|', 'Z', 'X', 'C', 'V', 
	'B', 'N', 'M', '<', '>', '?', '\0', '*', '\0', ' ', '\0'
};
char scan_code_set_caps[NUM_SCANCODES] = 
{
    '\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\0', '\0',
	'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\0', '\0', 'A', 'S',
	'D', 'F', 'G', 'H', 'J', 'K', 'L' , ';', '\'', '`', '\0', '\\', 'Z', 'X', 'C', 'V', 
	'B', 'N', 'M', ',', '.', '/', '\0', '*', '\0', ' ', '\0'
};
char scan_code_set_capsandshift[NUM_SCANCODES] = 
{
   '\0', '\0', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\0', '\0',
	'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '{', '}', '\0', '\0', 'a', 's',
	'd', 'f', 'g', 'h', 'j', 'k', 'l' , ':', '"', '~', '\0', '\\', 'z', 'x', 'c', 'v', 
	'b', 'n', 'm', '<', '>', '?', '\0', '*', '\0', ' ', '\0'
};

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
    if(scan_code >= NUM_SCANCODES)
    {
        send_eoi(KEYBOARD_IRQ);
        sti();
        return;
    }
    if (scan_code == TEST_RTC_HOTKEY)
        rtc_test_event();
    switch (scan_code)
    {
        case TAB: break;
        case ENTER: break;
        case CAPS_LOCK: break;
        case LSHIFT_MAKE: break;
        case LSHIFT_BREAK: break;
        case RSHIFT_MAKE: break;
        case RSHIFT_BREAK: break;
        case CTRL_MAKE: break;
        case CTRL_BREAK: break;
        case ALT_MAKE: break;
        case ALT_BREAK: break;
        case F1: break;
        case F2: break;
        case F3: break; 
        default:
            ascii = scan_code_set[scan_code];
            if((ascii >= 'a' && ascii <= 'z') || (ascii >= '0' && ascii <= '9'))   
                putc(ascii);
            break;
    }
    send_eoi(KEYBOARD_IRQ);
    
    sti();
}
