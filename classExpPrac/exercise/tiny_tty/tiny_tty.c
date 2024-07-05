/*
 * Tiny TTY driver
 *
 * Copyright (C) 2002-2004 Greg Kroah-Hartman (greg@kroah.com)
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, version 2 of the License.
 *
 * This driver shows how to create a minimal tty driver.  It does not rely on
 * any backing hardware, but creates a timer that emulates data being received
 * from some kind of hardware.
 */

#include <linux/version.h>
#define VERSION_CODE(ver,rel,seq)       ((ver << 16) | (rel << 8) | seq)
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/wait.h>
#include <linux/tty.h>
#include <linux/tty_driver.h>
#include <linux/tty_flip.h>
#include <linux/serial.h>
#include <linux/sched.h>
#include <asm/uaccess.h>

#ifdef __UART__ // jiunnder2000@yahoo.com.tw
#include <asm/io.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#define PORT_COM1       0x3f8
#define PORT_COM2       0x2f8
#define PORT_COM3       0x3e8
#define PORT_COM4       0x2e8
#define COM_IRQ		4

#define REG_RHR         0
#define REG_THR         0
#define REG_IER         1
#define REG_IIR         2
#define REG_LCR         3
#define REG_MCR         4
#define REG_LSR         5
#define REG_MSR         6
#define REG_SCRATCH     7

//LCR-related constants
#define PARITY_NONE     0
#define PARITY_ODD      8
#define PARITY_EVEN     24
#define PARITY_MARK     20
#define PARITY_SPACE    28

#define STOP_ONE        0
#define STOP_TWO        4

#define BITS_5          0
#define BITS_6          1
#define BITS_7          2
#define BITS_8          3

#define DLR_ON          128
#define COM1_IRQ_ON     8
#define REC_IRQ_ON      1

// Define the LSR bitmap
#define DATA_READY		0x01
#define READ_OVERRUN		0x02
#define PARITY_ERROR		0x04
#define FRAME_ERROR		0x08
#define BREAK_INTERRUPT		0x10
#define THRE_EMPTY		0x20
#define TRAMSMITTER_EMPTY	0x40
#define PARITY_FRAMING_ERROR	0x80

// Define the IIR bitmap
#define IIR_RECEIVED_AVALIABLE	0x04


#endif // End of --- jiunnder2000@yahoo.com.tw


#define DRIVER_VERSION "v2.0"
#define DRIVER_AUTHOR "Greg Kroah-Hartman <greg@kroah.com>"
#define DRIVER_DESC "Tiny TTY driver"

/* Module information */
MODULE_AUTHOR( DRIVER_AUTHOR );
MODULE_DESCRIPTION( DRIVER_DESC );
MODULE_LICENSE("GPL");

#define DELAY_TIME		HZ * 2	/* 2 seconds per character */
#define TINY_DATA_CHARACTER	't'

#define TINY_TTY_MAJOR		240	/* experimental range */
#define TINY_TTY_MINORS		4	/* only have 4 devices */

struct tiny_serial {
	#if (LINUX_VERSION_CODE >= VERSION_CODE(3,7,0))
	struct tty_port ttyPort;
	#endif
	struct tty_struct	*tty;		/* pointer to the tty for this device */
	int			open_count;	/* number of times this port has been opened */
	struct semaphore	sem;		/* locks this structure */
	struct timer_list	*timer;

	/* for tiocmget and tiocmset functions */
	int			msr;		/* MSR shadow */
	int			mcr;		/* MCR shadow */

	/* for ioctl fun */
	struct serial_struct	serial;
	wait_queue_head_t	wait;
	struct async_icount	icount;
};

static struct tiny_serial *tiny_table[TINY_TTY_MINORS];	/* initially all NULL */
static struct tty_driver *tiny_tty_driver;

#ifndef __UART__ // jiunnder2000@yahoo.com.tw
static void tiny_timer(unsigned long timer_data)
{
	struct tiny_serial *tiny = (struct tiny_serial *)timer_data;
	struct tty_struct *tty;
	char data[1] = {TINY_DATA_CHARACTER};
	int data_size = 1, i;

	if (!tiny)
		return;

	tty = tiny->tty;

	/* send the data to the tty layer for users to read.  This doesn't
	 * actually push the data through unless tty->low_latency is set */
	for (i = 0; i < data_size; ++i) {
#if (LINUX_VERSION_CODE < VERSION_CODE(3,9,0))
		tty_insert_flip_char(tty, data[i], TTY_NORMAL);
#else
		tty_insert_flip_char(&tiny->ttyPort, data[i], TTY_NORMAL);
#endif
	}
#if (LINUX_VERSION_CODE < VERSION_CODE(3,9,0))
	tty_flip_buffer_push(tty);
#else
	tty_flip_buffer_push(&tiny->ttyPort);
#endif

	/* resubmit the timer again */
	tiny->timer->expires = jiffies + DELAY_TIME;
	add_timer(tiny->timer);
}
#else
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,18) //jiunnder2000@yahoo.com.tw
static irqreturn_t uart_interrupt(int irq, void *dev_id)
#else
static irqreturn_t uart_interrupt(int irq, void *dev_id, struct pt_regs *regs)
#endif
{
	//To do: recive data from interrupt

	return IRQ_HANDLED;
}
#endif 


static int tiny_open(struct tty_struct *tty, struct file *file)
{
	struct tiny_serial *tiny;
#ifndef __UART__ // jiunnder2000@yahoo.com.tw
	struct timer_list *timer;
#endif
	int index;

	/* initialize the pointer in case something fails */
	tty->driver_data = NULL;

	/* get the serial object associated with this tty pointer */
	index = tty->index;
	tiny = tiny_table[index];
#if 0 // jiunnder2000@yahoo.com.tw, move the tiny_table[] initial to init_module()
	if (tiny == NULL) {
		/* first time accessing this device, let's create it */
		tiny = kmalloc(sizeof(*tiny), GFP_KERNEL);
		if (!tiny)
			return -ENOMEM;

		sema_init(&tiny->sem, 1);

		tiny->open_count = 0;
		tiny->timer = NULL;
		tiny_table[index] = tiny;
	}
#endif

	down(&tiny->sem);

	/* save our structure within the tty structure */
	tty->driver_data = tiny;
	tiny->tty = tty;

	++tiny->open_count;
	if (tiny->open_count == 1) {
		/* this is the first time this port is opened */
		/* do any hardware initialization needed here */

#ifndef __UART__ // jiunnder2000@yahoo.com.tw
		/* create our timer and submit it */
		if (!tiny->timer) {
			timer = kmalloc(sizeof(*timer), GFP_KERNEL);
			if (!timer) {
				up(&tiny->sem);
				return -ENOMEM;
			}
			tiny->timer = timer;
		}
		init_timer(tiny->timer); //jiunnder2000@yahoo.com.tw
		tiny->timer->data = (unsigned long )tiny;
		tiny->timer->expires = jiffies + DELAY_TIME;
		tiny->timer->function = tiny_timer;
		add_timer(tiny->timer);
#endif
	}

	up(&tiny->sem);
	return 0;
}

static void do_close(struct tiny_serial *tiny)
{
	down(&tiny->sem);

	if (!tiny->open_count) {
		/* port was never opened */
		goto exit;
	}

	--tiny->open_count;
	if (tiny->open_count <= 0) {
		/* The port is being closed by the last user. */
		/* Do any hardware specific stuff here */

#ifndef __UART__ // jiunnder2000@yahoo.com.tw
		/* shut down our timer */
		del_timer(tiny->timer);
#endif
	}
exit:
	up(&tiny->sem);
}

static void tiny_close(struct tty_struct *tty, struct file *file)
{
	struct tiny_serial *tiny = tty->driver_data;

	if (tiny)
		do_close(tiny);
}	

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,18) // jiunnder2000@yahoo.com.tw
static int tiny_write(struct tty_struct *tty, 
		      const unsigned char *buffer, int count)
#else
static int tiny_write(struct tty_struct *tty, int from_user, 
		      const unsigned char *buffer, int count)
#endif
{
	struct tiny_serial *tiny = tty->driver_data;
	int i;
	int retval = -EINVAL;

	if (!tiny)
		return -ENODEV;

	down(&tiny->sem);

	if (!tiny->open_count)
		/* port was not opened */
		goto exit;

	/* fake sending the data out a hardware port by
	 * writing it to the kernel debug log.
	 */
#ifndef __UART__ // jiunnder2000@yahoo.com.tw
	for (i = 0; i < count; ++i)
		printk("%02x ", buffer[i]);
	printk("\n");
#else // jiunnder2000@yahoo.com.tw, To do: write data into UART

#endif
	retval=i;

exit:
	up(&tiny->sem);
	return retval;
}

static unsigned int tiny_write_room(struct tty_struct *tty) 
{
	struct tiny_serial *tiny = tty->driver_data;
	int room = -EINVAL;

	if (!tiny)
		return -ENODEV;

	down(&tiny->sem);
	
	if (!tiny->open_count) {
		/* port was not opened */
		goto exit;
	}

	/* calculate how much room is left in the device */
	room = 255;

exit:
	up(&tiny->sem);
	return room;
}

#define RELEVANT_IFLAG(iflag) ((iflag) & (IGNBRK|BRKINT|IGNPAR|PARMRK|INPCK))

#if LINUX_VERSION_CODE > KERNEL_VERSION(6,0,0)
static void tiny_set_termios(struct tty_struct *tty, const struct ktermios *old_termios)
#elif LINUX_VERSION_CODE > KERNEL_VERSION(2,6,18)
static void tiny_set_termios(struct tty_struct *tty, struct ktermios *old_termios)
#else
static void tiny_set_termios(struct tty_struct *tty, struct termios *old_termios)
#endif
{
	unsigned int cflag;
#if (LINUX_VERSION_CODE < VERSION_CODE(3,7,0))
	cflag = tty->termios->c_cflag;
#else
	cflag = tty->termios.c_cflag;
#endif

	/* check that they really want us to change something */
	if (old_termios) {
#if (LINUX_VERSION_CODE < VERSION_CODE(3,7,0))
		if ((cflag == old_termios->c_cflag) &&
		    (RELEVANT_IFLAG(tty->termios->c_iflag) == RELEVANT_IFLAG(old_termios->c_iflag)))
#else
		if ((cflag == old_termios->c_cflag) &&
		    (RELEVANT_IFLAG(tty->termios.c_iflag) == RELEVANT_IFLAG(old_termios->c_iflag)))
#endif
		{
			printk(KERN_DEBUG " - nothing to change...\n");
			return;
		}
	}

	/* get the byte size */
	switch (cflag & CSIZE) {
		case CS5:
			printk(KERN_DEBUG " - data bits = 5\n");
			break;
		case CS6:
			printk(KERN_DEBUG " - data bits = 6\n");
			break;
		case CS7:
			printk(KERN_DEBUG " - data bits = 7\n");
			break;
		default:
		case CS8:
			printk(KERN_DEBUG " - data bits = 8\n");
			break;
	}
	
	/* determine the parity */
	if (cflag & PARENB)
		if (cflag & PARODD)
			printk(KERN_DEBUG " - parity = odd\n");
		else
			printk(KERN_DEBUG " - parity = even\n");
	else
		printk(KERN_DEBUG " - parity = none\n");

	/* figure out the stop bits requested */
	if (cflag & CSTOPB)
		printk(KERN_DEBUG " - stop bits = 2\n");
	else
		printk(KERN_DEBUG " - stop bits = 1\n");

	/* figure out the hardware flow control settings */
	if (cflag & CRTSCTS)
		printk(KERN_DEBUG " - RTS/CTS is enabled\n");
	else
		printk(KERN_DEBUG " - RTS/CTS is disabled\n");
	
	/* determine software flow control */
	/* if we are implementing XON/XOFF, set the start and 
	 * stop character in the device */
	if (I_IXOFF(tty) || I_IXON(tty)) {
		unsigned char stop_char  = STOP_CHAR(tty);
		unsigned char start_char = START_CHAR(tty);

		/* if we are implementing INBOUND XON/XOFF */
		if (I_IXOFF(tty))
			printk(KERN_DEBUG " - INBOUND XON/XOFF is enabled, "
				"XON = %2x, XOFF = %2x", start_char, stop_char);
		else
			printk(KERN_DEBUG" - INBOUND XON/XOFF is disabled");

		/* if we are implementing OUTBOUND XON/XOFF */
		if (I_IXON(tty))
			printk(KERN_DEBUG" - OUTBOUND XON/XOFF is enabled, "
				"XON = %2x, XOFF = %2x", start_char, stop_char);
		else
			printk(KERN_DEBUG" - OUTBOUND XON/XOFF is disabled");
	}

	/* get the baud rate wanted */
	printk(KERN_DEBUG " - baud rate = %d", tty_get_baud_rate(tty));
}

/* Our fake UART values */
#define MCR_DTR		0x01
#define MCR_RTS		0x02
#define MCR_LOOP	0x04
#define MSR_CTS		0x08
#define MSR_CD		0x10
#define MSR_RI		0x20
#define MSR_DSR		0x40

#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,39))
static int tiny_tiocmget(struct tty_struct *tty, struct file *file)
#else
static int tiny_tiocmget(struct tty_struct *tty)
#endif
{
	struct tiny_serial *tiny = tty->driver_data;

	unsigned int result = 0;
	unsigned int msr = tiny->msr;
	unsigned int mcr = tiny->mcr;

	result = ((mcr & MCR_DTR)  ? TIOCM_DTR  : 0) |	/* DTR is set */
             ((mcr & MCR_RTS)  ? TIOCM_RTS  : 0) |	/* RTS is set */
             ((mcr & MCR_LOOP) ? TIOCM_LOOP : 0) |	/* LOOP is set */
             ((msr & MSR_CTS)  ? TIOCM_CTS  : 0) |	/* CTS is set */
             ((msr & MSR_CD)   ? TIOCM_CAR  : 0) |	/* Carrier detect is set*/
             ((msr & MSR_RI)   ? TIOCM_RI   : 0) |	/* Ring Indicator is set */
             ((msr & MSR_DSR)  ? TIOCM_DSR  : 0);	/* DSR is set */

	return result;
}

#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,39))
static int tiny_tiocmset(struct tty_struct *tty, struct file *file,
                         unsigned int set, unsigned int clear)
#else
static int tiny_tiocmset(struct tty_struct *tty, unsigned int set, unsigned int clear)
#endif
{
	struct tiny_serial *tiny = tty->driver_data;
	unsigned int mcr = tiny->mcr;

	if (set & TIOCM_RTS)
		mcr |= MCR_RTS;
	if (set & TIOCM_DTR)
		mcr |= MCR_RTS;

	if (clear & TIOCM_RTS)
		mcr &= ~MCR_RTS;
	if (clear & TIOCM_DTR)
		mcr &= ~MCR_RTS;

	/* set the new MCR value in the device */
	tiny->mcr = mcr;
	return 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,31) // jiunnder2000@yahoo.com.tw
static int tiny_read_proc(char *page, char **start, off_t off, int count,
                          int *eof, void *data)
{
	struct tiny_serial *tiny;
	off_t begin = 0;
	int length = 0;
	int i;

	length += sprintf(page, "tinyserinfo:1.0 driver:%s\n", DRIVER_VERSION);
	for (i = 0; i < TINY_TTY_MINORS && length < PAGE_SIZE; ++i) {
		tiny = tiny_table[i];
		if (tiny == NULL)
			continue;

		length += sprintf(page+length, "%d\n", i);
		if ((length + begin) > (off + count))
			goto done;
		if ((length + begin) < off) {
			begin += length;
			length = 0;
		}
	}
	*eof = 1;
done:
	if (off >= (length + begin))
		return 0;
	*start = page + (off-begin);
	return (count < begin+length-off) ? count : begin + length-off;
}
#endif

#define tiny_ioctl tiny_ioctl_tiocgserial
#if (LINUX_VERSION_CODE > VERSION_CODE(2,6,35)) 
static int tiny_ioctl(struct tty_struct *tty, unsigned int cmd, unsigned long arg)
#else
static int tiny_ioctl(struct tty_struct *tty, struct file *file,
                      unsigned int cmd, unsigned long arg)
#endif
{
	struct tiny_serial *tiny = tty->driver_data;

	if (cmd == TIOCGSERIAL) {
		struct serial_struct tmp;

		if (!arg)
			return -EFAULT;

		memset(&tmp, 0, sizeof(tmp));

		tmp.type		= tiny->serial.type;
		tmp.line		= tiny->serial.line;
		tmp.port		= tiny->serial.port;
		tmp.irq			= tiny->serial.irq;
		tmp.flags		= ASYNC_SKIP_TEST | ASYNC_AUTO_IRQ;
		tmp.xmit_fifo_size	= tiny->serial.xmit_fifo_size;
		tmp.baud_base		= tiny->serial.baud_base;
		tmp.close_delay		= 5*HZ;
		tmp.closing_wait	= 30*HZ;
		tmp.custom_divisor	= tiny->serial.custom_divisor;
		tmp.hub6		= tiny->serial.hub6;
		tmp.io_type		= tiny->serial.io_type;

		if (copy_to_user((void __user *)arg, &tmp, sizeof(struct serial_struct)))
			return -EFAULT;
		return 0;
	}
	return -ENOIOCTLCMD;
}
#undef tiny_ioctl

#define tiny_ioctl tiny_ioctl_tiocmiwait
#if (LINUX_VERSION_CODE > VERSION_CODE(2,6,35)) 
static int tiny_ioctl(struct tty_struct *tty, unsigned int cmd, unsigned long arg)
#else
static int tiny_ioctl(struct tty_struct *tty, struct file *file,
                      unsigned int cmd, unsigned long arg)
#endif
{
	struct tiny_serial *tiny = tty->driver_data;

	if (cmd == TIOCMIWAIT) {
		DECLARE_WAITQUEUE(wait, current);
		struct async_icount cnow;
		struct async_icount cprev;

		cprev = tiny->icount;
		while (1) {
			add_wait_queue(&tiny->wait, &wait);
			set_current_state(TASK_INTERRUPTIBLE);
			schedule();
			remove_wait_queue(&tiny->wait, &wait);

			/* see if a signal woke us up */
			if (signal_pending(current))
				return -ERESTARTSYS;

			cnow = tiny->icount;
			if (cnow.rng == cprev.rng && cnow.dsr == cprev.dsr &&
			    cnow.dcd == cprev.dcd && cnow.cts == cprev.cts)
				return -EIO; /* no change => error */
			if (((arg & TIOCM_RNG) && (cnow.rng != cprev.rng)) ||
			    ((arg & TIOCM_DSR) && (cnow.dsr != cprev.dsr)) ||
			    ((arg & TIOCM_CD)  && (cnow.dcd != cprev.dcd)) ||
			    ((arg & TIOCM_CTS) && (cnow.cts != cprev.cts)) ) {
				return 0;
			}
			cprev = cnow;
		}

	}
	return -ENOIOCTLCMD;
}
#undef tiny_ioctl

#define tiny_ioctl tiny_ioctl_tiocgicount
#if (LINUX_VERSION_CODE > VERSION_CODE(2,6,35)) 
static int tiny_ioctl(struct tty_struct *tty, unsigned int cmd, unsigned long arg)
#else
static int tiny_ioctl(struct tty_struct *tty, struct file *file,
                      unsigned int cmd, unsigned long arg)
#endif
{
	struct tiny_serial *tiny = tty->driver_data;

	if (cmd == TIOCGICOUNT) {
		struct async_icount cnow = tiny->icount;
		struct serial_icounter_struct icount;

		icount.cts	= cnow.cts;
		icount.dsr	= cnow.dsr;
		icount.rng	= cnow.rng;
		icount.dcd	= cnow.dcd;
		icount.rx	= cnow.rx;
		icount.tx	= cnow.tx;
		icount.frame	= cnow.frame;
		icount.overrun	= cnow.overrun;
		icount.parity	= cnow.parity;
		icount.brk	= cnow.brk;
		icount.buf_overrun = cnow.buf_overrun;

		if (copy_to_user((void __user *)arg, &icount, sizeof(icount)))
			return -EFAULT;
		return 0;
	}
	return -ENOIOCTLCMD;
}
#undef tiny_ioctl

/* the real tiny_ioctl function.  The above is done to get the small functions in the book */
#if (LINUX_VERSION_CODE > VERSION_CODE(2,6,35)) 
static int tiny_ioctl(struct tty_struct *tty, unsigned int cmd, unsigned long arg)
#else
static int tiny_ioctl(struct tty_struct *tty, struct file *file,
                      unsigned int cmd, unsigned long arg)
#endif
{
	switch (cmd) {
	case TIOCGSERIAL:
#if (LINUX_VERSION_CODE > VERSION_CODE(2,6,35)) 
		return tiny_ioctl_tiocgserial(tty, cmd, arg);
#else
		return tiny_ioctl_tiocgserial(tty, file, cmd, arg);
#endif
	case TIOCMIWAIT:
#if (LINUX_VERSION_CODE > VERSION_CODE(2,6,35)) 
		return tiny_ioctl_tiocmiwait(tty, cmd, arg);
#else
		return tiny_ioctl_tiocmiwait(tty, file, cmd, arg);
#endif
	case TIOCGICOUNT:
#if (LINUX_VERSION_CODE > VERSION_CODE(2,6,35)) 
		return tiny_ioctl_tiocgicount(tty, cmd, arg);
#else
		return tiny_ioctl_tiocgicount(tty, file, cmd, arg);
#endif
	}

	return -ENOIOCTLCMD;
}

static struct tty_operations serial_ops = {
	.open = tiny_open,
	.close = tiny_close,
	.write = tiny_write,
	.write_room = tiny_write_room,
	.set_termios = tiny_set_termios,
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,18) // jiunnder2000@yahoo.com.tw
	.tiocmget = tiny_tiocmget,
	.tiocmset = tiny_tiocmset,
	.ioctl = tiny_ioctl,
#elif LINUX_VERSION_CODE < KERNEL_VERSION(2,6,31) // jiunnder2000@yahoo.com.tw
	.read_proc = tiny_read_proc,
#endif
};

static int __init tiny_init(void)
{
	int retval;
	int i;
#if 1 // jiunnder2000@yahoo.com.tw
	struct tiny_serial *tiny;
#endif

	/* allocate the tty driver */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
	tiny_tty_driver = tty_alloc_driver(TINY_TTY_MINORS, 0);
#else
	tiny_tty_driver = alloc_tty_driver(TINY_TTY_MINORS);
#endif
	if (!tiny_tty_driver)
		return -ENOMEM;

	/* initialize the tty driver */
	tiny_tty_driver->owner = THIS_MODULE;
	tiny_tty_driver->driver_name = "tiny_tty";
	tiny_tty_driver->name = "ttta";
        /* no more devfs subsystem */
	tiny_tty_driver->major = TINY_TTY_MAJOR,
	tiny_tty_driver->type = TTY_DRIVER_TYPE_SERIAL,
	tiny_tty_driver->subtype = SERIAL_TYPE_NORMAL,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18) // jiunnder2000@yahoo.com.tw
        tiny_tty_driver->flags = TTY_DRIVER_REAL_RAW | TTY_DRIVER_DYNAMIC_DEV,
#else
	tiny_tty_driver->devfs_name = "tts/ttty%d";
	tiny_tty_driver->major = TINY_TTY_MAJOR,
	tiny_tty_driver->type = TTY_DRIVER_TYPE_SERIAL,
	tiny_tty_driver->subtype = SERIAL_TYPE_NORMAL,
	tiny_tty_driver->flags = TTY_DRIVER_REAL_RAW | TTY_DRIVER_NO_DEVFS,
	tiny_tty_driver->init_termios = tty_std_termios;
        tiny_tty_driver->flags = TTY_DRIVER_REAL_RAW,
#endif
        /* no more devfs subsystem */
	tiny_tty_driver->init_termios = tty_std_termios;
	tiny_tty_driver->init_termios.c_cflag = B9600 | CS8 | CREAD | HUPCL | CLOCAL;
	tty_set_operations(tiny_tty_driver, &serial_ops);

	/* hack to make the book purty, yet still use these functions in the
	 * real driver.  They really should be set up in the serial_ops
	 * structure above... */
	// jiunnder2000@yahoo.com.tw,  move to serial_ops
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,18)
	tiny_tty_driver->tiocmget = tiny_tiocmget;
	tiny_tty_driver->tiocmset = tiny_tiocmset;
	tiny_tty_driver->ioctl = tiny_ioctl;
#elif LINUX_VERSION_CODE < KERNEL_VERSION(2,6,31) // jiunnder2000@yahoo.com.tw
	tiny_tty_driver->read_proc = tiny_read_proc;
#endif

#if 1  // jiunnder2000@yahoo.com.tw, initialize the tiny_table[]
	for ( i=0 ; i < TINY_TTY_MINORS; i++) {
		tiny = kmalloc(sizeof(*tiny), GFP_KERNEL);
		if (!tiny)
			return -ENOMEM;

		tiny->open_count = 0;
		tiny->timer = NULL;
		sema_init(&tiny->sem, 1);

		tiny_table[i] = tiny;
#if (LINUX_VERSION_CODE >= VERSION_CODE(3,7,0))
		tty_port_init(&tiny->ttyPort);
		tty_port_link_device(&tiny->ttyPort, tiny_tty_driver, i);
#endif
	}
#endif


	/* register the tty driver */
	retval = tty_register_driver(tiny_tty_driver);
	if (retval) {
		printk(KERN_ERR "failed to register tiny tty driver");
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
		tty_driver_kref_put(tiny_tty_driver);
#else
		put_tty_driver(tiny_tty_driver);
#endif
		return retval;
	}

	for (i = 0; i < TINY_TTY_MINORS; ++i)
		tty_register_device(tiny_tty_driver, i, NULL);

	printk(KERN_INFO DRIVER_DESC " " DRIVER_VERSION);
#ifdef __UART__ // jiunnder2000@yahoo.com.tw, To do: Initialize UART
	
	// To do: Initial the UART n,8,1, 2400 baud

	// To do: request interrupt for comport1
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24)
	retval = request_irq(COM_IRQ, uart_interrupt, SA_INTERRUPT,"ttty", NULL);
#elif  LINUX_VERSION_CODE < KERNEL_VERSION(4,1,0)
	retval = request_irq(COM_IRQ, uart_interrupt, IRQF_DISABLED,"ttty", NULL);
#else
	// request_irq()
#endif
    
	printk(KERN_ALERT "interrupt on\n");
	outb(COM1_IRQ_ON,(PORT_COM1 + REG_MCR));
	outb(REC_IRQ_ON,(PORT_COM1 + REG_IER));
#endif

	return retval;
}

static void __exit tiny_exit(void)
{
	struct tiny_serial *tiny;
	int i;

#ifdef __UART__ // jiunnder2000@yahoo.com.tw, To do: free IRQ

#endif
	for (i = 0; i < TINY_TTY_MINORS; ++i)
		tty_unregister_device(tiny_tty_driver, i);

	tty_unregister_driver(tiny_tty_driver);

	/* shut down all of the timers and free the memory */
	for (i = 0; i < TINY_TTY_MINORS; ++i) {
		tiny = tiny_table[i];
		if (tiny) {
			/* close the port */
			while (tiny->open_count)
				do_close(tiny);

#ifndef __UART__ // jiunnder2000@yahoo.com.tw
			/* shut down our timer and free the memory */
			del_timer(tiny->timer);
			kfree(tiny->timer);
#endif
			kfree(tiny);
			tiny_table[i] = NULL;
		}
	}
}

module_init(tiny_init);
module_exit(tiny_exit);
