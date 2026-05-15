/*
 * usbc2hd4_lib.h - User-space Library for USB-C to HDMI Adapter
 * 
 * StarTech.com USBC2HD4 device library for Linux
 * Provides C API for display management and configuration
 */

#ifndef _USBC2HD4_LIB_H_
#define _USBC2HD4_LIB_H_

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Handle type for device context */
typedef void *usbc2hd4_handle_t;

/* Display Orientation */
typedef enum {
	USBC2HD4_ORIENTATION_LANDSCAPE = 0,
	USBC2HD4_ORIENTATION_PORTRAIT = 1,
} usbc2hd4_orientation_t;

/* Display Connection Status */
typedef struct {
	uint8_t id;                     /* Display index 0-3 */
	int connected;                  /* 1 if connected, 0 if not */
	int enabled;                    /* 1 if enabled, 0 if disabled */
	uint16_t width;                 /* Current resolution width */
	uint16_t height;                /* Current resolution height */
	uint8_t refresh_rate;           /* Current refresh rate */
	usbc2hd4_orientation_t orientation; /* Current orientation */
} usbc2hd4_display_info_t;

/* Device Information */
typedef struct {
	char product_name[256];         /* Device product name */
	char serial_number[256];        /* Device serial number */
	uint16_t vendor_id;             /* USB vendor ID */
	uint16_t product_id;            /* USB product ID */
	uint8_t num_displays;           /* Number of connected displays */
	int major_version;              /* Driver major version */
	int minor_version;              /* Driver minor version */
} usbc2hd4_device_info_t;

/* Resolution entry */
typedef struct {
	uint16_t width;
	uint16_t height;
	uint8_t refresh_rate;
} usbc2hd4_resolution_t;

/* Function Prototypes */

/**
 * usbc2hd4_lib_version - Get library version string
 * 
 * Returns the version of the user-space library
 */
const char *usbc2hd4_lib_version(void);

/**
 * usbc2hd4_open - Open connection to first available device
 * 
 * Opens the first USB-C to HDMI adapter device found
 * Returns: Device handle on success, NULL on failure
 */
usbc2hd4_handle_t usbc2hd4_open(void);

/**
 * usbc2hd4_open_by_index - Open device by index
 * @index: Device index (0 for first device)
 * 
 * Opens a specific USB-C to HDMI adapter by index
 * Returns: Device handle on success, NULL on failure
 */
usbc2hd4_handle_t usbc2hd4_open_by_index(int index);

/**
 * usbc2hd4_close - Close device connection
 * @handle: Device handle from usbc2hd4_open()
 * 
 * Closes the connection to the device and frees resources
 */
void usbc2hd4_close(usbc2hd4_handle_t handle);

/**
 * usbc2hd4_get_device_info - Get device information
 * @handle: Device handle
 * @info: Output device info structure
 * 
 * Retrieves device information including product name, serial number, etc.
 * Returns: 0 on success, negative error code on failure
 */
int usbc2hd4_get_device_info(usbc2hd4_handle_t handle, 
			     usbc2hd4_device_info_t *info);

/**
 * usbc2hd4_enumerate_displays - Enumerate connected displays
 * @handle: Device handle
 * 
 * Queries device and updates internal display list
 * Returns: Number of displays found, negative error code on failure
 */
int usbc2hd4_enumerate_displays(usbc2hd4_handle_t handle);

/**
 * usbc2hd4_get_display_info - Get information about a display
 * @handle: Device handle
 * @display_id: Display index (0-3)
 * @info: Output display info structure
 * 
 * Retrieves current information about a display
 * Returns: 0 on success, negative error code on failure
 */
int usbc2hd4_get_display_info(usbc2hd4_handle_t handle, uint8_t display_id,
			      usbc2hd4_display_info_t *info);

/**
 * usbc2hd4_set_resolution - Set display resolution
 * @handle: Device handle
 * @display_id: Display index (0-3)
 * @width: Resolution width in pixels
 * @height: Resolution height in pixels
 * @refresh_rate: Refresh rate in Hz
 * 
 * Sets the resolution and refresh rate for a display
 * Returns: 0 on success, negative error code on failure
 */
int usbc2hd4_set_resolution(usbc2hd4_handle_t handle, uint8_t display_id,
			    uint16_t width, uint16_t height, 
			    uint8_t refresh_rate);

/**
 * usbc2hd4_set_orientation - Set display orientation
 * @handle: Device handle
 * @display_id: Display index (0-3)
 * @orientation: Orientation (LANDSCAPE or PORTRAIT)
 * 
 * Sets the display orientation
 * Returns: 0 on success, negative error code on failure
 */
int usbc2hd4_set_orientation(usbc2hd4_handle_t handle, uint8_t display_id,
			     usbc2hd4_orientation_t orientation);

/**
 * usbc2hd4_enable_display - Enable a display output
 * @handle: Device handle
 * @display_id: Display index (0-3)
 * 
 * Enables video output on a display
 * Returns: 0 on success, negative error code on failure
 */
int usbc2hd4_enable_display(usbc2hd4_handle_t handle, uint8_t display_id);

/**
 * usbc2hd4_disable_display - Disable a display output
 * @handle: Device handle
 * @display_id: Display index (0-3)
 * 
 * Disables video output on a display
 * Returns: 0 on success, negative error code on failure
 */
int usbc2hd4_disable_display(usbc2hd4_handle_t handle, uint8_t display_id);

/**
 * usbc2hd4_get_supported_resolutions - Get supported resolutions for display
 * @handle: Device handle
 * @display_id: Display index (0-3)
 * @resolutions: Output array of supported resolutions
 * @max_count: Maximum number of resolutions to return
 * 
 * Retrieves list of supported resolutions for a display
 * Returns: Number of resolutions returned, negative error code on failure
 */
int usbc2hd4_get_supported_resolutions(usbc2hd4_handle_t handle, 
				      uint8_t display_id,
				      usbc2hd4_resolution_t *resolutions,
				      int max_count);

/**
 * usbc2hd4_get_error_string - Get error description string
 * @error_code: Error code from other functions
 * 
 * Converts error code to human-readable string
 * Returns: Error description string
 */
const char *usbc2hd4_get_error_string(int error_code);

#ifdef __cplusplus
}
#endif

#endif /* _USBC2HD4_LIB_H_ */
