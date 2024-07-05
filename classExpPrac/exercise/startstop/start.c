#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

static int myint = 20;
static char *mystring = "blah";

module_param(myint, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
module_param(mystring, charp, 0);

static int __init hello_init(void)
{
    printk(KERN_ALERT"myint is an intager:%i\n", myint);
    printk(KERN_ALERT"mystring is a string:%s\n", mystring);
    return 0;
}

module_init(hello_init);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("PickEgg842335");
MODULE_DESCRIPTION("This is a test");
