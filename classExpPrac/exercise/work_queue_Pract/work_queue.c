#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>

// To do: include workqueue.h
#include <linux/workqueue.h>
#include <linux/interrupt.h>

struct my_data{
	int len;
	char *buf;
	unsigned long jiffies;
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,20)
	struct work_struct mywq_task;
#endif
} mywq_data;

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,20)
void mywq_fun (struct work_struct *work)
#else
void mywq_fun (void *ptr)
#endif
{
  // To do: implement your defered codes
    struct my_data *data = container_of(work, struct my_data, mywq_task);
    
    if(in_interrupt())
    {
        printk("in interrupt context\n");
    }
    else
    {
        printk("in process context\n");
    }
    printk("In mywq_fun, jiffies = %ld, len = %d\n", jiffies, data->buf);
}

char buf[] = "Hello from mywq_fun";

int init_module(void)
{
	int len = 100;
	
	mywq_data.len = len ;
	mywq_data.buf = buf ;
	mywq_data.jiffies = jiffies ;
	
	printk ("queued work at jiffies = %lu\n", jiffies );

	// To do: Initialize a work queue
    INIT_WORK(&mywq_data.mywq_task, mywq_fun);
	// To do: schedule a work
    schedule_work(&mywq_data.mywq_task);

  return 0;
}

void cleanup_module(void)
{
	printk ("I cleaned up, jiffies = %lu\n", jiffies );
}

MODULE_LICENSE("GPL");
