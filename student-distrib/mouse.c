// #include "mouse.h"
// #include "i8259.h"
// #include "systemcall.h"
// #include "page.h"
// #include "lib.h"

// int mouse_x = 0, mouse_y = 0;


// void mouse_wait(uint8_t type)
// {
//     uint32_t timeout = 100000;
//     if(!type)
//     {
//         while(--timeout)
//         {
//             if((inb(STATUS_PORT) & 1) == MOUSE_BBIT)
//             {
//                 return;
//             }
//         }
//         return;
//     }
//     else
//     {
//         while(--timeout)
//         {
//             if(!((inb(STATUS_PORT) & MOUSE_ABIT)))
//             {
//                 return;
//             }
//         }
//         return;
//     }
// }


// void mouse_init(void){
//     enable_irq(MOUSE_IRQ);
//     mouse_wait(1);
//     outb(0xA8, STATUS_PORT);
//     mouse_wait(1);
//     outb(0x20, STATUS_PORT);
//     uint8_t status_byte;
//     mouse_wait(0);
//     status_byte = (inb(MOUSE_PORT) | 2);
//     mouse_wait(1);
//     outb(MOUSE_PORT, STATUS_PORT);
//     mouse_wait(1);
//     outb(status_byte, MOUSE_PORT);
//     mouse_write(0xF6);
//     mouse_read();
//     mouse_write(0xF4);
//     mouse_read();
// }

// void mouse_handler(void)
// {
//     cli();
//     send_eoi(MOUSE_IRQ);
//     mouse_packet_t packet;
//     packet.val = mouse_read();
// 	if (packet.x_overflow || packet.y_overflow || !packet.always1)
//         return;
//     int32_t x_move = mouse_read();
//     int32_t y_move = mouse_read();

//     if (packet.x_sign)
//         x_move = 0xFFFFFF00 | x_move;
//     if (packet.y_sign)
//         y_move = 0xFFFFFF00 | y_move;
    
//     x_move /= 5;
//     y_move /= 5;

//     change_color(mouse_x, mouse_y, ATTRIB);

//     mouse_x += x_move;
//     mouse_y -= y_move;

//     if(mouse_x < 0)
//         mouse_x = 0;
//     if(mouse_y < 0)
//         mouse_y = 0;
//     if(mouse_x > NUM_COLS - 1)
//         mouse_x = NUM_COLS - 1;
//     if(mouse_y > NUM_ROWS - 1)
//         mouse_y = NUM_ROWS - 1;

//     change_color(mouse_x, mouse_y, 0xF7);

//     sti();
// }

// int32_t mouse_write (unsigned char data)
// {
//     mouse_wait(1);
//     outb(MOUSE_WRITE, STATUS_PORT);
//     mouse_wait(1);
//     outb(data, MOUSE_PORT);
//     return 0;
// }


// int32_t mouse_read (void)
// {
//     mouse_wait(0);
//     return inb(MOUSE_PORT);
// }

// int32_t mouse_open ()
// {
//     return 0;
// }

// int32_t mouse_close ()
// {
//     return 0;
// }
