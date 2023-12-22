/* tuxctl-ioctl.c
 *
 * Driver (skeleton) for the mp2 tuxcontrollers for ECE391 at UIUC.
 *
 * Mark Murphy 2006
 * Andrew Ofisher 2007
 * Steve Lumetta 12-13 Sep 2009
 * Puskar Naha 2013
 */

#include <asm/current.h>
#include <asm/uaccess.h>

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/file.h>
#include <linux/miscdevice.h>
#include <linux/kdev_t.h>
#include <linux/tty.h>
#include <linux/spinlock.h>

#include "tuxctl-ld.h"
#include "tuxctl-ioctl.h"
#include "mtcp.h"

#define debug(str, ...) \
	printk(KERN_DEBUG "%s: " str, __FUNCTION__, ## __VA_ARGS__)

void set_LED(struct tty_struct* tty,unsigned long arg);
int set_buttons(unsigned long* arg);
int init_tux(struct tty_struct* tty);
void bioc_event(unsigned b, unsigned c);
void reset_mtcp(struct tty_struct* tty);
static unsigned int ack; //acknowledge
static unsigned long LED_backup; //used to store the saved LED
static unsigned long button_value;
static spinlock_t lock_button;	
static int flag_button;
unsigned char LED_table[16] = {0xE7, 0x06, 0xCB, 0x8F,0x2E, 0xAD, 0xED, 0x86, 0xEF, 0xAF, 0xEE, 0x6D, 0xE1, 0x4F, 0xE9, 0xE8};
/************************ Protocol Implementation *************************/

/* tuxctl_handle_packet()
 * IMPORTANT : Read the header for tuxctl_ldisc_data_callback() in 
 * tuxctl-ld.c. It calls this function, so all warnings there apply 
 * here as well.
 */
void tuxctl_handle_packet (struct tty_struct* tty, unsigned char* packet)
{
    unsigned a, b, c;

    a = packet[0]; /* Avoid printk() sign extending the 8-bit */
    b = packet[1]; /* values when printing them. */
    c = packet[2];
	switch (a)
	{										
		case MTCP_ACK:		
			ack = 0;
			break;
		case MTCP_BIOC_EVENT:	
			bioc_event(b,c);
			break;
		case MTCP_RESET:	
			reset_mtcp(tty);
			break;
		default:			
			return;
	}
    /*printk("packet : %x %x %x\n", a, b, c); */
}

/*
 * reset_mtcp
 *   DESCRIPTION: reset the mtcp, including re-init controller and set LED 
 *   INPUTS: struct tty_struct* tty
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: setting LED and re-init controller
 */ 
void reset_mtcp(struct tty_struct* tty)
{
	unsigned char buf[2] = {MTCP_BIOC_ON, MTCP_LED_SET};
	tuxctl_ldisc_put(tty, buf, 2);
	set_LED(tty, LED_backup); //set LED based on the state before the reset
	ack = 0;
}
/*
 * bioc_event
 *   DESCRIPTION: handle the bioc event and reset the button
 *   INPUTS: unsigned b, unsigned c
 *   OUTPUTS: none
 *   RETURN VALUE: none
 */ 
void bioc_event(unsigned b, unsigned c)
{
	if (b == 0x80 && c == 0x80)	return;
	spin_lock_irqsave(&lock_button, flag_button);
	button_value = b & MASK_LOWER4BIT;
	c = c & MASK_LOWER4BIT;
	button_value |= (c & 0x01) << 4; //extract up
	button_value |= (c & 0x08) << 4; //extract right
	button_value |= (c & 0x02) << 5; //extract left
	button_value |= (c & 0x04) << 3; //extract down
	spin_unlock_irqrestore(&lock_button, flag_button);
}


/******** IMPORTANT NOTE: READ THIS BEFORE IMPLEMENTING THE IOCTLS ************
 *                                                                            *
 * The ioctls should not spend any time waiting for responses to the commands *
 * they send to the controller. The data is sent over the serial line at      *
 * 9600 BAUD. At this rate, a byte takes approximately 1 millisecond to       *
 * transmit; this means that there will be about 9 milliseconds between       *
 * the time you request that the low-level serial driver send the             *
 * 6-byte SET_LEDS packet and the time the 3-byte ACK packet finishes         *
 * arriving. This is far too long a time for a system call to take. The       *
 * ioctls should return immediately with success if their parameters are      *
 * valid.                                                                     *
 *                                                                            *
 ******************************************************************************/
int 
tuxctl_ioctl (struct tty_struct* tty, struct file* file, 
	      unsigned cmd, unsigned long arg)
{
    switch (cmd) {
	case TUX_INIT: return init_tux(tty);
	case TUX_BUTTONS: return set_buttons((unsigned long*) arg);
	case TUX_SET_LED: set_LED(tty, arg); return 0;
	case TUX_LED_ACK: return 0;
	case TUX_LED_REQUEST: return 0;
	case TUX_READ_LED: return 0;
	default:
	    return -EINVAL;
    }
}


//some helper functions

/*
 * init_tux
 *   DESCRIPTION: init the tux, clear the button value and led backup
 *   INPUTS: struct tty_struct* tty
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: change some state value
 */ 
int init_tux(struct tty_struct* tty)
{
	unsigned char buf[2] = {MTCP_BIOC_ON, MTCP_LED_SET};
	tuxctl_ldisc_put(tty, buf, 2); //init buffer to the device
	spin_lock_init(&lock_button);
	LED_backup = 0; //init some state value
	button_value = 0;
	ack = 0;
	return 0;
}
/*
 * init_tux
 *   DESCRIPTION: copy button value to user space 
 *   INPUTS: unsigned long arg (in fact a pointer)
 *   OUTPUTS: none
 *   RETURN VALUE: if success return 0 else return -EINVAL
 *   SIDE EFFECTS: none
 */ 
int set_buttons(unsigned long* arg)
{
	if(!arg) return -EINVAL;
	spin_lock_irqsave(&lock_button, flag_button);
	*arg = button_value;	/* set button */
	spin_unlock_irqrestore(&lock_button, flag_button);		
	return 0;
}

#define LED_0123_MASK 0x00010000
#define DECIMAL_MASK 0x01000000
/*
 * set_LED
 *   DESCRIPTION: set the led based on the pointer from ioctl 
 *   INPUTS: struct tty_struct* tty,unsigned long arg
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: set the tux led value
 */ 
void set_LED(struct tty_struct* tty,unsigned long arg)
{
	unsigned char buf[6] = {MTCP_LED_SET, 0x0F, 0, 0, 0, 0};
	int i;
	for(i = 0; i < 4; i++)
	{
		if(arg & (LED_0123_MASK << i))// test if corrsepondin led is on
		{
			buf[i + 2] = LED_table[(arg >> (4 * i)) & 0xF]; //using led table
			if (arg & ( DECIMAL_MASK << i)) //test if decimal point exist
			{
				buf[i + 2] |= 0x10; //update decimal point
			}
		}
	}
	LED_backup = arg; //store the last led
	tuxctl_ldisc_put(tty, buf, 6);
}
