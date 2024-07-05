#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>   // for put_user, strlen_user
#include "chardev.h"

#define DEVICE_NAME "hello1"  //Dev name as it appears in "/proc/devices"
#define BUF_LEN 80

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
#include <linux/cdev.h>
static struct cdev mycdev;
#endif

static char Message[BUF_LEN]="The string is from kernel module!\n";
static char *Message_Ptr=Message;
static int major=0;
static int device_open(struct inode *inode, struct file *file)
{
	Message_Ptr=Message;
	return 0;
}

static int device_release(struct inode *inode, struct file *file)
{
	return 0;
}

static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t *offset)
{
	int i=0;
	if( *Message_Ptr == 0 )
		return 0;

	while( length && *Message_Ptr )
	{
		put_user( *(Message_Ptr++), buffer++);
		length--;
		i++;
	}
  
  return i;
}

static ssize_t device_write(struct file *filp,const char *buffer, size_t length, loff_t *off)
{
	int i;

	for( i=0; i<length && i<BUF_LEN ; i++) {
		// To do: copy the data from kernel space to user space
        get_user(Message[i], buffer+i);
	}
	Message_Ptr = Message;

	return i;
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
long device_ioctl(struct file *file,unsigned int ioctl_num, unsigned long ioctl_param)
#else
int device_ioctl(struct inode *inode, struct file *file,unsigned int ioctl_num, unsigned long ioctl_param)
#endif
{
	int len, i;

	switch(ioctl_num)
	{
		case IOCTL_SET_MSG:
		// To do: Store the message
            len = strnlen_user((char*)ioctl_param, BUF_LEN);
            device_write(file, (char*)ioctl_param, len, 0);
		    break;
		case IOCTL_GET_MSG:
		// To do: return the message
            i = device_read(file, (char*)ioctl_param, 99, 0);
            put_user('\0', (char*)ioctl_param + i);
		    break;
		case IOCTL_GET_NTH_BYTE:
		// To do: return the nth byte
            return(Message[ioctl_param]);
		break;
	}
	return 0;
}

static struct file_operations fops={
  .read = device_read,
  .write = device_write,
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
  .unlocked_ioctl=device_ioctl,
#else
  .ioctl = device_ioctl,
#endif
  .open = device_open,
  .release = device_release
};

int init_module(void)
{
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
	dev_t devno=0;

	if(major) {
		if ( register_chrdev_region(MKDEV(major,0), 1, DEVICE_NAME) < 0 ) {
			printk ("register_chrdev_region() fail\n");
			return -1;
		}
	}
	else {
		if (alloc_chrdev_region(&devno, 0, 1, DEVICE_NAME) < 0) {
			printk ("alloc_chrdev_region() fail\n");
			return -1;
		}
		major=MAJOR(devno);
	}
	cdev_init(&mycdev, &fops);
	mycdev.owner=THIS_MODULE;
	if(cdev_add(&mycdev, MKDEV(major,0), 1)) {
		printk ("Error adding cdev\n");
	}
#else
	major = register_chrdev(0, DEVICE_NAME, &fops);
	if (major < 0) {
		printk ("Registering the character device failed with %d\n", major);
		 return -1;
	}
#endif

  printk("I was assigned major number %d. To talk to \n", major);
  printk("To create a device file with mknod %s c %d 0\n", DEVICE_FILE_NAME, major);

  return 0;
}

void cleanup_module(void) 
{
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
	cdev_del(&mycdev);
	unregister_chrdev_region(MKDEV(major, 0), 1);
#else
	/* Unregister the device */
	unregister_chrdev(major, DEVICE_NAME);
#endif
}
MODULE_LICENSE("GPL");
