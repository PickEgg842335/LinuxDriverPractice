#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/fcntl.h>  // for O_NONBLOCK
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
#include <linux/err.h>
#else
#include <linux/errno.h>
#endif
#include <linux/uaccess.h>   // for put_user

#include <linux/sched.h>   // for wait queue, TASK_INTERRUPTIBLE
//#include <linux/wait.h>   // for


#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
#include <linux/cdev.h>
static struct cdev mycdev;
#endif

#include "chardev.h"
#define DEVICE_NAME "hello1"  //Dev name as it appears in "/proc/devices"
#define BUF_LEN 80

static char Message[BUF_LEN];
int major=230;
// To do: declare a wait queue, char_wq, and a flag, bEmpty.
DECLARE_WAIT_QUEUE_HEAD(char_wq);
static int bEmpty = 0;

static int device_open(struct inode *inode, struct file *file)
{
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
	try_module_get(THIS_MODULE);
#else
	MOD_INC_USE_COUNT;
#endif
  return 0;
}

static int device_release(struct inode *inode, struct file *file)
{
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
    module_put(THIS_MODULE);
#elif LINUX_VERSION_CODE > KERNEL_VERSION(2,4,0)
    MOD_DEC_USE_COUNT;
#endif
  return 0;
}

static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t *off)
{
  char *from = Message + *off;

  if ( *off >= BUF_LEN )
      return 0;
  if ( *off + length >= BUF_LEN )
      length = BUF_LEN -*off;

  if (filp->f_flags & O_NONBLOCK) {
    // To do: return when the buffer is empty
        if(strnlen(from, length) == 0)
        {
            return(-EAGAIN);
        }
  }
  else {
    // To do: block here when the buffer is empty
        if(signal_pending(current)){
            printk("pid %u got signal\n", (unsigned)current->pid);
            return(-EINTR);
        }
  }
  if( copy_to_user( buffer, from, length ) )
    return -EFAULT;

  *off += length;

  return length;
}

static ssize_t device_write(struct file *filp,const char *buffer, size_t length, loff_t *off)
{
  char *to = Message + *off;

  if ( *off >= BUF_LEN )
    return 0;
  if ( *off + length >= BUF_LEN )
    length = BUF_LEN - *off;
  
  if( copy_from_user( to, buffer, length ) )
    return -EFAULT;

  if ( !(filp->f_flags & O_NONBLOCK) ) {
    // To do: wake up the blocking process
        bEmpty = 1;
        wake_up_interruptible(&char_wq);
  }

  *off += length;

  return length;
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
long device_ioctl(struct file *file,unsigned int ioctl_num, unsigned long ioctl_param)
#else
int device_ioctl(struct inode *inode, struct file *file,unsigned int ioctl_num, unsigned long ioctl_param)
#endif
{
  int len;

  switch(ioctl_num)
  {
     case IOCTL_SET_MSG:
	 len = strnlen_user((char*)ioctl_param, BUF_LEN);
	 device_write(file,(char*)ioctl_param,len,0);
	 break;
     case IOCTL_GET_MSG:
	 len=device_read(file,(char*)ioctl_param,99,0);
	 put_user('\0',(char*)ioctl_param+len);
	 break;
     case IOCTL_GET_NTH_BYTE:
	return Message[ioctl_param];
  }
  return 0;
}

static loff_t device_llseek( struct file *filp, loff_t offset, int whence )
{
	loff_t	newpos;

	switch( whence )
		{
		case 0:	// SEEK_SET
			newpos = offset;
//			printk("%s[%d]:SEEK_SET:%d\n", __FUNCTION__, __LINE__, offset);
			break;
		case 1: // SEEK_CUR
			newpos = filp->f_pos + offset;
//			printk("%s[%d]:SEEK_CUR:%d\n", __FUNCTION__, __LINE__, offset);
			break;
		case 2:	// SEEK_END
			newpos = BUF_LEN + offset;
//			printk("%s[%d]:SEEK_END:%d\n", __FUNCTION__, __LINE__, offset);
			break;
		default: // should never occur
			return -EINVAL;
		}		

	if ( newpos < 0 ) return -EINVAL;
	filp->f_pos = newpos;
	return	newpos;
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
  .release = device_release,
  .llseek = device_llseek,
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
      // To do: clear the buffer
      memset(Message, 0, BUF_LEN);

      printk("I was assigned major number %d. To talk to \n", major);
      printk("To create a device file with mknod %s c %d 0\n",DEVICE_FILE_NAME, major);


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
