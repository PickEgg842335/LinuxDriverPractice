#include <linux/version.h>
#define	VERSION_CODE(ver,rel,seq)	((ver << 16) | (rel << 8) | seq)

#if (LINUX_VERSION_CODE >= VERSION_CODE(3,10,0))
#include <linux/seq_file.h>
#endif
#include <linux/module.h> /* Specifically, a module */
#include <linux/kernel.h> /* We're doing kernel work */
#include <linux/proc_fs.h> /* Necessary because we use the proc fs */
#define PROCFS_NAME "test"
struct proc_dir_entry *Our_Proc_File;
static int count = 1;

int procfile_show(struct seq_file *m, void *v)
{
	// To do: put the date into struct seq_file *m

	return 0; /* Return the length */
}

int init_module()
{
	// To do: Create the /proc file, /proc/test


	if ( Our_Proc_File == NULL ) {
		printk(KERN_ALERT "proc_create_single() fail\n");
		return -ENOMEM;
	}

	printk(KERN_ALERT "Create /proc/%s successed!\n", PROCFS_NAME);

	return 0;
}

void cleanup_module()
{
	// To do: remove proc entry

}

MODULE_LICENSE("GPL");
