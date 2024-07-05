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

ssize_t uart_read (struct file * filp, char *buf, size_t count, loff_t * t)
{
    int rcount;
    unsigned char kbuf;

    // To do: read data from COM1 and copy to user space

    return 0;
}

struct file_operations uart_fops = {
    .read=uart_read,
    .open=uart_open,
    .release = uart_close
};

int init_module(void)
{
    word divisor;
    unsigned char reg;
    
    struct resource *base_res;

    printk(KERN_ALERT "uart: INIT_MOD\n");
    // To do: request the IO port region

   //  register character driver
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


    // To do: PORT_COM1, set the baud: 2400,

    // To do: BITS_8 | PARITY_NONE | STOP_ONE  
    
    return 0;
}

void cleanup_module(void)
{
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
    cdev_del(&mycdev);
    unregister_chrdev_region(MKDEV(uart_major, 0), 1);
#else
    /* Unregister the device */
    unregister_chrdev(uart_major, "uart");
#endif
    // To do: release the IO port region

}

MODULE_LICENSE("GPL");
