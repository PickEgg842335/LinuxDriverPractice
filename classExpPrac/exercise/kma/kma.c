#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/init.h>
#include<linux/slab.h>
#include<linux/vmalloc.h>
#include<linux/mm.h>
#include<linux/string.h>

unsigned long pFreeBuf;
char *pBuf1, *pBuf2;

int init_kma(void) {

	printk( "kma\n");
	pFreeBuf = __get_free_page(GFP_KERNEL);
	strcpy( (char *)pFreeBuf, "Copy to Buf\n");
	printk("strnlen():%ld, pFreeBuf:%s\n", strnlen((char*)pFreeBuf, PAGE_SIZE), (char*)pFreeBuf);

	pBuf1=kmalloc(40, GFP_KERNEL);
	memset(pBuf1, 0, 40);
	strcat(pBuf1, (char*)pFreeBuf);
	printk("strlen():%ld, pBuf1:%s\n", strnlen((char*)pBuf1, 40), (char*)pBuf1);

	pBuf2=vmalloc(40);
	sprintf(pBuf2,"%s", pBuf1);
	printk("strlen():%ld, pBuf2:%s\n", strnlen((char*)pBuf2, 40), (char*)pBuf2);

	return 0;
}

void exit_kma(void){
	printk("Bye\n");
}

module_init(init_kma);
module_exit(exit_kma);
MODULE_LICENSE("GPL");
