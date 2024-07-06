#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif


static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x92997ed8, "_printk" },
	{ 0x4629334c, "__preempt_count" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0x88db9f48, "__check_object_size" },
	{ 0x6b10bee1, "_copy_to_user" },
	{ 0x72de2b13, "module_put" },
	{ 0xf8128c62, "try_module_get" },
	{ 0x13c49cc2, "_copy_from_user" },
	{ 0xdbdf6c92, "ioport_resource" },
	{ 0x85bd1608, "__request_region" },
	{ 0x3fd78f3b, "register_chrdev_region" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0x1035c7c2, "__release_region" },
	{ 0xa463c6ea, "cdev_init" },
	{ 0x6ee90a52, "cdev_add" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0x6a5cb5ee, "__get_free_pages" },
	{ 0x1ae367ea, "cdev_del" },
	{ 0x92d5838e, "request_threaded_irq" },
	{ 0x4302d0eb, "free_pages" },
	{ 0x6bc3fbc0, "__unregister_chrdev" },
	{ 0xc1514a3b, "free_irq" },
	{ 0xc4ae50da, "module_layout" },
};

MODULE_INFO(depends, "");

