#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

void slave_test(void)
{
    printk("slave_test");
}

static int __init slave_init(void)
{
    printk(KERN_ALERT"slave loading\n");
    return 0;
}

static void __exit slave_exit(void)
{
    printk(KERN_ALERT"slave Exit\n");
}

module_init(slave_init);
module_exit(slave_exit);
EXPORT_SYMBOL(slave_test);
MODULE_LICENSE("GPL");
