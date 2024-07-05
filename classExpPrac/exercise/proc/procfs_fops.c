#include <linux/version.h>
#define	VERSION_CODE(ver,rel,seq)	((ver << 16) | (rel << 8) | seq)

#include <linux/kernel.h> /* We're doing kernel work */
#include <linux/module.h> /* Specifically, a module */
#include <linux/proc_fs.h> /* Necessary because we use proc fs */
#include <linux/uaccess.h> /* for get_user and put_user */
#include <linux/sched.h>
#include <linux/fs.h>
/*
* Here we keep the last message received, to prove
* that we can process our input
*/
#define MESSAGE_LENGTH 80
static char Message[MESSAGE_LENGTH];
static struct proc_dir_entry *Our_Proc_File;
#define PROC_ENTRY_FILENAME "rw_test"

ssize_t module_output(struct file *filep, char __user *buffer, size_t length, loff_t *offset)
{
    static int finished = 0;
    int i;
    char message[MESSAGE_LENGTH + 30];

    if (finished) {
        finished = 0;
        return 0;
    }

    sprintf(message, "Last input:%s", Message);
    for (i = 0; i < length && message[i]; i++)
        put_user(message[i], buffer + i);

    finished = 1;
    return i;
}

static ssize_t module_input(struct file *filp, const char *buff, size_t len, loff_t * off)
{
  int i;

  for (i = 0; i < MESSAGE_LENGTH - 1 && i < len; i++)
    get_user(Message[i], buff + i);

  return i;
}

int module_open(struct inode *inode, struct file *file)
{
    return 0;
}

int module_close(struct inode *inode, struct file *file)
{
    return 0;
}

#if LINUX_VERSION_CODE >= VERSION_CODE(5,10,0)
// struct proc_ops defined in include/linux/proc_fs.h
struct proc_ops File_Ops_4_Our_Proc_File =
{	.proc_open = module_open,
	.proc_read = module_output,
	.proc_write = module_input,
	.proc_release = module_close
};
#else
static struct file_operations File_Ops_4_Our_Proc_File = 
{
    .read = module_output,
    .write = module_input,
    .open = module_open,
    .release = module_close,
};
#endif

// Module initialization and cleanup
int init_module()
{
    int rv = 0;

#if LINUX_VERSION_CODE >= VERSION_CODE(5,10,0)
    Our_Proc_File = proc_create(PROC_ENTRY_FILENAME, 0644, NULL, &File_Ops_4_Our_Proc_File);
#elif LINUX_VERSION_CODE >= VERSION_CODE(3,10,0)
    Our_Proc_File = proc_create(PROC_ENTRY_FILENAME, 0644, NULL, &File_Ops_4_Our_Proc_File);
#endif
    if (Our_Proc_File == NULL) {
        rv = -ENOMEM;
        printk(KERN_INFO "Error: Could not initialize /proc/test\n");
    }
    return rv;
}
void cleanup_module()
{
    remove_proc_entry(PROC_ENTRY_FILENAME, NULL);
}

MODULE_LICENSE("GPL");
