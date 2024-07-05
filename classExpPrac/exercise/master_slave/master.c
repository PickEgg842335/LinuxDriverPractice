#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kmod.h>
#include "slave.h"

static int __init master_init(void)
{
    slave_test();
    return 0;
}

static void __exit master_exit(void)
{
    printk(KERN_ALERT"Master exit\n");
}

module_init(master_init);
module_exit(master_exit);
MODULE_LICENSE("GPL");

