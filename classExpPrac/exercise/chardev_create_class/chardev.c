#include <linux/version.h>
#include <linux/module.h>  
#include <linux/kernel.h>  
#include <linux/fs.h>
#include <linux/init.h>  
#include <linux/major.h>  
#include <linux/device.h>  
#include <linux/poll.h>  
 
#include <linux/uaccess.h> /* for put_user */

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
#include <linux/cdev.h>
static struct cdev mycdev;
#endif

#define SUCCESS 0
#define DEVICE_NAME "chardev" /* Dev name as it appears in /proc/devices */
#define BUF_LEN 80

// Global variables are declared as static, so are global within the file.
static int Major=0; // Major number assigned to our device driver
static char Message[BUF_LEN], *msg_Ptr;
// To do: declare a class for udev used
static struct class *charmodule_class; 

static int device_open(struct inode *inode, struct file *file)
{
    static int counter = 0;
    sprintf(Message,"I already told you %d times Hello world!\n", counter++);
    msg_Ptr = Message;
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
    try_module_get(THIS_MODULE);
#else
    MOD_INC_USE_COUNT;
#endif
    return SUCCESS;
}
static int device_release(struct inode *inode, struct file *file)
{
    /* Decrement the usage count, or else once you opened the file, you'll never get get rid of the module. */
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
    module_put(THIS_MODULE);
#else
    MOD_DEC_USE_COUNT;
#endif
    return 0;
}
/* Called when a process, which already opened the dev file, attempts to read from it. 
 * struct file *filp: the file operation
 * char *buffer: The buffer to fill with data
 * size_t length: The length of the buffer
 * loff_t *offset: Our offset in the file
 */
static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t *offset)
{
    int bytes_read = 0;
    if (*msg_Ptr == 0) 
        return 0;
    // Actually put the data into the buffer
    while (length && *msg_Ptr) {
	// the kernel data segment to the user data segment. 
        put_user(*(msg_Ptr++), buffer++);
        length--;
        bytes_read++;
    }

    // Most read functions return the number of bytes put into the buffer
    return bytes_read;
}
/* Called when a process writes to dev file: echo "hi" > /dev/hello */
static ssize_t device_write(struct file *filp, const char *buff, size_t len, loff_t *off)
{
    printk (KERN_ALERT "Sorry, this operation isn't supported.\n");
    return -EINVAL;
}

static struct file_operations fops = {
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release
};
// Functions

int __init init_module(void)
{
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
    dev_t devno=0;

    if(Major) {
        if ( register_chrdev_region(MKDEV(Major,0), 1, DEVICE_NAME) < 0 ) {
            printk ("register_chrdev_region() fail\n");
            return -1;
        }
    }
    else {
        if (alloc_chrdev_region(&devno, 0, 1, DEVICE_NAME) < 0) {
           printk ("alloc_chrdev_region() fail\n");
           return -1;
        }
        Major=MAJOR(devno);
    }
    cdev_init(&mycdev, &fops);
    mycdev.owner=THIS_MODULE;
    if(cdev_add(&mycdev, MKDEV(Major,0), 1)) {
        printk ("Error adding cdev\n");
    }
#else
    Major = register_chrdev(0, DEVICE_NAME, &fops);
    if (Major < 0) {
        printk ("Registering the character device failed with %d\n", Major);
        return -1;
    }
#endif

    /* To do: create your own class under /sysfs */
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,9)
    charmodule_class = class_create(THIS_MODULE, "charmodule_class"); 
#else
    charmodule_class = class_simple_create(THIS_MODULE, "charmodule_class");
#endif

    if(IS_ERR(charmodule_class)) {
      printk("Err: failed in creating class.\n");
      return -1;
    }

     /* To do: register your own device in sysfs, and this will cause udev to create corresponding device node */
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,27)
    int i=0;
    device_create(charmodule_class, NULL, MKDEV(Major, i), NULL, "hello%d", i);
    i++;
#elif LINUX_VERSION_CODE > KERNEL_VERSION(2,6,9)
    device_create(charmodule_class, NULL, MKDEV(Major, 0), "hello"); 
#else
    class_simple_device_add( charmodule_class, MKDEV(Major, 0), NULL, "hello" );
#endif
    printk("I was assigned major number %d. To talk to\n", Major);
    printk("the driver, create a dev file with\n");
    printk("'mknod /dev/hello c %d 0'.\n", Major);
    printk("Try various minor numbers. Try to cat and echo to\n");
    printk("the device file.\n");
    printk("Remove the device file and module when done.\n");
    return 0;
}

void __exit cleanup_module(void)
{
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,9)
    // To do: delete device node under /dev
    device_destroy(charmodule_class, MKDEV(Major, 0)); 
    // To do: delete class created by us
    class_destroy(charmodule_class); 
#else
    class_simple_device_remove(MKDEV(Major, 0));
    class_simple_destroy(charmodule_class);
#endif
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
    cdev_del(&mycdev);
    unregister_chrdev_region(MKDEV(Major, 0), 1);
#else
    /* Unregister the device */
    unregister_chrdev(Major, DEVICE_NAME);
#endif
    printk("unregister_chrdev\n");
}

MODULE_LICENSE ("GPL");
