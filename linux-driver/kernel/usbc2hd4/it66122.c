/*
 * it66122.c - IT66122 USB to HDMI Bridge Communication
 * 
 * Implements communication protocol with the Trigger T6-688SL chipset
 * which uses an IT66122 USB to HDMI bridge controller
 */

#include <linux/kernel.h>
#include <linux/usb.h>
#include <linux/slab.h>
#include "usbc2hd4.h"

/* IT66122 USB Control Transfer Constants */
#define IT66122_CTRL_TIMEOUT_MS  1000
#define IT66122_CTRL_BUFFER_SIZE 256

/* USB Control Request Types */
#define IT66122_REQUEST_TYPE_OUT  (USB_DIR_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE)
#define IT66122_REQUEST_TYPE_IN   (USB_DIR_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE)

/**
 * it66122_send_command - Send command to IT66122 chip
 * @dev: Device structure
 * @cmd: Command ID
 * @data: Command data buffer
 * @data_len: Command data length
 * @response: Response buffer
 * @response_len: Response length pointer
 * 
 * Sends a command to the IT66122 chip via USB control transfer
 * and receives the response
 */
int it66122_send_command(struct usbc2hd4_device *dev, uint8_t cmd,
			 const void *data, size_t data_len,
			 void *response, size_t *response_len)
{
	unsigned char *cmd_buffer = NULL;
	unsigned char *resp_buffer = NULL;
	int retval = 0;
	size_t max_data_len = IT66122_CTRL_BUFFER_SIZE - 1;
	int actual_len;

	if (!dev || !dev->udev || !dev->connected) {
		pr_err("USBC2HD4: Device not connected or invalid\n");
		return -ENODEV;
	}

	if (data_len > max_data_len) {
		pr_err("USBC2HD4: Command data too large (%zu > %zu)\n",
		       data_len, max_data_len);
		return -EINVAL;
	}

	/* Allocate command buffer */
	cmd_buffer = kmalloc(IT66122_CTRL_BUFFER_SIZE, GFP_KERNEL);
	if (!cmd_buffer) {
		pr_err("USBC2HD4: Failed to allocate command buffer\n");
		return -ENOMEM;
	}

	/* Allocate response buffer */
	resp_buffer = kmalloc(IT66122_CTRL_BUFFER_SIZE, GFP_KERNEL);
	if (!resp_buffer) {
		pr_err("USBC2HD4: Failed to allocate response buffer\n");
		kfree(cmd_buffer);
		return -ENOMEM;
	}

	/* Build command packet */
	cmd_buffer[0] = cmd;
	if (data && data_len > 0) {
		memcpy(&cmd_buffer[1], data, data_len);
	}

	pr_debug("USBC2HD4: Sending command 0x%02x (data_len=%zu)\n", cmd, data_len);

	/* Send command via USB control transfer */
	retval = usb_control_msg(dev->udev,
				 usb_sndctrlpipe(dev->udev, 0),
				 0xB0,  /* Vendor-specific request */
				 IT66122_REQUEST_TYPE_OUT,
				 cmd,
				 0,
				 cmd_buffer,
				 data_len + 1,
				 IT66122_CTRL_TIMEOUT_MS);
	if (retval < 0) {
		pr_err("USBC2HD4: Failed to send command (error %d)\n", retval);
		goto cleanup;
	}

	/* Receive response */
	if (response && response_len) {
		retval = usb_control_msg(dev->udev,
					 usb_rcvctrlpipe(dev->udev, 0),
					 0xB1,  /* Vendor-specific request */
					 IT66122_REQUEST_TYPE_IN,
					 cmd,
					 0,
					 resp_buffer,
					 IT66122_CTRL_BUFFER_SIZE,
					 IT66122_CTRL_TIMEOUT_MS);
		if (retval < 0) {
			pr_err("USBC2HD4: Failed to receive response (error %d)\n", retval);
			goto cleanup;
		}

		actual_len = (retval > *response_len) ? *response_len : retval;
		memcpy(response, resp_buffer, actual_len);
		*response_len = actual_len;

		pr_debug("USBC2HD4: Received response (%d bytes)\n", actual_len);
	}

	retval = 0;

cleanup:
	kfree(cmd_buffer);
	kfree(resp_buffer);
	return retval;
}

/**
 * it66122_get_status - Get device status
 * @dev: Device structure
 * @status: Status buffer pointer
 * 
 * Queries the IT66122 for current device status
 */
int it66122_get_status(struct usbc2hd4_device *dev, uint8_t *status)
{
	uint8_t response[8];
	size_t response_len = sizeof(response);
	int retval;

	pr_debug("USBC2HD4: Getting device status\n");

	retval = it66122_send_command(dev, IT66122_CMD_GET_STATUS,
				      NULL, 0, response, &response_len);
	if (retval < 0) {
		pr_err("USBC2HD4: Failed to get status\n");
		return retval;
	}

	if (response_len > 0 && status) {
		*status = response[0];
	}

	return 0;
}
