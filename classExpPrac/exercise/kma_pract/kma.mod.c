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
	{ 0x6a5cb5ee, "__get_free_pages" },
	{ 0xa916b694, "strnlen" },
	{ 0xcbd4898c, "fortify_panic" },
	{ 0x22f749f2, "kmalloc_caches" },
	{ 0xfcef4fc3, "kmalloc_trace" },
	{ 0x61651be, "strcat" },
	{ 0xd6ee688f, "vmalloc" },
	{ 0xe914e41e, "strcpy" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0x999e8297, "vfree" },
	{ 0x4302d0eb, "free_pages" },
	{ 0xc4ae50da, "module_layout" },
};

MODULE_INFO(depends, "");

