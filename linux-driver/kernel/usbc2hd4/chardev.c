/*
 * chardev.c - Character device interface for USBC2HD4 driver
 * 
 * Provides /dev/usbc2hd4X character device for user-space communication
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include "usbc2hd4.h"

/* Global character device infrastructure */
static struct class *usbc2hd4_class = NULL;
static int usbc2hd4_major = 0;

/**
 * usbc2hd4_open - Open character device
 */
static int usbc2hd4_chardev_open(struct inode *inode, struct file *filp)
{
	struct usbc2hd4_device *dev;
	
	dev = container_of(inode->i_cdev, struct usbc2hd4_device, cdev);
	if (!dev) {
		return -ENODEV;
	}
	
	filp->private_data = dev;
	return 0;
}

/**
 * usbc2hd4_release - Close character device
 */
static int usbc2hd4_chardev_release(struct inode *inode, struct file *filp)
{
	filp->private_data = NULL;
	return 0;
}

/**
 * usbc2hd4_ioctl - Process ioctl commands
 */
static long usbc2hd4_chardev_ioctl(struct file *filp, unsigned int cmd,
				   unsigned long arg)
{
	struct usbc2hd4_device *dev = filp->private_data;
	struct usbc2hd4_device_info dev_info;
	struct usbc2hd4_display_info disp_info;
	struct usbc2hd4_resolution_cmd res_cmd;
	uint8_t display_id;
	int ret = 0;
	
	if (!dev) {
		return -ENODEV;
	}
	
	mutex_lock(&dev->lock);
	
	if (!dev->connected) {
		mutex_unlock(&dev->lock);
		return -ENODEV;
	}
	
	switch (cmd) {
	case USBC2HD4_IOCTL_GET_DEVICE_INFO:
		memset(&dev_info, 0, sizeof(dev_info));
		dev_info.vendor_id = le16_to_cpu(dev->udev->descriptor.idVendor);
		dev_info.product_id = le16_to_cpu(dev->udev->descriptor.idProduct);
		dev_info.num_displays = dev->num_displays;
		strncpy(dev_info.product_name, "USBC2HD4", sizeof(dev_info.product_name) - 1);
		snprintf(dev_info.serial_number, sizeof(dev_info.serial_number), 
			 "%s", dev->udev->serial ?: "N/A");
		
		if (copy_to_user((void __user *)arg, &dev_info, sizeof(dev_info))) {
			ret = -EFAULT;
		}
		break;
	
	case USBC2HD4_IOCTL_ENUMERATE_DISPLAYS:
		/* Return number of displays */
		ret = dev->num_displays;
		break;
	
	case USBC2HD4_IOCTL_GET_DISPLAY_INFO:
		if (copy_from_user(&disp_info, (void __user *)arg, sizeof(disp_info))) {
			ret = -EFAULT;
			break;
		}
		
		display_id = disp_info.display_id;
		if (display_id >= USBC2HD4_MAX_DISPLAYS) {
			ret = -EINVAL;
			break;
		}
		
		/* Fill display info */
		disp_info.connected = (dev->displays[display_id].status & DISPLAY_CONNECTED) ? 1 : 0;
		disp_info.enabled = (dev->displays[display_id].status & DISPLAY_ENABLED) ? 1 : 0;
		disp_info.width = dev->displays[display_id].resolution.width;
		disp_info.height = dev->displays[display_id].resolution.height;
		disp_info.refresh_rate = dev->displays[display_id].resolution.refresh_rate;
		disp_info.orientation = dev->displays[display_id].orientation;
		
		if (copy_to_user((void __user *)arg, &disp_info, sizeof(disp_info))) {
			ret = -EFAULT;
		}
		break;
	
	case USBC2HD4_IOCTL_SET_RESOLUTION:
		if (copy_from_user(&res_cmd, (void __user *)arg, sizeof(res_cmd))) {
			ret = -EFAULT;
			break;
		}
		
		display_id = res_cmd.display_id;
		if (display_id >= USBC2HD4_MAX_DISPLAYS) {
			ret = -EINVAL;
			break;
		}
		
		pr_info("USBC2HD4: Setting display %d to %ux%u@%uHz\n",
			display_id, res_cmd.width, res_cmd.height, res_cmd.refresh_rate);
		
		/* Update cached resolution */
		dev->displays[display_id].resolution.width = res_cmd.width;
		dev->displays[display_id].resolution.height = res_cmd.height;
		dev->displays[display_id].resolution.refresh_rate = res_cmd.refresh_rate;
		break;
	
	case USBC2HD4_IOCTL_ENABLE_DISPLAY:
		if (copy_from_user(&display_id, (void __user *)arg, sizeof(display_id))) {
			ret = -EFAULT;
			break;
		}
		
		if (display_id >= USBC2HD4_MAX_DISPLAYS) {
			ret = -EINVAL;
			break;
		}
		
		pr_info("USBC2HD4: Enabling display %d\n", display_id);
		dev->displays[display_id].status |= DISPLAY_ENABLED;
		break;
	
	case USBC2HD4_IOCTL_DISABLE_DISPLAY:
		if (copy_from_user(&display_id, (void __user *)arg, sizeof(display_id))) {
			ret = -EFAULT;
			break;
		}
		
		if (display_id >= USBC2HD4_MAX_DISPLAYS) {
			ret = -EINVAL;
			break;
		}
		
		pr_info("USBC2HD4: Disabling display %d\n", display_id);
		dev->displays[display_id].status &= ~DISPLAY_ENABLED;
		break;
	
	default:
		ret = -ENOTTY;
		break;
	}
	
	mutex_unlock(&dev->lock);
	return ret;
}

/* File operations */
static const struct file_operations usbc2hd4_fops = {
	.owner = THIS_MODULE,
	.open = usbc2hd4_chardev_open,
	.release = usbc2hd4_chardev_release,
	.unlocked_ioctl = usbc2hd4_chardev_ioctl,
};

/**
 * usbc2hd4_chardev_create - Create character device for a USB device
 */
int usbc2hd4_chardev_create(struct usbc2hd4_device *dev, int minor)
{
	dev_t devnum;
	struct device *device;
	int ret = 0;
	
	if (!usbc2hd4_class) {
		return -ENODEV;
	}
	
	devnum = MKDEV(usbc2hd4_major, minor);
	
	/* Initialize character device */
	cdev_init(&dev->cdev, &usbc2hd4_fops);
	dev->cdev.owner = THIS_MODULE;
	
	/* Add character device */
	ret = cdev_add(&dev->cdev, devnum, 1);
	if (ret) {
		pr_err("USBC2HD4: Failed to add character device: %d\n", ret);
		return ret;
	}
	
	/* Create device file */
	device = device_create(usbc2hd4_class, NULL, devnum, NULL,
			       "usbc2hd4%d", minor);
	if (IS_ERR(device)) {
		pr_err("USBC2HD4: Failed to create device file\n");
		cdev_del(&dev->cdev);
		return PTR_ERR(device);
	}
	
	dev->minor = minor;
	return 0;
}

/**
 * usbc2hd4_chardev_remove - Remove character device
 */
void usbc2hd4_chardev_remove(struct usbc2hd4_device *dev)
{
	if (!usbc2hd4_class) {
		return;
	}
	
	device_destroy(usbc2hd4_class, MKDEV(usbc2hd4_major, dev->minor));
	cdev_del(&dev->cdev);
}

/**
 * usbc2hd4_chardev_init - Initialize character device system
 */
int usbc2hd4_chardev_init(void)
{
	dev_t dev_num;
	int ret;
	
	/* Allocate character device number */
	ret = alloc_chrdev_region(&dev_num, 0, 10, "usbc2hd4");
	if (ret < 0) {
		pr_err("USBC2HD4: Failed to allocate character device region\n");
		return ret;
	}
	
	usbc2hd4_major = MAJOR(dev_num);
	
	/* Create device class */
	usbc2hd4_class = class_create(THIS_MODULE, "usbc2hd4");
	if (IS_ERR(usbc2hd4_class)) {
		pr_err("USBC2HD4: Failed to create device class\n");
		unregister_chrdev_region(dev_num, 10);
		return PTR_ERR(usbc2hd4_class);
	}
	
	pr_info("USBC2HD4: Character device initialized (major: %d)\n", usbc2hd4_major);
	return 0;
}

/**
 * usbc2hd4_chardev_cleanup - Clean up character device system
 */
void usbc2hd4_chardev_cleanup(void)
{
	if (usbc2hd4_class) {
		class_destroy(usbc2hd4_class);
		usbc2hd4_class = NULL;
	}
	
	if (usbc2hd4_major) {
		unregister_chrdev_region(MKDEV(usbc2hd4_major, 0), 10);
		usbc2hd4_major = 0;
	}
	
	pr_info("USBC2HD4: Character device cleaned up\n");
}

/* Export functions for main driver */
struct class * usbc2hd4_get_class(void)
{
	return usbc2hd4_class;
}

EXPORT_SYMBOL(usbc2hd4_chardev_create);
EXPORT_SYMBOL(usbc2hd4_chardev_remove);
EXPORT_SYMBOL(usbc2hd4_chardev_init);
EXPORT_SYMBOL(usbc2hd4_chardev_cleanup);
EXPORT_SYMBOL(usbc2hd4_get_class);
