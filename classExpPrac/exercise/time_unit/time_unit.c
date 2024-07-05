#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/init.h>
#include<linux/sched.h>
#include<linux/delay.h>

int __init time_init(void) {

	unsigned long jiffies_a, jiffies_b;

	jiffies_a=jiffies;
	jiffies_b=jiffies + HZ/10;

	printk("jiffies:%lu\n", jiffies_a);
	printk("HZ:%d, jiffies_b:%lu\n", HZ, jiffies_b);
	mdelay(100);
	if( time_before(jiffies, jiffies_b) )
		printk("jiffies: %lu < jiffies_b:%lu\n", jiffies, jiffies_b);
	else
		printk("jiffies: %lu >= jiffies_b:%lu\n", jiffies, jiffies_b);


	return 0;
}

void __exit time_exit(void){

}

module_init(time_init);
module_exit(time_exit);
MODULE_LICENSE("GPL");
