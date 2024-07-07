#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/uaccess.h> /* for put_user */
#include <linux/interrupt.h>

void mytl_fun (unsigned long t)
{
	// To do: implement your defered codesa
    if(in_interrupt())
    {
        printk("in intterrupt context\n");
    }
    printk("I am in mytl_func, jiffies = %ld\n", jiffies);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(5,9,0)
DECLARE_TASKLET(my_tasklet, mytl_fun, 0);
#else
DECLARE_TASKLET_OLD(my_tasklet, mytl_fun);
#endif
MODULE_LICENSE("GPL");

int init_module(void)
{
	printk ("queued task at jiffies = %ld\n", jiffies );

	// To do : schedule a tasklet
    tasklet_schedule(&my_tasklet);
    printk("TL schedule at jiffies = %ld\n", jiffies);

	return 0;
}

void cleanup_module(void)
{
	printk ("I cleaned up, jiffies = %ld\n", jiffies );
}

