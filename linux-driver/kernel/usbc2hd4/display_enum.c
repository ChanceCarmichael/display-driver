/*
 * display_enum.c - Display Enumeration and Info Retrieval
 * 
 * Implements display detection, enumeration, and information retrieval
 * from the USB-C to HDMI adapter via IT66122 chip
 */

#include <linux/kernel.h>
#include <linux/usb.h>
#include "usbc2hd4.h"

/**
 * usbc2hd4_enumerate_displays - Enumerate all connected displays
 * @dev: Device structure
 * 
 * Queries the IT66122 chip to detect and enumerate all connected HDMI displays
 * Updates dev->num_displays and device list
 */
int usbc2hd4_enumerate_displays(struct usbc2hd4_device *dev)
{
	uint8_t response[256];
	size_t response_len = sizeof(response);
	int retval;
	int i;

	if (!dev || !dev->connected) {
		pr_err("USBC2HD4: Device not connected\n");
		return -ENODEV;
	}

	mutex_lock(&dev->lock);

	pr_info("USBC2HD4: Enumerating connected displays\n");

	/* Send enumeration command to IT66122 */
	retval = it66122_send_command(dev, IT66122_CMD_ENUMERATE_DISPLAYS,
				      NULL, 0, response, &response_len);
	if (retval) {
		pr_err("USBC2HD4: Failed to enumerate displays (error %d)\n", retval);
		goto unlock;
	}

	/* Parse enumeration response */
	if (response_len < 1) {
		pr_err("USBC2HD4: Invalid response length from enumeration\n");
		retval = -EINVAL;
		goto unlock;
	}

	/* First byte contains number of displays */
	dev->num_displays = response[0];
	if (dev->num_displays > USBC2HD4_MAX_DISPLAYS) {
		pr_warn("USBC2HD4: Detected %d displays, clamping to %d\n",
			dev->num_displays, USBC2HD4_MAX_DISPLAYS);
		dev->num_displays = USBC2HD4_MAX_DISPLAYS;
	}

	pr_info("USBC2HD4: Found %d connected displays\n", dev->num_displays);

	/* Enumerate each display */
	for (i = 0; i < dev->num_displays; i++) {
		retval = usbc2hd4_get_display_info(dev, i, &dev->displays[i]);
		if (retval) {
			pr_warn("USBC2HD4: Failed to get info for display %d\n", i);
			continue;
		}
		pr_info("USBC2HD4: Display %d: %ux%u@%uHz\n", i,
			dev->displays[i].resolution.width,
			dev->displays[i].resolution.height,
			dev->displays[i].resolution.refresh_rate);
	}

unlock:
	mutex_unlock(&dev->lock);
	return retval;
}

/**
 * usbc2hd4_get_display_info - Get information about a specific display
 * @dev: Device structure
 * @display_id: Display index (0-3)
 * @info: Output display info structure
 * 
 * Retrieves detailed information about a display including resolution, 
 * orientation, status flags, and EDID data
 */
int usbc2hd4_get_display_info(struct usbc2hd4_device *dev, uint8_t display_id,
			      struct display_info *info)
{
	uint8_t cmd_data[2];
	uint8_t response[256];
	size_t response_len = sizeof(response);
	int retval;

	if (!dev || !info) {
		pr_err("USBC2HD4: Invalid parameters\n");
		return -EINVAL;
	}

	if (display_id >= USBC2HD4_MAX_DISPLAYS) {
		pr_err("USBC2HD4: Invalid display ID %d\n", display_id);
		return -EINVAL;
	}

	mutex_lock(&dev->lock);

	/* Prepare command: display ID */
	cmd_data[0] = display_id;

	/* Query display info from IT66122 */
	retval = it66122_send_command(dev, IT66122_CMD_GET_DISPLAY_INFO,
				      cmd_data, 1, response, &response_len);
	if (retval) {
		pr_err("USBC2HD4: Failed to get display info (error %d)\n", retval);
		goto unlock;
	}

	/* Parse response and populate display_info structure */
	if (response_len < 8) {
		pr_err("USBC2HD4: Invalid response length from get_display_info\n");
		retval = -EINVAL;
		goto unlock;
	}

	info->id = display_id;
	info->status = response[0];
	info->orientation = (enum display_orientation)response[1];
	info->resolution.width = (response[2] << 8) | response[3];
	info->resolution.height = (response[4] << 8) | response[5];
	info->resolution.refresh_rate = response[6];
	info->edid_length = 0;  /* TODO: Retrieve EDID data */

	if (info->status & DISPLAY_CONNECTED) {
		pr_debug("USBC2HD4: Display %d connected\n", display_id);
	}

unlock:
	mutex_unlock(&dev->lock);
	return retval;
}

/**
 * usbc2hd4_set_resolution - Set display resolution
 * @dev: Device structure
 * @display_id: Display index (0-3)
 * @width: Resolution width in pixels
 * @height: Resolution height in pixels
 * @refresh_rate: Refresh rate in Hz
 * 
 * Sets the resolution and refresh rate for a specific display
 */
int usbc2hd4_set_resolution(struct usbc2hd4_device *dev, uint8_t display_id,
			    uint16_t width, uint16_t height, uint8_t refresh_rate)
{
	uint8_t cmd_data[6];
	uint8_t response[16];
	size_t response_len = sizeof(response);
	int retval;

	if (!dev || !dev->connected) {
		return -ENODEV;
	}

	if (display_id >= USBC2HD4_MAX_DISPLAYS) {
		pr_err("USBC2HD4: Invalid display ID %d\n", display_id);
		return -EINVAL;
	}

	mutex_lock(&dev->lock);

	/* Prepare command data */
	cmd_data[0] = display_id;
	cmd_data[1] = (width >> 8) & 0xFF;
	cmd_data[2] = width & 0xFF;
	cmd_data[3] = (height >> 8) & 0xFF;
	cmd_data[4] = height & 0xFF;
	cmd_data[5] = refresh_rate;

	pr_info("USBC2HD4: Setting display %d resolution to %ux%u@%uHz\n",
		display_id, width, height, refresh_rate);

	/* Send resolution command */
	retval = it66122_send_command(dev, IT66122_CMD_SET_RESOLUTION,
				      cmd_data, 6, response, &response_len);
	if (retval) {
		pr_err("USBC2HD4: Failed to set resolution (error %d)\n", retval);
		goto unlock;
	}

	/* Update local cache */
	dev->displays[display_id].resolution.width = width;
	dev->displays[display_id].resolution.height = height;
	dev->displays[display_id].resolution.refresh_rate = refresh_rate;

unlock:
	mutex_unlock(&dev->lock);
	return retval;
}

/**
 * usbc2hd4_set_orientation - Set display orientation
 * @dev: Device structure
 * @display_id: Display index (0-3)
 * @orientation: Orientation (LANDSCAPE or PORTRAIT)
 * 
 * Sets the display orientation
 */
int usbc2hd4_set_orientation(struct usbc2hd4_device *dev, uint8_t display_id,
			     enum display_orientation orientation)
{
	uint8_t cmd_data[2];
	uint8_t response[16];
	size_t response_len = sizeof(response);
	int retval;

	if (!dev || !dev->connected) {
		return -ENODEV;
	}

	if (display_id >= USBC2HD4_MAX_DISPLAYS) {
		pr_err("USBC2HD4: Invalid display ID %d\n", display_id);
		return -EINVAL;
	}

	mutex_lock(&dev->lock);

	cmd_data[0] = display_id;
	cmd_data[1] = (uint8_t)orientation;

	pr_info("USBC2HD4: Setting display %d orientation to %d\n",
		display_id, orientation);

	retval = it66122_send_command(dev, IT66122_CMD_SET_ORIENTATION,
				      cmd_data, 2, response, &response_len);
	if (retval) {
		pr_err("USBC2HD4: Failed to set orientation (error %d)\n", retval);
		goto unlock;
	}

	dev->displays[display_id].orientation = orientation;

unlock:
	mutex_unlock(&dev->lock);
	return retval;
}

/**
 * usbc2hd4_enable_display - Enable a display output
 * @dev: Device structure
 * @display_id: Display index (0-3)
 * 
 * Enables video output on a display
 */
int usbc2hd4_enable_display(struct usbc2hd4_device *dev, uint8_t display_id)
{
	uint8_t cmd_data[1];
	uint8_t response[16];
	size_t response_len = sizeof(response);
	int retval;

	if (!dev || !dev->connected) {
		return -ENODEV;
	}

	if (display_id >= USBC2HD4_MAX_DISPLAYS) {
		pr_err("USBC2HD4: Invalid display ID %d\n", display_id);
		return -EINVAL;
	}

	mutex_lock(&dev->lock);

	cmd_data[0] = display_id;

	pr_info("USBC2HD4: Enabling display %d\n", display_id);

	retval = it66122_send_command(dev, IT66122_CMD_ENABLE_DISPLAY,
				      cmd_data, 1, response, &response_len);
	if (retval) {
		pr_err("USBC2HD4: Failed to enable display (error %d)\n", retval);
		goto unlock;
	}

	dev->displays[display_id].status |= DISPLAY_ENABLED;

unlock:
	mutex_unlock(&dev->lock);
	return retval;
}

/**
 * usbc2hd4_disable_display - Disable a display output
 * @dev: Device structure
 * @display_id: Display index (0-3)
 * 
 * Disables video output on a display
 */
int usbc2hd4_disable_display(struct usbc2hd4_device *dev, uint8_t display_id)
{
	uint8_t cmd_data[1];
	uint8_t response[16];
	size_t response_len = sizeof(response);
	int retval;

	if (!dev || !dev->connected) {
		return -ENODEV;
	}

	if (display_id >= USBC2HD4_MAX_DISPLAYS) {
		pr_err("USBC2HD4: Invalid display ID %d\n", display_id);
		return -EINVAL;
	}

	mutex_lock(&dev->lock);

	cmd_data[0] = display_id;

	pr_info("USBC2HD4: Disabling display %d\n", display_id);

	retval = it66122_send_command(dev, IT66122_CMD_DISABLE_DISPLAY,
				      cmd_data, 1, response, &response_len);
	if (retval) {
		pr_err("USBC2HD4: Failed to disable display (error %d)\n", retval);
		goto unlock;
	}

	dev->displays[display_id].status &= ~DISPLAY_ENABLED;

unlock:
	mutex_unlock(&dev->lock);
	return retval;
}
