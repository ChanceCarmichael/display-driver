/*
 * main.c - USB-C to Quad HDMI Display Adapter Kernel Driver
 * 
 * StarTech.com USBC2HD4 device driver for Linux
 * Main module initialization and cleanup
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/usb.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include "usbc2hd4.h"

MODULE_AUTHOR("StarTech Driver Developer");
MODULE_DESCRIPTION("StarTech USB-C to Quad HDMI Display Adapter Kernel Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1.0");

/* USB device IDs supported by this driver */
static const struct usb_device_id usbc2hd4_id_table[] = {
	{ USB_DEVICE(USBC2HD4_VENDOR_ID, USBC2HD4_PRODUCT_ID) },
	{ USB_DEVICE(MCT_VENDOR_ID, MCT_T6_PRODUCT_ID) },  /* Magic Control T6 variant */
	{},  /* Terminating entry */
};
MODULE_DEVICE_TABLE(usb, usbc2hd4_id_table);

/* Character device variables */
static int next_minor = 0;

/* USB Driver Structure */
static struct usb_driver usbc2hd4_driver = {
	.name = "usbc2hd4",
	.probe = usbc2hd4_probe,
	.disconnect = usbc2hd4_disconnect,
	.id_table = usbc2hd4_id_table,
};

/**
 * usbc2hd4_module_init - Driver module initialization
 */
static int __init usbc2hd4_module_init(void)
{
	int retval;

	pr_info("USBC2HD4: Initializing StarTech USB-C to Quad HDMI driver\n");

	/* Initialize character device system */
	retval = usbc2hd4_chardev_init();
	if (retval < 0) {
		return retval;
	}

	/* Register USB driver */
	retval = usb_register(&usbc2hd4_driver);
	if (retval) {
		pr_err("USBC2HD4: Failed to register USB driver\n");
		usbc2hd4_chardev_cleanup();
		return retval;
	}

	pr_info("USBC2HD4: Driver initialized successfully\n");
	return 0;
}

/**
 * usbc2hd4_module_exit - Driver module cleanup
 */
static void __exit usbc2hd4_module_exit(void)
{
	pr_info("USBC2HD4: Cleaning up StarTech USB-C to Quad HDMI driver\n");

	usb_deregister(&usbc2hd4_driver);
	usbc2hd4_chardev_cleanup();

	pr_info("USBC2HD4: Driver cleanup completed\n");
}

module_init(usbc2hd4_module_init);
module_exit(usbc2hd4_module_exit);
