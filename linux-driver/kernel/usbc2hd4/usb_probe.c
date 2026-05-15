/*
 * usb_probe.c - USB device detection and probe functions
 * 
 * Handles USB device attachment, initialization, and cleanup
 */

#include <linux/kernel.h>
#include <linux/usb.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include "usbc2hd4.h"

/**
 * usbc2hd4_probe - Device probe callback
 * @intf: USB interface
 * @id: USB device ID
 * 
 * Called when a USB device matching our ID table is attached
 */
int usbc2hd4_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
	struct usbc2hd4_device *dev;
	struct usb_device *udev = interface_to_usbdev(intf);
	int retval = 0;
	int i;

	pr_info("USBC2HD4: Detected USB device (vendor:%04x product:%04x)\n",
		le16_to_cpu(udev->descriptor.idVendor),
		le16_to_cpu(udev->descriptor.idProduct));

	/* Allocate device structure */
	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev) {
		pr_err("USBC2HD4: Failed to allocate device structure\n");
		return -ENOMEM;
	}

	/* Initialize device structure */
	dev->udev = usb_get_dev(udev);
	dev->intf = intf;
	dev->connected = true;
	mutex_init(&dev->lock);

	/* Initialize display array */
	for (i = 0; i < USBC2HD4_MAX_DISPLAYS; i++) {
		dev->displays[i].id = i;
		dev->displays[i].status = 0;
		dev->displays[i].orientation = ORIENTATION_LANDSCAPE;
		dev->displays[i].resolution.width = 1920;
		dev->displays[i].resolution.height = 1080;
		dev->displays[i].resolution.refresh_rate = 60;
	}
	dev->num_displays = USBC2HD4_MAX_DISPLAYS;

	/* Create workqueue for hotplug events */
	dev->hotplug_wq = create_workqueue("usbc2hd4_hotplug");
	if (!dev->hotplug_wq) {
		pr_err("USBC2HD4: Failed to create workqueue\n");
		retval = -ENOMEM;
		goto error_wq;
	}

	/* Allocate interrupt buffer for hotplug detection */
	dev->irq_buffer = kmalloc(8, GFP_KERNEL);
	if (!dev->irq_buffer) {
		pr_err("USBC2HD4: Failed to allocate interrupt buffer\n");
		retval = -ENOMEM;
		goto error_irq_buffer;
	}

	/* Store device reference in USB interface */
	usb_set_intfdata(intf, dev);

	/* Allocate minor number and create character device */
	static int next_minor = 0;
	if (next_minor < 10) {  /* Support up to 10 devices */
		retval = usbc2hd4_chardev_create(dev, next_minor);
		if (retval) {
			pr_err("USBC2HD4: Failed to create character device\n");
			goto error_chardev;
		}
		next_minor++;
	} else {
		pr_warn("USBC2HD4: Exceeded maximum supported devices\n");
		goto error_chardev;
	}
	
	pr_info("USBC2HD4: Device probe successful\n");
	return 0;

error_chardev:
	usb_set_intfdata(intf, NULL);
	destroy_workqueue(dev->hotplug_wq);
error_wq:
	usb_put_dev(dev->udev);
	kfree(dev);
	return retval;
}

/**
 * usbc2hd4_disconnect - Device disconnect callback
 * @intf: USB interface
 * 
 * Called when the USB device is removed
 */
void usbc2hd4_disconnect(struct usb_interface *intf)
{
	struct usbc2hd4_device *dev = usb_get_intfdata(intf);

	if (!dev) {
		pr_warn("USBC2HD4: Disconnect called with NULL device\n");
		return;
	}

	pr_info("USBC2HD4: Device disconnect\n");

	mutex_lock(&dev->lock);
	dev->connected = false;
	mutex_unlock(&dev->lock);

	usbc2hd4_chardev_remove(dev);

	/* Cancel any pending work */
	flush_workqueue(dev->hotplug_wq);
	destroy_workqueue(dev->hotplug_wq);

	/* Cancel any pending URBs */
	if (dev->irq_urb) {
		usb_kill_urb(dev->irq_urb);
		usb_free_urb(dev->irq_urb);
	}

	/* Free resources */
	kfree(dev->irq_buffer);
	usb_set_intfdata(intf, NULL);
	usb_put_dev(dev->udev);
	kfree(dev);

	pr_info("USBC2HD4: Device disconnect completed\n");
}
