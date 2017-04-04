/**
 * @file rocket-echo.c
 * @author Nick Clark
 * @date 03 April 2017
 * @brief Rocket-echo Linux kernel module
 *
 * @details This file contains an implementation of a ringbuffer-based
 * character loopback device.  */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

#include "_rkt_buf.h"
#define DEVICE_NAME "rocket"
#define CLASS_NAME "rkt"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nick Clark");
MODULE_DESCRIPTION("An echo/loopback character device");
MODULE_VERSION("0.1");

/** @brief Size of the module's internal byte buffer. Can be set as a parameter. */
static unsigned int bufsize = 256;

/** @brief Storage for the module's buffer. Initialized at init(). */
static char *storage = NULL;

/** @brief Instance of the rkt_buf ring-buffer data structure. Initialized at init(). */
static rkt_buf* my_buf = NULL;

/** @brief Major number for the device. */
static int device_major;

/** @brief Number of times the module has been opened.*/
static unsigned int open_count = 0;

/** @brief Character-class instance holder. */
static struct class* my_class = NULL;

/** @brief Device instance holder. */
static struct device* my_device = NULL;

module_param(bufsize, uint, S_IRUGO);
MODULE_PARM_DESC(bufsize, "Size of loopback buffer (in bytes, default 256)");

static int rkt_open(struct inode *inode, struct file *file);
static int rkt_release(struct inode *inode, struct file *file);
static ssize_t rkt_read(struct file *filep, char *out, size_t len, loff_t *offp);
static ssize_t rkt_write(struct file *filep, const char *in, size_t len, loff_t *offp);

/** @brief Set of implemented file operations. */
static struct file_operations fops = {
    .open = rkt_open, /**< Called when the device is opened. */
    .release = rkt_release, /**< Called when the device is closed. */
    .read = rkt_read, /**< Called when the device is read. */
    .write = rkt_write /**< Called when the device is written. */
};

/** @brief Function for initializing the kernel module. Allocates storage
 * and sets up devices. */
static int __init rkt_init(void)
{
    printk(KERN_INFO "Initializing Rocket-echo module.");
    open_count = 0;
    my_buf = kmalloc(sizeof(rkt_buf), GFP_KERNEL);
    if(my_buf == NULL) {
        printk(KERN_ALERT "Error: Rocket-echo failed to allocate rkt_buf instance.");
        return 1;
    }

    storage = kmalloc(bufsize, GFP_KERNEL);
    if(storage == NULL) {
        printk(KERN_ALERT "Error: Rocket-echo failed to allocate byte storage.");
        return 1;
    }

    rkt_buf_init(my_buf, storage, bufsize);
    device_major = register_chrdev(0, DEVICE_NAME, &fops);

    if(device_major <= 0) {
        printk(KERN_ALERT "Error: Rocket-echo failed to receive a device major number.\n");
        return device_major;
    }

    printk(KERN_INFO "Rocket-echo received device major number %d\n", device_major);
    my_class = class_create(THIS_MODULE, CLASS_NAME);

    if (IS_ERR(my_class)) {
        unregister_chrdev(device_major, DEVICE_NAME);
        return PTR_ERR(my_class);
    }

    printk(KERN_INFO "Rocket-echo device class registered successfully.\n");

    my_device = device_create(my_class, NULL, MKDEV(device_major, 0), NULL, DEVICE_NAME);
    if (IS_ERR(my_device)) {
        class_destroy(my_class);
        unregister_chrdev(device_major, DEVICE_NAME);
        printk(KERN_ALERT "Failed to create Rocket-echo device.\n");
        return PTR_ERR(my_device);
    }

    printk(KERN_INFO "Rocket-echo device created successfully.\n");
    return 0;
}

/** @brief Function for cleaning up after the kernel module. Dea-allocates
 * all storage and tears down all devices. */
static void __exit rkt_exit(void)
{
    if(storage != NULL) {
        kfree(storage);
    }

    if(my_buf != NULL) {
        kfree(my_buf);
    }

    device_destroy(my_class, MKDEV(device_major, 0));
    class_unregister(my_class);
    class_destroy(my_class);
    unregister_chrdev(device_major, DEVICE_NAME);
    printk(KERN_INFO "Rocket-echo wishes you well. Goodbye.\n");
    return;
}

/** @brief Function called when a Rocket-echo device is opened.
 * @param[in] inodep Inode pointer for the target device.
 * @param[in] filep File object for the target device.
 * @returns Exit code.
 * @retval 0 Exit uccess. */
static int rkt_open(struct inode *inodep, struct file *filep)
{
    open_count++;
    printk(KERN_INFO "Rocket-echo: device has been opened %d times\n", open_count);
    return 0;
}

/** @brief Function called when a Rocket-echo device is read.
 * @param[in] filep File pointer to the target device (ignored).
 * @param[out] out User-space pointer to output memory.
 * @param[in] len Maximum number of bytes to read.
 * @param[in] offp Pointer to the requested seek offset (ignored).
 * @returns Number of retrieved bytes.
 * @retval -EFAULT The read operation failed. */
static ssize_t rkt_read(struct file *filep, char *out, size_t len, loff_t *offp)
{
    int result;
    unsigned int current_level = rkt_buf_level(my_buf);

    if (len > current_level) {
        len = current_level;
    }

    result = rkt_buf_read(my_buf, out, len);

    if(result != 0) {
        printk(KERN_ALERT "Error: Rocket-echo buffer read failed with code: %d\n", result);
        return -EFAULT;
    }

    printk(KERN_INFO "Rocket-echo: sent %d bytes back to the user.\n", (unsigned int) len);
    return len;
}

/** @brief Function called when a Rocket-echo device is written.
 * @param[in] filep File pointer to the target device (ignored).
 * @param[in] in User-space pointer to input memory.
 * @param[in] len Number of bytes to write.
 * @param[in] offp Pointer to the requested seek offset (ignored).
 * @returns Number of written bytes.
 * @retval -EFAULT The write operation failed. */
static ssize_t rkt_write(struct file *filep, const char *in, size_t len, loff_t *offp)
{
    int result;
    unsigned int space;
    space = bufsize - rkt_buf_level(my_buf);

    if (len > space) {
        len = space;
        printk(KERN_ALERT "Warn: Rocket-echo truncated write to %d bytes", (unsigned int) len);
    }

    result = rkt_buf_write(my_buf, in, len);
    if (result != 0) {
        printk(KERN_ALERT "Error: Rocket-echo buffer-write failed with code: %d\n", result);
        return -EFAULT;
    }

    printk(KERN_INFO "Rocket-echo: received %d bytes from the user.\n", (unsigned int) len);
    return len;
}

/** @brief Function called when a Rocket-echo device is written.
 * @param[in] filep File pointer to the target device (ignored).
 * @param[in] inodep Inode pointer for the target device (ignored).
 * @returns Exit code.
 * @retval 0 Exit uccess. */
static int rkt_release(struct inode* inodep, struct file* filep)
{
    printk(KERN_INFO "Rocket-echo: device closed OK.\n");
    return 0;
}

module_init(rkt_init);
module_exit(rkt_exit);
