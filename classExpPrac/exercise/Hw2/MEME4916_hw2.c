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
static char myIDdata[BUF_LEN]="MEME4916";
static char* myID = myIDdata;
static char Message[BUF_LEN], *msg_Ptr;
static int firstWriteFlag = true;
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
    int result = -1;
    result = register_chrdev(Major, DEVICE_NAME, &fops);
    if (result < 0) 
    {
        printk(KERN_ALERT"Registering the character device failed with %d\n", Major);
        return -1;
    }
    else if(result == 0)
    {
        printk(KERN_ALERT"Registering Success,major number is %d, device name is %s\n"
            , Major, DEVICE_NAME);
    }
    else
    {
        printk(KERN_ALERT"Registering Success, major number is %d, device name is %s\n"
            , result, DEVICE_NAME);
        Major = result;
    }
    return SUCCESS;
}

void cleanup_module(void)
{
    unregister_chrdev(Major, DEVICE_NAME);
}

static int device_open(struct inode *inode, struct file *file)
{
    printk(KERN_ALERT"Opening\n");
    memset(Message, 0, BUF_LEN);
    sprintf(Message, "%s\n", myIDdata);
    myID = myIDdata;
    msg_Ptr = Message;
    firstWriteFlag = true;
    return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file)
{
    printk(KERN_ALERT"Release\n");
    return SUCCESS;
}

static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t *offset)
{
    int bytes_read = 0;
    int DataLength = strlen(msg_Ptr);
    int StringReadLength = 0;

    printk(KERN_ALERT"Reading\n");

    if(DataLength == 0)
    {
        memset(Message, 0, BUF_LEN);
        sprintf(Message, "%s\n", myIDdata);
        msg_Ptr = Message;
        return SUCCESS;
    }

    StringReadLength = strlen(msg_Ptr);
    if(StringReadLength == 0)
    {
        return SUCCESS;
    }
    else if(length < StringReadLength)
    {
        StringReadLength = length;
    }

    for(int i = 0; i < StringReadLength; i++)
    {
        put_user(*(msg_Ptr++), buffer++);
        bytes_read++;
    }
    printk(KERN_ALERT"I send %d bytes to you, the data is %s\n", bytes_read, myIDdata);
    return bytes_read;
}

static ssize_t device_write(struct file *filp, const char *buffer, size_t length, loff_t *off)
{
    int bytes_write = 0;
    int writeLength = BUF_LEN;
    int resetwriteptrAtFinish = false;

    printk(KERN_ALERT"Writing\n");

    if(length == 0)
    {
        printk(KERN_ALERT"No Data in the write Data buffer\n");
        return (SUCCESS);
    }
    else if(BUF_LEN > length)
    {
        writeLength = length;
        resetwriteptrAtFinish = true;
    }

    if(firstWriteFlag == true)
    {
        printk(KERN_ALERT"NULL\n");
        memset(myIDdata, 0, BUF_LEN);
        firstWriteFlag = false;
    }

    for(int i = 0; i < writeLength; i++)
    {
        get_user(*(myID++), buffer++);
        bytes_write++;
        if((i == (writeLength - 1)) && (*(myID - 1) < 0x20))
        {
            *(myID - 1) = '\0';
        }
    }

    if(resetwriteptrAtFinish == true)
    {
        firstWriteFlag = true;
        memset(Message, 0, BUF_LEN);
        sprintf(Message, "%s\n", myIDdata);
        myID = myIDdata;
    }

    printk(KERN_ALERT"I get %d bytes from you, the data is %s\n", bytes_write, myIDdata);
    return bytes_write;
}

module_param(myID, charp, 444);
MODULE_PARM_DESC(myID, "My ID in III\n ");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("MEME4916");
MODULE_DESCRIPTION("Character Driver: open, read, write, release!\n");
