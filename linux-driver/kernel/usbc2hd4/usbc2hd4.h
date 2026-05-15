/*
 * usbc2hd4.h - USB-C to Quad HDMI Display Adapter Kernel Driver Header
 * 
 * StarTech.com USBC2HD4 device driver for Linux
 * Device: StarTech USB-C to Quad HDMI Adapter
 * Chipset: Trigger T6-688SL (IT66122 USB to HDMI bridge)
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef _USBC2HD4_H_
#define _USBC2HD4_H_

#include <linux/kernel.h>
#include <linux/usb.h>
#include <linux/device.h>
#include <linux/workqueue.h>
#include <linux/ioctl.h>

/* USB Vendor and Product IDs */
#define USBC2HD4_VENDOR_ID   0x0bda  /* Realtek */
#define USBC2HD4_PRODUCT_ID  0x8153  /* Trigger T6-688SL */

/* Magic Control Technology T6 variant */
#define MCT_VENDOR_ID        0x0711  /* Magic Control Technology */
#define MCT_T6_PRODUCT_ID    0x5601  /* T6 USB Station */

/* Device limits */
#define USBC2HD4_MAX_DISPLAYS   4
#define USBC2HD4_MAX_RESOLUTIONS 16

/* IT66122 Chip Command Protocol */
#define IT66122_CMD_GET_STATUS          0x01
#define IT66122_CMD_SET_MODE            0x02
#define IT66122_CMD_GET_DISPLAY_INFO    0x03
#define IT66122_CMD_SET_RESOLUTION      0x04
#define IT66122_CMD_SET_ORIENTATION     0x05
#define IT66122_CMD_ENABLE_DISPLAY      0x06
#define IT66122_CMD_DISABLE_DISPLAY     0x07
#define IT66122_CMD_ENUMERATE_DISPLAYS  0x08

/* Display Orientation */
enum display_orientation {
	ORIENTATION_LANDSCAPE = 0,
	ORIENTATION_PORTRAIT = 1,
};

/* Device Control Ioctls */
#define USBC2HD4_IOC_MAGIC		'U'
#define USBC2HD4_IOCTL_GET_DEVICE_INFO		_IOR(USBC2HD4_IOC_MAGIC, 1, struct usbc2hd4_device_info)
#define USBC2HD4_IOCTL_ENUMERATE_DISPLAYS	_IO(USBC2HD4_IOC_MAGIC, 2)
#define USBC2HD4_IOCTL_GET_DISPLAY_INFO		_IOR(USBC2HD4_IOC_MAGIC, 3, struct usbc2hd4_display_info)
#define USBC2HD4_IOCTL_SET_RESOLUTION		_IOW(USBC2HD4_IOC_MAGIC, 4, struct usbc2hd4_resolution_cmd)
#define USBC2HD4_IOCTL_ENABLE_DISPLAY		_IOW(USBC2HD4_IOC_MAGIC, 5, uint8_t)
#define USBC2HD4_IOCTL_DISABLE_DISPLAY		_IOW(USBC2HD4_IOC_MAGIC, 6, uint8_t)

/* Device Info Structure */
struct usbc2hd4_device_info {
	uint16_t vendor_id;
	uint16_t product_id;
	uint8_t num_displays;
	uint8_t reserved[3];
	char product_name[32];
	char serial_number[32];
};

/* Display Info Structure */
struct usbc2hd4_display_info {
	uint8_t display_id;
	uint8_t connected;
	uint8_t enabled;
	uint8_t reserved;
	uint16_t width;
	uint16_t height;
	uint8_t refresh_rate;
	uint8_t orientation;
};

/* Resolution Command */
struct usbc2hd4_resolution_cmd {
	uint8_t display_id;
	uint8_t reserved[3];
	uint16_t width;
	uint16_t height;
	uint8_t refresh_rate;
	uint8_t reserved2;
};

/* Display Status Flags */
#define DISPLAY_CONNECTED       (1 << 0)
#define DISPLAY_ENABLED         (1 << 1)
#define DISPLAY_HOTPLUG         (1 << 2)

/* Resolution Definition */
struct display_resolution {
	uint16_t width;
	uint16_t height;
	uint8_t refresh_rate;
	uint8_t reserved;
};

/* Display Information */
struct display_info {
	uint8_t id;                              /* Display index 0-3 */
	uint8_t status;                          /* Connection status flags */
	enum display_orientation orientation;    /* Current orientation */
	struct display_resolution resolution;    /* Current resolution */
	uint32_t edid_length;                    /* EDID data length */
	uint8_t edid_data[256];                  /* EDID raw data */
};

/* Device Structure */
struct usbc2hd4_device {
	struct usb_device *udev;                 /* USB device handle */
	struct usb_interface *intf;              /* USB interface */
	
	struct cdev cdev;                        /* Character device */
	struct device *dev;                      /* Linux device */
	int minor;                               /* Character device minor number */
	
	int num_displays;                        /* Number of connected displays */
	struct display_info displays[USBC2HD4_MAX_DISPLAYS];
	
	struct urb *irq_urb;                     /* Interrupt URB for hotplug detection */
	unsigned char *irq_buffer;               /* Interrupt buffer */
	
	struct workqueue_struct *hotplug_wq;     /* Workqueue for hotplug events */
	struct work_struct hotplug_work;
	
	struct mutex lock;                       /* Device state mutex */
	bool connected;                          /* Device connection state */
	
	void *priv;                              /* Private driver data */
};

/* Function Prototypes */
int usbc2hd4_probe(struct usb_interface *intf, const struct usb_device_id *id);
void usbc2hd4_disconnect(struct usb_interface *intf);

int usbc2hd4_enumerate_displays(struct usbc2hd4_device *dev);

/* Character device functions */
int usbc2hd4_chardev_init(void);
void usbc2hd4_chardev_cleanup(void);
int usbc2hd4_chardev_create(struct usbc2hd4_device *dev, int minor);
void usbc2hd4_chardev_remove(struct usbc2hd4_device *dev);
struct class * usbc2hd4_get_class(void);
int usbc2hd4_get_display_info(struct usbc2hd4_device *dev, uint8_t display_id,
			      struct display_info *info);
int usbc2hd4_set_resolution(struct usbc2hd4_device *dev, uint8_t display_id,
			    uint16_t width, uint16_t height, uint8_t refresh_rate);
int usbc2hd4_set_orientation(struct usbc2hd4_device *dev, uint8_t display_id,
			     enum display_orientation orientation);
int usbc2hd4_enable_display(struct usbc2hd4_device *dev, uint8_t display_id);
int usbc2hd4_disable_display(struct usbc2hd4_device *dev, uint8_t display_id);

/* IT66122 Communication */
int it66122_send_command(struct usbc2hd4_device *dev, uint8_t cmd,
			 const void *data, size_t data_len,
			 void *response, size_t *response_len);

#endif /* _USBC2HD4_H_ */
