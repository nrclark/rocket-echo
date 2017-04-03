/*
 *  hello-1.c - The simplest kernel module.
 */
#include <linux/module.h>   /* Needed by all modules */
#include <linux/kernel.h>   /* Needed for KERN_INFO */
#include "dummy.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nick Clark");
MODULE_DESCRIPTION("The GREATEST MODULE in the world");

static int __init my_init(void) {
    printk(KERN_INFO "Hello, world %d\n", 2);
    return 0;
}

static void __exit my_exit(void) {
    printk(KERN_INFO "Goodbye, world 2\n");
    return;
}

module_init(my_init);
module_exit(my_exit);
