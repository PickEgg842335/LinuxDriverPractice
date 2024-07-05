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

	// To do: schedule a work

  return 0;
}

void cleanup_module(void)
{
	printk ("I cleaned up, jiffies = %lu\n", jiffies );
}

MODULE_LICENSE("GPL");
