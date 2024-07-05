/*
 *  uart driver exercise code copyright is owned by the authors
 *  author: jiunnder2000@yahoo.com.tw
 */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h> /* printk() */
#include <linux/errno.h>  /* error codes */
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <linux/poll.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
#include <linux/cdev.h>
static struct cdev mycdev;
static dev_t devno=0;
#endif

#include "ringbuf.h"

int uart_major = 0;
int uart_minor = 0;
typedef short word;

#define PORT_COM1       0x3f8
#define PORT_COM2       0x2f8
#define PORT_COM3       0x3e8
#define PORT_COM4       0x2e8
#define COM_IRQ		4

//SOME CONSTANTS FOR PROGRAMMING THE UART

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
#define IIR_RECEIVED_AVAILABLE	0x04

static unsigned char *prbuf, *pwbuf;
static struct ring_buffer ring;      // Declared the ring buffer pointer

// To do: Declare a spinlock 


static int uart_open (struct inode *inode, struct file *filp)
{
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
        try_module_get(THIS_MODULE);
#else
        MOD_INC_USE_COUNT;
#endif
    printk(KERN_ALERT "uart_open():\n");
 
    return 0;
}

static int uart_close(struct inode *inode, struct file *file)
{
    printk(KERN_ALERT "uart_close():\n");
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
    module_put(THIS_MODULE);
#elif LINUX_VERSION_CODE > KERNEL_VERSION(2,4,0)
    MOD_DEC_USE_COUNT;
#endif
    return 0;
}

ssize_t uart_read (struct file * filp, char *buf, size_t count, loff_t *t)
{
    int rcount=0;

    // To do: lock

    while ( (rcount < count) && !ring_buffer_isenmpty(&ring) ) {
        *(prbuf+rcount)=ring_buffer_get (&ring);
        rcount++;
    }
    // To do: unlock 


    if( rcount != 0 ) {
      // Copy the data to user space
      if( copy_to_user(buf, prbuf, rcount) != 0) {
        return -EFAULT;
      }
    }

    return rcount;
}

ssize_t uart_write (struct file * filp, const char *buf, size_t count, loff_t *t)
{
    int i=0;

    // Copy the data from user space
    
    if( copy_from_user(pwbuf, buf, count) != 0 ) {
      return -EFAULT;
    }
    
    printk("uart_write(): count:%ld\n", count);

    // Wait until transmit holding register empty
    while( ! (inb(PORT_COM1+REG_LSR) & THRE_EMPTY ) );

    while( i < count ) {
      if( inb(PORT_COM1+REG_LSR) & THRE_EMPTY ) {
        // write to IO port, PORT_COM1
        outb(*(pwbuf+i), PORT_COM1);
        i++;
      }
    }

    return i;
}

struct file_operations uart_fops = {
    .read=uart_read,
    .write=uart_write,
    .open=uart_open,
    .release = uart_close
};

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24)
static irqreturn_t uart_interrupt(int irq, void *dev_id, struct pt_regs *regs)
#else
static irqreturn_t uart_interrupt(int irq, void *dev_id)
#endif
{ 
  unsigned char reg, ch;

  // To do: lock 


  // Recive data from interrupt
  reg = inb(PORT_COM1+REG_IIR);

  if ( reg & IIR_RECEIVED_AVAILABLE ) {
    printk("uart_interrupt(): iir:%x rx interrupt\n", reg);

    while ( inb(PORT_COM1+REG_LSR) & DATA_READY ) {
      // Read recive character form register and write to ring buffer
      ch=inb(PORT_COM1);
      ring_buffer_put (&ring, ch);
    }
  }
  else {
    printk("uart_interrupt(): iir:%x unknown\n", reg);
  }

  // To do: unlock


  return IRQ_HANDLED;
}

int __init init_module(void)
{
    int result;
    word divisor;
    unsigned char reg;
    
    struct resource *base_res;
    // Request the IO port region
 
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
    base_res = request_region(PORT_COM1,8,"uart");

    printk(KERN_ALERT "uart: INIT_MOD\n");
    if (!base_res) 
    {
        printk(KERN_ALERT "uart: can't get I/O address 0x%x\n",PORT_COM1);
        return 1;
    }
#else
    if ( check_region(PORT_COM1, 8) ) {
        printk(KERN_INFO "uart: Can't get I/O address 0x%x\n", PORT_COM1);
        return -1;
    }

    request_region(PORT_COM1, 8, "uart");
#endif

    // Register character driver
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
    if(uart_major) {
        if ( register_chrdev_region(MKDEV(uart_major,0), 1, "uart") < 0 ) {
            printk ("register_chrdev_region() fail\n");
            release_region(PORT_COM1,8);
            return -1;
        }
    }
    else {
        if (alloc_chrdev_region(&devno, 0, 1, "uart") < 0) {
            printk ("alloc_chrdev_region() fail\n");
            release_region(PORT_COM1,8);
            return -1;
        }
        uart_major=MAJOR(devno);
    }
    cdev_init(&mycdev, &uart_fops);
    mycdev.owner=THIS_MODULE;
    if(cdev_add(&mycdev, MKDEV(uart_major,0), 1)) {
        printk ("Error adding cdev\n");
        unregister_chrdev_region(MKDEV(uart_major, 0), 1);
        release_region(PORT_COM1,8);
    }
#else
    uart_major = register_chrdev(0, "uart", &uart_fops);
    if (uart_major < 0) {
	printk(KERN_ALERT "uart: can't get major number\n");
        release_region(PORT_COM1,8);
        return -1;
    }
#endif

    // Allocate read/write buffer
    prbuf = (unsigned char *)__get_free_page(GFP_KERNEL);
    pwbuf = (unsigned char *)__get_free_page(GFP_KERNEL);
    if ( !prbuf || !pwbuf ) {
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
        cdev_del(&mycdev);
        unregister_chrdev_region(MKDEV(uart_major, 0), 1);
#else
        unregister_chrdev(uart_major, "uart");
#endif
        release_region(PORT_COM1,8);
        return -ENOMEM;
    }

    // PORT_COM1, 9600, BITS_8 | PARITY_NONE | STOP_ONE
    outb(DLR_ON,(PORT_COM1 + REG_LCR));
    reg = inb(PORT_COM1+REG_LCR);

    printk(KERN_ALERT "REG_LCR %x\n", reg); 
    divisor = 48;
    outw(divisor, PORT_COM1);
    outb((BITS_8 | PARITY_NONE | STOP_ONE), (PORT_COM1 + REG_LCR));
    
    // Request interrupt for comport1
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24)
    result = request_irq(COM_IRQ, uart_interrupt,SA_INTERRUPT,"uart",NULL);
#elif LINUX_VERSION_CODE < KERNEL_VERSION(4,1,0)
    result = request_irq(COM_IRQ, uart_interrupt,IRQF_DISABLED,"uart",NULL);
#else
    result = request_irq(COM_IRQ, uart_interrupt,0,"uart",NULL);
#endif
    
    if(result) {
    	printk(KERN_ALERT "can't register irq\n");
        if( prbuf) free_page((unsigned long)prbuf);
        if( pwbuf) free_page((unsigned long)pwbuf);
        release_region(PORT_COM1,8);
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
        cdev_del(&mycdev);
        unregister_chrdev_region(MKDEV(uart_major, 0), 1);
#else
        unregister_chrdev(uart_major, "uart");
#endif
	return -EFAULT;
    }
    else {
    	printk(KERN_ALERT "interrupt on\n");
    	outb(COM1_IRQ_ON,(PORT_COM1 + REG_MCR));
    	outb(REC_IRQ_ON,(PORT_COM1 + REG_IER));
    }

    ring_buffer_init (&ring);

    return 0;
}

void __exit cleanup_module(void)
{
    // free irq
    free_irq(COM_IRQ, NULL);
    if( prbuf) free_page((unsigned long)prbuf);
    if( pwbuf) free_page((unsigned long)pwbuf);
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
    cdev_del(&mycdev);
    unregister_chrdev_region(MKDEV(uart_major, 0), 1);
#else
    unregister_chrdev(uart_major, "uart");
#endif
    release_region(PORT_COM1,8);
}

MODULE_LICENSE("GPL");
