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

	/* Get device info */
	if (usbc2hd4_get_device_info((usbc2hd4_handle_t)ctx, &ctx->dev_info) < 0) {
		fprintf(stderr, "Failed to get device info\n");
		close(fd);
		free(ctx);
		return NULL;
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

	if (!ctx || !info) {
		return -EINVAL;
	}

	if (ctx->fd < 0) {
		return -EBADF;
	}

	/* TODO: Implement ioctl to get device info from kernel driver */
	/* For now, return cached info */
	*info = ctx->dev_info;

	return 0;
}

/**
 * usbc2hd4_enumerate_displays - Enumerate connected displays
 */
int usbc2hd4_enumerate_displays(usbc2hd4_handle_t handle)
{
	usbc2hd4_context_t *ctx = (usbc2hd4_context_t *)handle;
	// int i;

	if (!ctx) {
		return -EINVAL;
	}

	if (ctx->fd < 0) {
		return -EBADF;
	}

	/* TODO: Implement ioctl to enumerate displays */
	/* For now, return cached display count */
	return ctx->num_displays;
}

/**
 * usbc2hd4_get_display_info - Get information about a display
 */
int usbc2hd4_get_display_info(usbc2hd4_handle_t handle, uint8_t display_id,
			      usbc2hd4_display_info_t *info)
{
	usbc2hd4_context_t *ctx = (usbc2hd4_context_t *)handle;

	if (!ctx || !info) {
		return -EINVAL;
	}

	if (display_id >= 4) {
		return -EINVAL;
	}

	if (ctx->fd < 0) {
		return -EBADF;
	}

	/* TODO: Implement ioctl to get display info */
	/* For now, return cached display info */
	*info = ctx->displays[display_id];

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

	if (!ctx) {
		return -EINVAL;
	}

	if (display_id >= 4 || ctx->fd < 0) {
		return -EINVAL;
	}

	/* TODO: Implement ioctl to set resolution */
	printf("Setting display %d to %ux%u@%uHz\n",
	       display_id, width, height, refresh_rate);

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

	if (!ctx) {
		return -EINVAL;
	}

	if (display_id >= 4 || ctx->fd < 0) {
		return -EINVAL;
	}

	/* TODO: Implement ioctl to enable display */
	printf("Enabling display %d\n", display_id);

	return 0;
}

/**
 * usbc2hd4_disable_display - Disable a display output
 */
int usbc2hd4_disable_display(usbc2hd4_handle_t handle, uint8_t display_id)
{
	usbc2hd4_context_t *ctx = (usbc2hd4_context_t *)handle;

	if (!ctx) {
		return -EINVAL;
	}

	if (display_id >= 4 || ctx->fd < 0) {
		return -EINVAL;
	}

	/* TODO: Implement ioctl to disable display */
	printf("Disabling display %d\n", display_id);

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
