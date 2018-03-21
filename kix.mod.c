#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x4099a494, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x9ecc97b9, __VMLINUX_SYMBOL_STR(kmalloc_caches) },
	{ 0xce31b8a1, __VMLINUX_SYMBOL_STR(pv_lock_ops) },
	{ 0x43a53735, __VMLINUX_SYMBOL_STR(__alloc_workqueue_key) },
	{ 0xed2748a2, __VMLINUX_SYMBOL_STR(pci_disable_device) },
	{ 0xeae3dfd6, __VMLINUX_SYMBOL_STR(__const_udelay) },
	{ 0x67c2bd, __VMLINUX_SYMBOL_STR(pci_set_master) },
	{ 0x253ebfe, __VMLINUX_SYMBOL_STR(pci_enable_pcie_error_reporting) },
	{ 0x8083eec7, __VMLINUX_SYMBOL_STR(dev_err) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0x8c03d20c, __VMLINUX_SYMBOL_STR(destroy_workqueue) },
	{ 0xc340c498, __VMLINUX_SYMBOL_STR(pci_select_bars) },
	{ 0xdb7305a1, __VMLINUX_SYMBOL_STR(__stack_chk_fail) },
	{ 0xbdfb6dbb, __VMLINUX_SYMBOL_STR(__fentry__) },
	{ 0x51c930f4, __VMLINUX_SYMBOL_STR(pci_unregister_driver) },
	{ 0xf02a3537, __VMLINUX_SYMBOL_STR(kmem_cache_alloc_trace) },
	{ 0xe259ae9e, __VMLINUX_SYMBOL_STR(_raw_spin_lock) },
	{ 0xe8532640, __VMLINUX_SYMBOL_STR(dma_supported) },
	{ 0x57ac638, __VMLINUX_SYMBOL_STR(__pci_register_driver) },
	{ 0x360feca6, __VMLINUX_SYMBOL_STR(pci_enable_device_mem) },
	{ 0xdb33b8c7, __VMLINUX_SYMBOL_STR(pci_release_selected_regions) },
	{ 0x7005223a, __VMLINUX_SYMBOL_STR(pci_request_selected_regions) },
	{ 0x7a2322f7, __VMLINUX_SYMBOL_STR(param_ops_uint) },
	{ 0x2036c444, __VMLINUX_SYMBOL_STR(dma_ops) },
	{ 0x6df38acc, __VMLINUX_SYMBOL_STR(pci_save_state) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

MODULE_ALIAS("pci:v00008086d000010B6sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000010C6sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000010C7sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000010C8sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d0000150Bsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000010DDsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000010ECsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000010F1sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000010E1sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000010F4sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000010DBsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001508sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000010F7sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000010FCsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001517sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000010FBsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001507sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001514sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000010F9sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d0000152Asv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001529sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d0000151Csv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000010F8sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001528sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d0000154Dsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d0000154Fsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001558sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001557sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d0000154Asv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001560sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d00001563sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000015D1sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000015AAsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000015B0sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000015ABsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000015ADsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000015ACsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000015AEsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000015C2sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000015C3sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000015C4sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000015C6sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000015C7sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000015C8sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000015CEsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000015E4sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00008086d000015E5sv*sd*bc*sc*i*");

MODULE_INFO(srcversion, "ABC42633AB3E7DB610CF520");
