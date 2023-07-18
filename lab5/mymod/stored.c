/*
 * Lab problem set for UNIX programming course
 * by Chun-Ying Huang <chuang@cs.nctu.edu.tw>
 * License: GPLv2
 */
#include <linux/module.h>	// included for all kernel modules
#include <linux/kernel.h>	// included for KERN_INFO
#include <linux/init.h>		// included for __init and __exit macros
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/errno.h>
#include <linux/sched.h>	// task_struct requried for current_uid()
#include <linux/cred.h>		// for current_uid();
#include <linux/slab.h>		// for kmalloc/kfree
#include <linux/uaccess.h>	// copy_to_user
#include <linux/string.h>
#include <linux/device.h>
#include <linux/cdev.h>

#define NUM_DEVICES 8

struct kshram_dev {
    struct cdev c_dev;
    void *data;
};

// int count = 8;
static dev_t devnum;
static struct cdev c_dev;
static struct class *clazz;
// static struct kshram_dev kshram_devs[NUM_DEVICES];

static int kshram_dev_open(struct inode *i, struct file *f) {
	printk(KERN_INFO "hellomod: device opened.\n");
	return 0;
}

static int kshram_dev_close(struct inode *i, struct file *f) {
	printk(KERN_INFO "hellomod: device closed.\n");
	return 0;
}

static ssize_t kshram_dev_read(struct file *f, char __user *buf, size_t len, loff_t *off) {
	printk(KERN_INFO "hellomod: read %zu bytes @ %llu.\n", len, *off);
	return len;
}

static ssize_t kshram_dev_write(struct file *f, const char __user *buf, size_t len, loff_t *off) {
	printk(KERN_INFO "hellomod: write %zu bytes @ %llu.\n", len, *off);
	return len;
}

static long kshram_dev_ioctl(struct file *fp, unsigned int cmd, unsigned long arg) {
	printk(KERN_INFO "hellomod: ioctl cmd=%u arg=%lu.\n", cmd, arg);
	return 0;
}

static const struct file_operations kshram_dev_fops = {
	.owner = THIS_MODULE,
	.open = kshram_dev_open,
	.read = kshram_dev_read,
	.write = kshram_dev_write,
	.unlocked_ioctl = kshram_dev_ioctl,
	.release = kshram_dev_close
};

static int kshram_proc_read(struct seq_file *m, void *v) {
	char buf[] = "`hello, world!` in /proc.\n";
	seq_printf(m, buf);
	return 0;
}

static int kshram_proc_open(struct inode *inode, struct file *file) {
	return single_open(file, kshram_proc_read, NULL);
}

static const struct proc_ops kshram_proc_fops = {
	.proc_open = kshram_proc_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};

static char *kshram_devnode(const struct device *dev, umode_t *mode) {
	if(mode == NULL) return NULL;
	*mode = 0666;
	return NULL;
}

static int __init kshram_init(void)
{
	// create char dev
	if(alloc_chrdev_region(&devnum, 0, 8, "kshram") < 0)
		return -1;
	if((clazz = class_create(THIS_MODULE, "kshram_class")) == NULL)
		goto release_region;
	clazz->devnode = kshram_devnode;

	for ( int i=0; i<NUM_DEVICES; i++)
	{
		char name[20];
		sprintf(name, "kshram%d", i);
		if(device_create(clazz, NULL, MKDEV(MAJOR(devnum), MINOR(devnum) + i) , NULL, name) == NULL)
		{
			goto release_class;
		}	
		cdev_init(&c_dev, &kshram_dev_fops);
    	if(cdev_add(&c_dev, MKDEV(MAJOR(devnum), MINOR(devnum) + i), 1) == -1) 
		{
        	goto release_device;
    	}
	}

	// if(device_create(clazz, NULL, devnum, NULL, "kshram_dev") == NULL)
	// 	goto release_class;
	// cdev_init(&c_dev, &kshram_dev_fops);
	// if(cdev_add(&c_dev, devnum, 1) == -1)
	// 	goto release_device;

		

	// create proc
	proc_create("kshram", 0, NULL, &kshram_proc_fops);

	printk(KERN_INFO "kshram: initialized.\n");
	return 0;    // Non-zero return means that the module couldn't be loaded.

release_device:
	
	for ( int i=0;i<NUM_DEVICES;i++)
	{
		char devname[12];
		sprintf(devname, "kshram%d", i);
		device_destroy(clazz, MKDEV(MAJOR(devnum), MINOR(devnum) + i));
	}
	// device_destroy(clazz, MKDEV(MAJOR(devnum), MINOR(devnum) + i));
	cdev_del(&c_dev);
release_class:
	class_destroy(clazz);
release_region:
	unregister_chrdev_region(devnum, 8);
	return -1;
}

static void __exit kshram_cleanup(void)
{
	remove_proc_entry("kshram", NULL);

	
	cdev_del(&c_dev);
	// cdev_del(&c_dev);
	device_destroy(clazz, devnum);
	class_destroy(clazz);
	unregister_chrdev_region(devnum, 1);

	printk(KERN_INFO "kshram: cleaned up.\n");
}

module_init(kshram_init);
module_exit(kshram_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tan-Wen Chang");
MODULE_DESCRIPTION("The unix programming course demo kernel module.");
