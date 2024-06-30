#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

static int __init hello_init(void)
{
    printk(KERN_ALERT"MEME4916 Hello world\n");
    return 0;
}

static void __exit hello_exit(void)
{
    printk(KERN_ALERT"MEME4916 Bye\n");
}

module_init(hello_init);
module_exit(hello_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("MEME4916 Yang, Ming-Cheng");
MODULE_DESCRIPTION("This is homework No.1 for Linux driver program \ 
    development class");
