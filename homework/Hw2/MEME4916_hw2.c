#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h> /* for put_user */
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
#include <linux/cdev.h>
static dev_t devno=0;
static struct cdev mycdev;
#endif
/* Prototypes - this would normally go in a .h file */
int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

#define SUCCESS 0
#define DEVICE_NAME "MEME4916" /* Dev name as it appears in /proc/devices */
#define BUF_LEN 80

// Global variables are declared as static, so are global within the file.
static int Major=216; // Major number assigned to our device driver
static char *myID="MEME4916";
//static char Message[BUF_LEN]; // The Message the device will give when asked
static struct file_operations fops = {
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release
};
// Functions
int init_module(void)
{
    Major = register_chrdev(Major, DEVICE_NAME, &fops);
    printk(KERN_ALERT"YYYYy\n");
    
    if (Major < 0) 
    {
        printk("Registering the character device failed with %d\n", Major);
        return -1;
    }
    printk("Registering Success,\n major number is %d \n device name is %s"
        , Major, DEVICE_NAME);

    printk(KERN_ALERT"%s\n", myID);
    int x = strlen(myID);
    printk(KERN_ALERT"%d\n", x);
    return SUCCESS;
}

void cleanup_module(void)
{
    unregister_chrdev(Major, DEVICE_NAME);
}

static int device_open(struct inode *inode, struct file *file)
{
    return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file)
{
    return SUCCESS;
}

static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t *offset)
{
    int bytes_read = 0;
    int ReadDataLength = 0;

    for(int i = 0; i < BUF_LEN; i++)
    {
        if(*(myID + i) == NULL)
        {
            break;
        }
        ReadDataLength++;
    }

    if(length < ReadDataLength)
    {
        printk("Buffer is not enough.");
        return(-ENOBUFS);
    }

    for(int i = 0; i < ReadDataLength; i++)
    {
        put_user(*(myID + i), buffer++);
        bytes_read++;
    }
    printk("We send %d bytes to you", bytes_read);
    return bytes_read;
}

static ssize_t device_write(struct file *filp, const char *buffer, size_t length, loff_t *off)
{
    int bytes_write = 0;

    if(length <= 0)
    {
        printk("No Data in the write Data\n");
        return (-ENOBUFS);
    }

    for(int i = 0; i < BUF_LEN; i++)
    {
        get_user(*(myID + i), buffer++);
        bytes_write++;
        if((i == (BUF_LEN - 1)) && (*buffer != NULL))
        {
            printk("buffer is overfull\n");
            return(-ENOBUFS);
        }
        else if(*buffer == NULL)
        {
            break;
        }
    }
    printk("we get %d bytes from you", bytes_write);
    return(bytes_write);
}

module_param(myID, charp, 444);
MODULE_PARM_DESC(myID, "My ID in III\n ");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("MEME4916");
MODULE_DESCRIPTION("Character Driver: open, read, write, release!");
