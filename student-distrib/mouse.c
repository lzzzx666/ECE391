#include "mouse.h"
#include "i8259.h"
#include "systemcall.h"
#include "page.h"
#include "lib.h"

int mouse_x = 0, mouse_y = 0;
mouse_packet_t mouse_bytes[3];
int cycle = 0;

void mouse_wait(uint8_t type)
{
    uint32_t timeout = 100000;
    if(!type)
    {
        while(--timeout)
        {
            if((inb(STATUS_PORT) & 1) == MOUSE_BBIT)
            {
                return;
            }
        }
        return;
    }
    else
    {
        while(--timeout)
        {
            if(!((inb(STATUS_PORT) & MOUSE_ABIT)))
            {
                return;
            }
        }
        return;
    }
}


void mouse_init(void){
    
    mouse_wait(1);
    outb(0xA8, STATUS_PORT);
    mouse_wait(1);
    outb(0x20, STATUS_PORT);
    uint8_t status_byte;
    mouse_wait(0);
    status_byte = (inb(MOUSE_PORT) | 2);
    mouse_wait(1);
    outb(MOUSE_PORT, STATUS_PORT);
    mouse_wait(1);
    outb(status_byte, MOUSE_PORT);
    mouse_write(0xF6);
    mouse_read();
    mouse_write(0xF4);
    mouse_read();
    cycle = 1;
    enable_irq(MOUSE_IRQ);
}

void mouse_handler(void)
{
    cli();
    send_eoi(MOUSE_IRQ);
    if((mouse_bytes[0].x_overflow == 0 && mouse_bytes[0].y_overflow == 0) && mouse_bytes[0].always1 == 0)
    {
        cycle = 0;
    }
    mouse_bytes[cycle].val= inb(MOUSE_PORT);
    cycle++;
    int8_t x, y;
    if(cycle == 3)
    {
        cycle = 0;
        if(mouse_bytes[0].x_overflow || mouse_bytes[0].y_overflow)
        {
            return; //overflow
        }
        else 
        {
            x = mouse_bytes[1].val, y = mouse_bytes[2].val;
        } 
        change_color(mouse_x, mouse_y, ATTRIB);
        if(mouse_x + x / 10 >= 0 && mouse_x + x / 10 < NUM_COLS)
        {
            mouse_x += x / 10;
        }
        if(mouse_y - y / 10 >= 0 && mouse_y - y/ 10 < NUM_ROWS)
        {
            mouse_y -= y / 10;
        }
        change_color(mouse_x, mouse_y, 0x70);
    }
    sti();
}

int32_t mouse_write (unsigned char data)
{
    mouse_wait(1);
    outb(MOUSE_WRITE, STATUS_PORT);
    mouse_wait(1);
    outb(data, MOUSE_PORT);
    return 0;
}


int32_t mouse_read (void)
{
    mouse_wait(0);
    return inb(MOUSE_PORT);
}

int32_t mouse_open ()
{
    return 0;
}

int32_t mouse_close ()
{
    return 0;
}
