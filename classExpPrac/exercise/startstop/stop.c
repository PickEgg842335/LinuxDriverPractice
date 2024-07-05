#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>


static void __exit hello_exit(void)
{
    printk(KERN_ALERT"Bye\n");
}

module_exit(hello_exit);
MODULE_LICENSE("GPL");
