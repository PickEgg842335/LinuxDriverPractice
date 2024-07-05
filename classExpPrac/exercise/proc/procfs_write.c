#include <linux/version.h>
#define	VERSION_CODE(ver,rel,seq)	((ver << 16) | (rel << 8) | seq)

#if (LINUX_VERSION_CODE >= VERSION_CODE(3,10,0))
#include <linux/seq_file.h>
#endif
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/uaccess.h> 
#include <linux/proc_fs.h>
#define PROCFS_NAME "test"

struct proc_dir_entry *Our_Proc_File;
struct proc_dir_entry *our_dir;
static int write_value=0; // flag, 1:start the garbage collection, 0:stop the gargabe collection

#if LINUX_VERSION_CODE >= VERSION_CODE(5,10,0)

ssize_t procfile_read(struct file *filp, char __user *buf, size_t nbytes , loff_t *ppos) {

	char tmpbuf[4];

	if ( *ppos >= sizeof(tmpbuf) ) {
		return 0;
	}

	memset(tmpbuf, 0, 4);
	sprintf(tmpbuf, "%d", write_value);

	if ( copy_to_user(buf, (void *)&tmpbuf, strnlen(tmpbuf, 4)) )
		return -EFAULT;

	*ppos += nbytes;

	return nbytes;
}

#elif LINUX_VERSION_CODE >= VERSION_CODE(3,10,0)
int procfile_read(struct seq_file *m, void *data) {

	seq_printf(m, "write_value:%d\n", write_value);

	return 0;
}
#else
ssize_t procfile_read(char *buffer,char **buffer_location,off_t offset, int buffer_length, int *eof, void *data)
{
	int len=0;

	if (offset == 0) /* Fill the buffer and get its length */
		len += sprintf(buffer,"write_value:%d\n", write_value);

	return len;
}
#endif

#if LINUX_VERSION_CODE >= VERSION_CODE(3,10,0)
static ssize_t procfile_write(struct file *file, const char __user *buffer, size_t count, loff_t *data)
#else
static int procfile_write(struct file *file, const char *buffer, unsigned long count, void *data)
#endif
{
	char tmpbuf[5];
	int len;
	
	len = ( count > 4 ) ? 4 : count;
	
	if(copy_from_user( tmpbuf, buffer, len)) {
		return -EFAULT;
	}
	tmpbuf[len]='\0';

	sscanf(tmpbuf, "%d", &write_value);
	
	if ( write_value ) // wake up the sleeping garbage collection thread
		printk("You Input nonzero value:%d\n", write_value);
	else
		printk("You Input zero value:%d\n", write_value);
	
	return len;
}

#if LINUX_VERSION_CODE >= VERSION_CODE(5,10,0)
static struct proc_ops proc_ops= {
	.proc_read = procfile_read,
	.proc_write = procfile_write,
};
#elif LINUX_VERSION_CODE >= VERSION_CODE(3,10,0)
static int procfile_open (struct inode *inode, struct file *file) 
{
	return single_open(file, procfile_read, NULL);
}

static struct file_operations proc_fops = {
	.owner	=	THIS_MODULE,
	.open	=	procfile_open,
	.read	=	seq_read,
	.write	=	procfile_write,
};
#endif

int __init procfile_init(void)
{
	printk(KERN_ALERT "Trying to create /proc/oudir/%s:\n", PROCFS_NAME);

	our_dir=proc_mkdir("ourdir",NULL);
#if LINUX_VERSION_CODE >= VERSION_CODE(5,10,0)
	Our_Proc_File = proc_create(PROCFS_NAME,0644, our_dir, &proc_ops);
#elif LINUX_VERSION_CODE >= VERSION_CODE(3,10,0)
	Our_Proc_File = proc_create(PROCFS_NAME,0644, our_dir, &proc_fops);
#else
	#if 1
	Our_Proc_File = create_proc_entry(PROCFS_NAME, 0644, our_dir);
	Our_Proc_File->read_proc = procfile_read;
	#else
	Our_Proc_File = create_proc_read_entry(PROCFS_NAME, 0644, NULL, procfile_read, NULL);
	#endif
	Our_Proc_File->write_proc = procfile_write;
	if ( Our_Proc_File == NULL ) {
		printk(KERN_ALERT "Error: Could not initialize /proc/ourdir/%s\n", PROCFS_NAME);
		remove_proc_entry(PROCFS_NAME, NULL);
		return -ENOMEM;
	} else {
		printk(KERN_ALERT "Success!\n");
	}
#endif
	return 0;
}
void __exit procfile_exit(void)
{
    remove_proc_entry(PROCFS_NAME, our_dir);
    remove_proc_entry("ourdir",NULL);
    printk(KERN_ALERT "/proc/ourdir/%s removed\n", PROCFS_NAME);
}

module_init(procfile_init);
module_exit(procfile_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("jiunnder2000@yahoo.com.tw");
MODULE_DESCRIPTION("Create /proc file for read/write");
