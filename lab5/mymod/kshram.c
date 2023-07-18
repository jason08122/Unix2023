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
#include <linux/mm.h>
#include <linux/pci.h>
#include <linux/fs.h>
#include <asm/io.h>
#include "kshram.h"

#define NUM_DEVICES 8
#define KSHRAM_SIZE 4096 // 4KB

struct kshram_dev {
    char *data;
	long SIZE;
	int dev_num;
	unsigned long phys_addr;
	struct cdev cdev;
};

// int count = 8;
// int current_dev = 0;
static dev_t devnum;
static struct cdev c_dev;
static struct class *clazz;
static struct kshram_dev kshram_devices[NUM_DEVICES];

static int kshram_dev_open(struct inode *i, struct file *f) {
	struct kshram_dev* current_dev = container_of(i->i_cdev, struct kshram_dev, cdev);
    f->private_data = current_dev;

	printk(KERN_INFO "kshrammod: device opened.\n");
	return 0;
}

static int kshram_dev_close(struct inode *i, struct file *f) {
	printk(KERN_INFO "kshrammod: device closed.\n");
	return 0;
}

static ssize_t kshram_dev_read(struct file *f, char __user *buf, size_t len, loff_t *off) {
	printk(KERN_INFO "kshrammod: read %zu bytes @ %llu.\n", len, *off);
	return len;
}

static ssize_t kshram_dev_write(struct file *f, const char __user *buf, size_t len, loff_t *off) {
	printk(KERN_INFO "kshrammod: write %zu bytes @ %llu.\n", len, *off);
	return len;
}

static long kshram_dev_ioctl(struct file *fp, unsigned int cmd, unsigned long arg) 
{
	struct kshram_dev *current_dev = fp->private_data;
	switch(cmd)
	{
		case KSHRAM_GETSLOTS:
			printk(KERN_INFO "kshrammod: number of slots is 8\n");
			return 8;
		case KSHRAM_GETSIZE:
			printk(KERN_INFO "kshrammod: size of the shared memory of kshram%d is %ld\n", current_dev->dev_num, current_dev->SIZE);
			return current_dev->SIZE;
		case KSHRAM_SETSIZE:
			current_dev->data = krealloc(current_dev->data, arg, GFP_KERNEL);

			current_dev->SIZE = arg;
			printk(KERN_INFO "kshrammod: resize shared memory to %ld\n", arg);
			break;
	}


	return 0;
}

static int kshram_dev_mmap ( struct file* f, struct vm_area_struct *vma)
{
	struct kshram_dev *current_dev = f->private_data;

    unsigned long phys_addr;
    unsigned long size;

    // Calculate size of memory region
    size = vma->vm_end - vma->vm_start;

    phys_addr = virt_to_phys(current_dev->data);

    // Remap the physical memory region to the user space virtual address
    if (remap_pfn_range(vma, vma->vm_start, phys_addr >> PAGE_SHIFT, size, vma->vm_page_prot)) {
        return -EAGAIN;
    }

    return 0;
}

static const struct file_operations kshram_dev_fops = {
	.owner = THIS_MODULE,
	.open = kshram_dev_open,
	.read = kshram_dev_read,
	.write = kshram_dev_write,
	.unlocked_ioctl = kshram_dev_ioctl,
	.release = kshram_dev_close,
	.mmap = kshram_dev_mmap
};

static int kshram_proc_read(struct seq_file *m, void *v) {
	for ( int i =0;i<NUM_DEVICES;i++)
	{
		char tmp[50];
		sprintf(tmp, "0%d: %ld\n", i, kshram_devices[i].SIZE);
		seq_printf(m, tmp);
	}
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
	if(alloc_chrdev_region(&devnum, 0, NUM_DEVICES, "kshram") < 0)
		return -1;
	if((clazz = class_create(THIS_MODULE, "kshram_class")) == NULL)
		goto release_region;
	clazz->devnode = kshram_devnode;

	for ( int i=0; i<NUM_DEVICES; i++)
	{
		cdev_init(&kshram_devices[i].cdev, &kshram_dev_fops);
		// char name;
		// sprintf(name, "kshram%d", i);
		if(device_create(clazz, NULL, MKDEV(MAJOR(devnum), i) , NULL, "kshram%d", i) == NULL)
		{
			goto release_class;
		}

		
    	if(cdev_add(&kshram_devices[i].cdev, MKDEV(MAJOR(devnum), MINOR(devnum) + i), 1) == -1) 
		{
        	goto release_device;
    	}

		kshram_devices[i].data = kzalloc(KSHRAM_SIZE, GFP_KERNEL);
        if (!kshram_devices[i].data) 
		{
            goto release_data;
        }
		kshram_devices[i].SIZE = KSHRAM_SIZE;

		kshram_devices[i].dev_num = i;

		
	}		

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
	cdev_del(&c_dev);
release_data:
	for ( int i=0;i<NUM_DEVICES;i++) kfree(kshram_devices[i].data);
release_class:
	class_destroy(clazz);
release_region:
	unregister_chrdev_region(devnum, 8);
	return -1;
}

static void __exit kshram_cleanup(void)
{
	remove_proc_entry("kshram", NULL);

	for ( int i = 0;i<8;i++)
	{
		device_destroy(clazz, MKDEV(MAJOR(devnum), i));
		unregister_chrdev_region(MKDEV(MAJOR(devnum), i), 1);
		cdev_del(&kshram_devices[i].cdev);
	}
	
	// cdev_del(&c_dev);
	device_destroy(clazz, devnum);
	class_destroy(clazz);

	printk(KERN_INFO "kshram: cleaned up.\n");
}

module_init(kshram_init);
module_exit(kshram_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tan-Wen Chang");
MODULE_DESCRIPTION("The unix programming course demo kernel module.");
