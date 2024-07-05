#include <linux/version.h>
#define	VERSION_CODE(ver,rel,seq)	((ver << 16) | (rel << 8) | seq)

#if (LINUX_VERSION_CODE >= VERSION_CODE(3,10,0))
#include <linux/seq_file.h>
#endif
#include <linux/module.h> /* Specifically, a module */
#include <linux/kernel.h> /* We're doing kernel work */
#include <linux/proc_fs.h> /* Necessary because we use the proc fs */
#define PROCFS_NAME "test"
struct proc_dir_entry *Our_Proc_File;
struct proc_dir_entry *our_dir;
static int count = 1;

#if LINUX_VERSION_CODE >= VERSION_CODE(5,10,0)

int procfile_show(struct seq_file *m, void *v)
{
	seq_printf(m, "Read from the /proc/%s, count:%d", PROCFS_NAME, count++);
	seq_puts(m, "Go away!\n");

	return 0; /* Return the length */
}

#elif LINUX_VERSION_CODE >= VERSION_CODE(3,10,0)

int procfile_read(struct seq_file *m, void *data)
{
	seq_printf(m, "For the %d time, go away!\n", count++);

	return 0;
}

static int procfile_open (struct inode *inode, struct file *file) 
{
	return single_open(file, procfile_read, NULL);
}

static struct file_operations proc_fops = {
	.owner	=	THIS_MODULE,
	.open	=	procfile_open,
	.read	=	seq_read,
};

#else
ssize_t procfile_read(char *buffer,char **buffer_location,off_t offset, int buffer_length, int *eof, void *data)
{
	int len; /* The number of bytes actually used */

	if (offset > 0)
		len =0;
	else	/* Fill the buffer and get its length */
		len = sprintf(buffer,"For the %d time, go away!\n", count++);
 
	printk(KERN_ALERT "leaving /proc/%s : procfile_read, wrote %d Bytes, offset:%lu\n", PROCFS_NAME, len, offset);

	return len; /* Return the length */
}
#endif


int init_module()
{
	printk(KERN_ALERT "Trying to create /proc/%s:\n", PROCFS_NAME);

	// To do: Create the subdir, our_dir.

#if LINUX_VERSION_CODE >= VERSION_CODE(5,10,0)

#elif LINUX_VERSION_CODE >= VERSION_CODE(3,10,0)
	Our_Proc_File = proc_create(PROCFS_NAME,0644, our_dir, &proc_fops);
#else
	#if 1
	Our_Proc_File = create_proc_entry(PROCFS_NAME, 0644, our_dir);
	Our_Proc_File->read_proc = procfile_read;
	#else
	Our_Proc_File = create_proc_read_entry(PROCFS_NAME, 0644, NULL, procfile_read, NULL);
	#endif
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

void cleanup_module()
{
	remove_proc_entry(PROCFS_NAME, our_dir);
	// To do: Remove the subdir, our_dir.
	
	printk(KERN_ALERT "/proc/ourdir/%s removed\n", PROCFS_NAME);
}

MODULE_LICENSE("GPL");
