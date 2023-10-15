/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/* Initialize the 8259 PIC */
void i8259_init(void) {

    outb(0xFF, MASTER_8259_PORT_DATA); // set all mask to 1
    outb(0xFF, SLAVE_8259_PORT_DATA);
    outb(ICW1, MASTER_8259_PORT_CMD);
    outb(ICW1, SLAVE_8259_PORT_CMD); //ICW1
    outb(ICW2_MASTER, MASTER_8259_PORT_DATA);
    outb(ICW2_SLAVE, SLAVE_8259_PORT_DATA); //ICW2
    outb(ICW3_MASTER, MASTER_8259_PORT_DATA);
    outb(ICW3_SLAVE, SLAVE_8259_PORT_DATA); //ICW3
    outb(ICW4, MASTER_8259_PORT_DATA); //ICW4
    outb(ICW4, SLAVE_8259_PORT_DATA);
    master_mask = 0xFB; //bit 2 set to zero to open the PIC2 for slave
    slave_mask = 0xFF; 
    enable_irq(2); //open the PIC2 for slave

}

/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num) {
    if(irq_num > 15) return;
    uint16_t port;
    uint32_t line;
    uint8_t value;
    if(irq_num < 8) //from master
    {
        port = MASTER_8259_PORT_DATA;
        line = irq_num;
        master_mask = inb(port) & (~(1 << line)); 
        value = master_mask;
    }
    else //from slave
    {
        port = SLAVE_8259_PORT_DATA;
        line = irq_num - 8;
        slave_mask = inb(port) & (~(1 << line)); 
        value = slave_mask;
    }
    outb(value, port);
}

/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num) {
    if(irq_num > 15) return;
    uint16_t port;
    uint32_t line;
    uint8_t value;
    if(irq_num < 8) // from master
    {
        port = MASTER_8259_PORT_DATA;
        line = irq_num;
        master_mask = inb(port) | (1 << line);
        value = master_mask;
    }
    else // from slave
    {
        port = SLAVE_8259_PORT_DATA;
        line = irq_num - 8;
        slave_mask = inb(port) | (1 << line);
        value = slave_mask;
    }
    outb(value, port);
}

/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num) {
    if(irq_num > 15) return;
    if(irq_num < 8)    
    {
        outb(EOI | irq_num, MASTER_8259_PORT_CMD);
    }
    else
    {
        outb(EOI | (irq_num - 8), SLAVE_8259_PORT_CMD);
        outb(EOI | 2, MASTER_8259_PORT_CMD);
    }
    return;
}
