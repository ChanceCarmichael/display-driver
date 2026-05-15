/*
 * hotplug.c - Hot-plug Detection and Event Handling
 * 
 * Monitors display connection/disconnection events via interrupt URB
 * and triggers hotplug event handling
 */

#include <linux/kernel.h>
#include <linux/usb.h>
#include <linux/workqueue.h>
#include <linux/slab.h>
#include "usbc2hd4.h"

#define HOTPLUG_URB_ENDPOINT  1
#define HOTPLUG_BUFFER_SIZE   64

/**
 * usbc2hd4_hotplug_work - Work queue handler for hotplug events
 * @work: Work structure
 * 
 * Called when a hotplug event is detected
 * Re-enumerates displays and updates device state
 */
static void usbc2hd4_hotplug_work(struct work_struct *work)
{
	struct usbc2hd4_device *dev = container_of(work, 
					struct usbc2hd4_device, hotplug_work);
	int retval;

	if (!dev || !dev->connected) {
		return;
	}

	pr_info("USBC2HD4: Hotplug event detected\n");

	/* Re-enumerate displays */
	retval = usbc2hd4_enumerate_displays(dev);
	if (retval) {
		pr_err("USBC2HD4: Failed to re-enumerate displays\n");
	}

	/* TODO: Send uevent to userspace for display configuration tools */
	/* kobject_uevent(&dev->dev->kobj, KOBJ_CHANGE); */
}

/**
 * usbc2hd4_hotplug_irq - Interrupt handler for hotplug events
 * @urb: USB request block
 * 
 * Called when interrupt URB completes
 * Schedules hotplug work and resubmits URB
 */
static void usbc2hd4_hotplug_irq(struct urb *urb)
{
	struct usbc2hd4_device *dev = urb->context;
	int retval;
	int status = urb->status;

	if (!dev || !dev->connected) {
		return;
	}

	switch (status) {
	case 0:
		/* Interrupt received, schedule hotplug work */
		pr_debug("USBC2HD4: Hotplug interrupt received\n");
		queue_work(dev->hotplug_wq, &dev->hotplug_work);
		break;

	case -ECONNRESET:
	case -ENOENT:
	case -ESHUTDOWN:
		/* URB was killed, don't resubmit */
		pr_debug("USBC2HD4: Hotplug URB cancelled (status %d)\n", status);
		return;

	default:
		pr_warn("USBC2HD4: Hotplug URB error (status %d)\n", status);
		break;
	}

	/* Resubmit the URB */
	retval = usb_submit_urb(urb, GFP_ATOMIC);
	if (retval) {
		pr_err("USBC2HD4: Failed to resubmit hotplug URB (error %d)\n", retval);
	}
}

/**
 * usbc2hd4_setup_hotplug - Setup hotplug detection
 * @dev: Device structure
 * 
 * Initializes interrupt URB for hotplug event detection
 */
int usbc2hd4_setup_hotplug(struct usbc2hd4_device *dev)
{
	struct usb_interface *intf;
	struct usb_endpoint_descriptor *endpoint;
	int pipe;
	int retval;

	if (!dev || !dev->intf) {
		pr_err("USBC2HD4: Invalid device for hotplug setup\n");
		return -EINVAL;
	}

	intf = dev->intf;

	/* Get interrupt endpoint descriptor */
	if (intf->cur_altsetting->desc.bNumEndpoints < 1) {
		pr_warn("USBC2HD4: No interrupt endpoint found\n");
		return 0;  /* Not fatal, continue without hotplug */
	}

	endpoint = &intf->cur_altsetting->endpoint[0].desc;
	if (!usb_endpoint_is_int_in(endpoint)) {
		pr_warn("USBC2HD4: First endpoint is not interrupt in\n");
		return 0;
	}

	/* Allocate interrupt buffer */
	dev->irq_buffer = kmalloc(HOTPLUG_BUFFER_SIZE, GFP_KERNEL);
	if (!dev->irq_buffer) {
		pr_err("USBC2HD4: Failed to allocate hotplug buffer\n");
		return -ENOMEM;
	}

	/* Allocate URB */
	dev->irq_urb = usb_alloc_urb(0, GFP_KERNEL);
	if (!dev->irq_urb) {
		pr_err("USBC2HD4: Failed to allocate hotplug URB\n");
		kfree(dev->irq_buffer);
		return -ENOMEM;
	}

	/* Create workqueue for hotplug events */
	dev->hotplug_wq = create_singlethread_workqueue("usbc2hd4_hotplug");
	if (!dev->hotplug_wq) {
		pr_err("USBC2HD4: Failed to create hotplug workqueue\n");
		usb_free_urb(dev->irq_urb);
		kfree(dev->irq_buffer);
		return -ENOMEM;
	}

	/* Initialize work structure */
	INIT_WORK(&dev->hotplug_work, usbc2hd4_hotplug_work);

	/* Setup USB pipe for interrupt endpoint */
	pipe = usb_rcvintpipe(dev->udev, endpoint->bEndpointAddress);

	/* Setup URB */
	usb_fill_int_urb(dev->irq_urb, dev->udev, pipe,
			 dev->irq_buffer, HOTPLUG_BUFFER_SIZE,
			 usbc2hd4_hotplug_irq, dev,
			 endpoint->bInterval);

	/* Submit URB */
	retval = usb_submit_urb(dev->irq_urb, GFP_KERNEL);
	if (retval) {
		pr_err("USBC2HD4: Failed to submit hotplug URB (error %d)\n", retval);
		destroy_workqueue(dev->hotplug_wq);
		usb_free_urb(dev->irq_urb);
		kfree(dev->irq_buffer);
		return retval;
	}

	pr_info("USBC2HD4: Hotplug detection initialized\n");
	return 0;
}

/**
 * usbc2hd4_shutdown_hotplug - Shutdown hotplug detection
 * @dev: Device structure
 * 
 * Stops hotplug interrupt URB and cleans up resources
 */
void usbc2hd4_shutdown_hotplug(struct usbc2hd4_device *dev)
{
	if (!dev) {
		return;
	}

	/* Kill hotplug URB */
	if (dev->irq_urb) {
		usb_kill_urb(dev->irq_urb);
		usb_free_urb(dev->irq_urb);
		dev->irq_urb = NULL;
	}

	/* Flush and destroy workqueue */
	if (dev->hotplug_wq) {
		flush_workqueue(dev->hotplug_wq);
		destroy_workqueue(dev->hotplug_wq);
		dev->hotplug_wq = NULL;
	}

	/* Free interrupt buffer */
	if (dev->irq_buffer) {
		kfree(dev->irq_buffer);
		dev->irq_buffer = NULL;
	}

	pr_info("USBC2HD4: Hotplug detection shutdown\n");
}
