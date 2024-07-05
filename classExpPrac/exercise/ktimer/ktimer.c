#include <linux/version.h>
#define	VERSION_CODE(ver,rel,seq)	((ver << 16) | (rel << 8) | seq)

#include <linux/kernel.h>    // need for kernel alert
#include <linux/module.h>
#include <linux/init.h>         // need for __init and __exit
#include <linux/proc_fs.h>
#include <linux/timer.h>

#include <linux/interrupt.h>
#define MAX_TIMEOUT_COUNT 20

#if (LINUX_VERSION_CODE >= VERSION_CODE(3,10,0))
#include <linux/seq_file.h>
#endif

// To do: Declare a kernel timer

static struct timespec64 curtime[MAX_TIMEOUT_COUNT];
static int timeout_count;
static struct proc_dir_entry *ktimer_proc;

#if LINUX_VERSION_CODE >= VERSION_CODE(3,10,0)
int ktimer_proc_fn(struct seq_file *m, void *data)
#else
ssize_t ktimer_proc_fn (char *buffer, char **buffer_location, off_t off, int buffer_length, int *eof, void *data)
#endif
{
	int len = 0, i;
#if LINUX_VERSION_CODE >= VERSION_CODE(3,10,0)
	for( i=0; i<MAX_TIMEOUT_COUNT; i++) {
		seq_printf(m, "[%d].tv_sec:%lld, [%d].tv_nsec:%llu\n", i, curtime[i].tv_sec, i, curtime[i].tv_nsec);
	}
#else

	if( off > 0 )
		return len;

	for( i=0; i<MAX_TIMEOUT_COUNT; i++) {
		len += sprintf(buffer+len,"[%d].tv_sec:%lld, [%d].tv_nsec:%llu\n", i, curtime[i].tv_sec, i, curtime[i].tv_nsec);
	}
#endif

	return len;
}

#if LINUX_VERSION_CODE < VERSION_CODE(3,10,0)

static int proc_ktimer_open (struct inode *inode, struct file *file) 
{
	return single_open(file, ktimer_proc_fn, NULL);
}

static struct file_operations proc_ktimer_fops = {
	.owner	=	THIS_MODULE,
	.open	=	proc_ktimer_open,
	.read	=	seq_read,
	.llseek	=	seq_lseek,
};

#endif

#if LINUX_VERSION_CODE >= VERSION_CODE(4,15,0)
static void timer1_expire(struct timer_list *t)
#else
static void timer1_expire(unsigned long data)
#endif
{
  // If you want to do something periodically, add your code here
#if LINUX_VERSION_CODE >= VERSION_CODE(4,15,0)
	ktime_get_real_ts64(&curtime[timeout_count]);
#else
	do_gettimeofday(&curtime[timeout_count]);
#endif
	timeout_count++;
	if( timeout_count < MAX_TIMEOUT_COUNT ) {
		// To do: add timer again

	}
}

static int __init ktimer_init(void) 
{
	printk("HZ:%d\n",HZ);
#if LINUX_VERSION_CODE >= VERSION_CODE(5,6,0)
	ktimer_proc=proc_create_single("ktimer", 0644, NULL, ktimer_proc_fn);
#elif LINUX_VERSION_CODE >= VERSION_CODE(3,10,0)
	ktimer_proc=proc_create("ktimer",0644, NULL, &proc_ktimer_fops);
#else
	ktimer_proc=create_proc_read_entry("ktimer", 0644, NULL, ktimer_proc_fn, NULL);
#endif
	// To do: initialize the kernel timer and callback function

	// To do: set the kernel expire time

	// To do: add timer

	return 0;
}

static void __exit ktimer_exit(void) 
{
	remove_proc_entry("ktimer",NULL);

	// To do: delete the pending timer

}

module_init(ktimer_init);
module_exit(ktimer_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("jiunnder2000@yahoo.com.tw");
MODULE_DESCRIPTION("kernel timer example");
