#include <linux/module.h>
MODULE_AUTHOR("Yusuke Tada and Ryuichi Ueda");
MODULE_AUTHOR("Driver for LED control, source code originally from lecture cit_robosys2020 organized by Prof.Ryuichi Ueda");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.0.1");

static int __init init_mod(void)
{
    printk(KERN_INFO "%s is loade. \n", __FILE__);
	return 0;
}

static void __exit cleanup_mod(void)
{
	printk(KERN_INFO "%s is unloaded.\n", __FILE__);
}

module_init(init_mod);
module_exit(cleanup_mod);

