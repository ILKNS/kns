#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/module.h>

#include <kns/ioctl.h>
#include <kns/mem.h>

void* mem_start = NULL;
void* mem_phy_base = NULL;
static dev_t dev;
static struct cdev c_dev;
static struct class *uio_cls = NULL;

static long uio_ioctl(struct file *File, unsigned int cmd, unsigned long args){
	switch(cmd){
		case PASSING_MEM_ADDR:
			if(copy_from_user(&mem_start, (int*)args, sizeof(unsigned long)))
				return -EACCES;
			printk(KERN_DEBUG "ADDR received: %p", mem_start);
			mem_phy_base = mmap_hp_u2k(mem_start);
			printk(KERN_DEBUG "ADDR mapped to kernel: %p", mem_phy_base);
			break;
		default:
			return -EINVAL;
	}
	return 0;
}

static struct file_operations uio_fops =
{
	.unlocked_ioctl = uio_ioctl,
};

int uio_init(void){
	int ret;
	struct device *dev_ret;

	if((ret = alloc_chrdev_region(&dev, FIRST_MINOR, MINOR_CNT, DEVICE_NAME"_CDEV"))<0)
		goto err_alloc_chrdev;

	if(IS_ERR(uio_cls = class_create(THIS_MODULE, DEVICE_NAME"_CLS")))
		goto err_cls_create;

	cdev_init(&c_dev, &uio_fops);
	if((ret = cdev_add(&c_dev, dev, MINOR_CNT)) < 0)
		goto err_cdev_add;
	if(IS_ERR(dev_ret = device_create(uio_cls, NULL, dev, NULL, DEVICE_NAME"_DEV")))
		goto err_dev_create;

	printk(KERN_DEBUG "Registered character device.");
	return 0;

err_dev_create:
err_cdev_add:
	cdev_del(&c_dev);
err_cls_create:
	class_destroy(uio_cls);
err_alloc_chrdev:
	unregister_chrdev_region(dev, MINOR_CNT);
	return ret;
}

void uio_remove(void){
	device_destroy(uio_cls, dev);
    class_destroy(uio_cls);
    cdev_del(&c_dev);
    unregister_chrdev_region(dev, MINOR_CNT);
}