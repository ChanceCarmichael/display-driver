/*
 * usbc2hd4_lib.c - User-space Library Implementation
 * 
 * Implements user-space API for USB-C to HDMI adapter control
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include "usbc2hd4_lib.h"

#define USBC2HD4_IOC_MAGIC 'U'

struct _k_device_info {
	uint16_t vendor_id;
	uint16_t product_id;
	uint8_t num_displays;
	uint8_t reserved[3];
	char product_name[32];
	char serial_number[32];
};

struct _k_display_info {
	uint8_t display_id;
	uint8_t connected;
	uint8_t enabled;
	uint8_t reserved;
	uint16_t width;
	uint16_t height;
	uint8_t refresh_rate;
	uint8_t orientation;
};

#define USBC2HD4_IOCTL_GET_DEVICE_INFO _IOR(USBC2HD4_IOC_MAGIC, 1, struct _k_device_info)
#define USBC2HD4_IOCTL_ENUMERATE_DISPLAYS _IO(USBC2HD4_IOC_MAGIC, 2)
#define USBC2HD4_IOCTL_GET_DISPLAY_INFO _IOR(USBC2HD4_IOC_MAGIC, 3, struct _k_display_info)
#define USBC2HD4_IOCTL_SET_RESOLUTION _IOW(USBC2HD4_IOC_MAGIC, 4, struct _k_display_info)
#define USBC2HD4_IOCTL_ENABLE_DISPLAY _IOW(USBC2HD4_IOC_MAGIC, 5, uint8_t)
#define USBC2HD4_IOCTL_DISABLE_DISPLAY _IOW(USBC2HD4_IOC_MAGIC, 6, uint8_t)

#define USBC2HD4_DEVICE_PATH "/dev/usbc2hd4"
#define LIBRARY_VERSION "0.1.0"

/* Device context */
typedef struct {
	int fd;                         /* Device file descriptor */
	usbc2hd4_device_info_t dev_info; /* Cached device info */
	usbc2hd4_display_info_t displays[4]; /* Cached display info */
	int num_displays;
} usbc2hd4_context_t;

/**
 * usbc2hd4_lib_version - Get library version string
 */
const char *usbc2hd4_lib_version(void)
{
	return LIBRARY_VERSION;
}

/**
 * usbc2hd4_open - Open connection to first available device
 */
usbc2hd4_handle_t usbc2hd4_open(void)
{
	return usbc2hd4_open_by_index(0);
}

/**
 * usbc2hd4_open_by_index - Open device by index
 */
usbc2hd4_handle_t usbc2hd4_open_by_index(int index)
{
	usbc2hd4_context_t *ctx;
	char device_path[64];
	int fd;

	/* Allocate context */
	ctx = (usbc2hd4_context_t *)malloc(sizeof(*ctx));
	if (!ctx) {
		perror("malloc");
		return NULL;
	}

	memset(ctx, 0, sizeof(*ctx));

	/* Open device file */
	snprintf(device_path, sizeof(device_path), "%s%d", 
		 USBC2HD4_DEVICE_PATH, index);

	fd = open(device_path, O_RDWR);
	if (fd < 0) {
		fprintf(stderr, "Failed to open device %s: %s\n",
			device_path, strerror(errno));
		free(ctx);
		return NULL;
	}

	ctx->fd = fd;

	/* Get device info (populate cached info) */
	if (usbc2hd4_get_device_info((usbc2hd4_handle_t)ctx, &ctx->dev_info) < 0) {
		/* Non-fatal: leave open but with empty cached info */
		ctx->num_displays = 0;
	}

	return (usbc2hd4_handle_t)ctx;
}

/**
 * usbc2hd4_close - Close device connection
 */
void usbc2hd4_close(usbc2hd4_handle_t handle)
{
	usbc2hd4_context_t *ctx = (usbc2hd4_context_t *)handle;

	if (!ctx) {
		return;
	}

	if (ctx->fd >= 0) {
		close(ctx->fd);
	}

	free(ctx);
}

/**
 * usbc2hd4_get_device_info - Get device information
 */
int usbc2hd4_get_device_info(usbc2hd4_handle_t handle, 
			     usbc2hd4_device_info_t *info)
{
	usbc2hd4_context_t *ctx = (usbc2hd4_context_t *)handle;

	struct _k_device_info kinfo;
	int ret;

	if (!ctx || !info)
		return -EINVAL;

	if (ctx->fd < 0)
		return -EBADF;

	ret = ioctl(ctx->fd, USBC2HD4_IOCTL_GET_DEVICE_INFO, &kinfo);
	if (ret == -1) {
		return -errno;
	}

	/* Map kernel struct to library struct */
	memset(info, 0, sizeof(*info));
	strncpy(info->product_name, kinfo.product_name, sizeof(info->product_name)-1);
	strncpy(info->serial_number, kinfo.serial_number, sizeof(info->serial_number)-1);
	info->vendor_id = kinfo.vendor_id;
	info->product_id = kinfo.product_id;
	info->num_displays = kinfo.num_displays;

	/* Cache into context */
	ctx->dev_info = *info;
	ctx->num_displays = kinfo.num_displays;

	return 0;
}

/**
 * usbc2hd4_enumerate_displays - Enumerate connected displays
 */
int usbc2hd4_enumerate_displays(usbc2hd4_handle_t handle)
{
	usbc2hd4_context_t *ctx = (usbc2hd4_context_t *)handle;
	int ret;

	if (!ctx)
		return -EINVAL;

	if (ctx->fd < 0)
		return -EBADF;

	ret = ioctl(ctx->fd, USBC2HD4_IOCTL_ENUMERATE_DISPLAYS);
	if (ret == -1)
		return -errno;

	/* ioctl returns number of displays */
	ctx->num_displays = ret;
	return ret;
}

/**
 * usbc2hd4_get_display_info - Get information about a display
 */
int usbc2hd4_get_display_info(usbc2hd4_handle_t handle, uint8_t display_id,
			      usbc2hd4_display_info_t *info)
{
	usbc2hd4_context_t *ctx = (usbc2hd4_context_t *)handle;

	struct _k_display_info kdisp;
	int ret;

	if (!ctx || !info)
		return -EINVAL;

	if (display_id >= 4)
		return -EINVAL;

	if (ctx->fd < 0)
		return -EBADF;

	memset(&kdisp, 0, sizeof(kdisp));
	kdisp.display_id = display_id;

	ret = ioctl(ctx->fd, USBC2HD4_IOCTL_GET_DISPLAY_INFO, &kdisp);
	if (ret == -1)
		return -errno;

	/* Map kernel struct to library struct */
	memset(info, 0, sizeof(*info));
	info->id = kdisp.display_id;
	info->connected = kdisp.connected;
	info->enabled = kdisp.enabled;
	info->width = kdisp.width;
	info->height = kdisp.height;
	info->refresh_rate = kdisp.refresh_rate;
	info->orientation = (kdisp.orientation == 0) ? USBC2HD4_ORIENTATION_LANDSCAPE : USBC2HD4_ORIENTATION_PORTRAIT;

	/* Cache */
	ctx->displays[display_id] = *info;

	return 0;
}

/**
 * usbc2hd4_set_resolution - Set display resolution
 */
int usbc2hd4_set_resolution(usbc2hd4_handle_t handle, uint8_t display_id,
			    uint16_t width, uint16_t height, 
			    uint8_t refresh_rate)
{
	usbc2hd4_context_t *ctx = (usbc2hd4_context_t *)handle;

	struct _k_display_info cmd;
	int ret;

	if (!ctx)
		return -EINVAL;

	if (display_id >= 4 || ctx->fd < 0)
		return -EINVAL;

	memset(&cmd, 0, sizeof(cmd));
	cmd.display_id = display_id;
	cmd.width = width;
	cmd.height = height;
	cmd.refresh_rate = refresh_rate;

	ret = ioctl(ctx->fd, USBC2HD4_IOCTL_SET_RESOLUTION, &cmd);
	if (ret == -1)
		return -errno;

	/* Update cache */
	ctx->displays[display_id].width = width;
	ctx->displays[display_id].height = height;
	ctx->displays[display_id].refresh_rate = refresh_rate;

	return 0;
}

/**
 * usbc2hd4_set_orientation - Set display orientation
 */
int usbc2hd4_set_orientation(usbc2hd4_handle_t handle, uint8_t display_id,
			     usbc2hd4_orientation_t orientation)
{
	usbc2hd4_context_t *ctx = (usbc2hd4_context_t *)handle;

	if (!ctx) {
		return -EINVAL;
	}

	if (display_id >= 4 || ctx->fd < 0) {
		return -EINVAL;
	}

	/* TODO: Implement ioctl to set orientation */
	printf("Setting display %d orientation to %d\n",
	       display_id, orientation);

	return 0;
}

/**
 * usbc2hd4_enable_display - Enable a display output
 */
int usbc2hd4_enable_display(usbc2hd4_handle_t handle, uint8_t display_id)
{
	usbc2hd4_context_t *ctx = (usbc2hd4_context_t *)handle;

	int ret;

	if (!ctx)
		return -EINVAL;

	if (display_id >= 4 || ctx->fd < 0)
		return -EINVAL;

	ret = ioctl(ctx->fd, USBC2HD4_IOCTL_ENABLE_DISPLAY, &display_id);
	if (ret == -1)
		return -errno;

	ctx->displays[display_id].enabled = 1;
	return 0;
}

/**
 * usbc2hd4_disable_display - Disable a display output
 */
int usbc2hd4_disable_display(usbc2hd4_handle_t handle, uint8_t display_id)
{
	usbc2hd4_context_t *ctx = (usbc2hd4_context_t *)handle;

	int ret;

	if (!ctx)
		return -EINVAL;

	if (display_id >= 4 || ctx->fd < 0)
		return -EINVAL;

	ret = ioctl(ctx->fd, USBC2HD4_IOCTL_DISABLE_DISPLAY, &display_id);
	if (ret == -1)
		return -errno;

	ctx->displays[display_id].enabled = 0;
	return 0;
}

/**
 * usbc2hd4_get_supported_resolutions - Get supported resolutions for display
 */
int usbc2hd4_get_supported_resolutions(usbc2hd4_handle_t handle, 
				      uint8_t display_id,
				      usbc2hd4_resolution_t *resolutions,
				      int max_count)
{
	(void)max_count;  /* unused parameter */
	usbc2hd4_context_t *ctx = (usbc2hd4_context_t *)handle;
	// int count;

	if (!ctx || !resolutions) {
		return -EINVAL;
	}

	if (display_id >= 4 || ctx->fd < 0) {
		return -EINVAL;
	}

	/* TODO: Implement ioctl to get supported resolutions */
	/* For now, return empty list */
	return 0;
}

/**
 * usbc2hd4_get_error_string - Get error description string
 */
const char *usbc2hd4_get_error_string(int error_code)
{
	switch (error_code) {
	case 0:
		return "Success";
	case -EINVAL:
		return "Invalid argument";
	case -EBADF:
		return "Bad file descriptor";
	case -ENODEV:
		return "Device not found";
	case -ENOMEM:
		return "Out of memory";
	case -EACCES:
		return "Permission denied";
	default:
		return strerror(-error_code);
	}
}
