# API Reference

## Kernel Module API

### Data Structures

#### struct display_orientation

```c
enum display_orientation {
    ORIENTATION_LANDSCAPE = 0,  // Standard landscape mode
    ORIENTATION_PORTRAIT = 1,   // Portrait mode (rotated 90°)
};
```

#### struct display_resolution

```c
struct display_resolution {
    uint16_t width;             // Resolution width in pixels
    uint16_t height;            // Resolution height in pixels
    uint8_t refresh_rate;       // Refresh rate in Hz
    uint8_t reserved;           // Reserved for future use
};
```

#### struct display_info

```c
struct display_info {
    uint8_t id;                 // Display index 0-3
    uint8_t status;             // Connection status flags
    enum display_orientation orientation; // Current orientation
    struct display_resolution resolution;  // Current resolution
    uint32_t edid_length;       // EDID data length
    uint8_t edid_data[256];     // EDID raw data (partial)
};
```

Display status flags:
- `DISPLAY_CONNECTED` (0x01) - Display is physically connected
- `DISPLAY_ENABLED` (0x02) - Display output is enabled
- `DISPLAY_HOTPLUG` (0x04) - Hot-plug event pending

#### struct usbc2hd4_device

```c
struct usbc2hd4_device {
    struct usb_device *udev;              // USB device handle
    struct usb_interface *intf;           // USB interface
    struct device *dev;                   // Linux device
    int minor;                            // Character device minor number
    int num_displays;                     // Number of connected displays
    struct display_info displays[4];      // Display info array
    struct urb *irq_urb;                  // Interrupt URB for hot-plug
    unsigned char *irq_buffer;            // Interrupt buffer
    struct workqueue_struct *hotplug_wq;  // Hotplug work queue
    struct work_struct hotplug_work;      // Hotplug work structure
    struct mutex lock;                    // Device state mutex
    bool connected;                       // Connection state
    void *priv;                           // Private driver data
};
```

### Kernel Functions

#### usbc2hd4_enumerate_displays

```c
int usbc2hd4_enumerate_displays(struct usbc2hd4_device *dev);
```

**Description**: Detects and enumerates all connected HDMI displays.

**Parameters**:
- `dev` - Device structure pointer

**Returns**:
- 0 on success
- Negative error code on failure

**Errors**:
- `-ENODEV` - Device not connected
- `-EINVAL` - Invalid device

**Notes**:
- Updates `dev->num_displays` and `dev->displays[]` array
- Blocks while querying IT66122 chip
- Caller must hold device lock

---

#### usbc2hd4_get_display_info

```c
int usbc2hd4_get_display_info(struct usbc2hd4_device *dev, 
                              uint8_t display_id,
                              struct display_info *info);
```

**Description**: Retrieves detailed information about a specific display.

**Parameters**:
- `dev` - Device structure pointer
- `display_id` - Display index (0-3)
- `info` - Output display info structure

**Returns**:
- 0 on success
- Negative error code on failure

**Errors**:
- `-ENODEV` - Device not connected
- `-EINVAL` - Invalid display_id or parameters

---

#### usbc2hd4_set_resolution

```c
int usbc2hd4_set_resolution(struct usbc2hd4_device *dev, 
                            uint8_t display_id,
                            uint16_t width, 
                            uint16_t height, 
                            uint8_t refresh_rate);
```

**Description**: Configures the resolution and refresh rate for a display.

**Parameters**:
- `dev` - Device structure pointer
- `display_id` - Display index (0-3)
- `width` - Resolution width in pixels
- `height` - Resolution height in pixels
- `refresh_rate` - Refresh rate in Hz (60, 75, etc.)

**Returns**:
- 0 on success
- Negative error code on failure

**Example**:
```c
// Set 1920x1080@60Hz on display 0
usbc2hd4_set_resolution(dev, 0, 1920, 1080, 60);
```

---

#### usbc2hd4_set_orientation

```c
int usbc2hd4_set_orientation(struct usbc2hd4_device *dev, 
                             uint8_t display_id,
                             enum display_orientation orientation);
```

**Description**: Sets the display orientation.

**Parameters**:
- `dev` - Device structure pointer
- `display_id` - Display index (0-3)
- `orientation` - ORIENTATION_LANDSCAPE or ORIENTATION_PORTRAIT

**Returns**:
- 0 on success
- Negative error code on failure

---

#### usbc2hd4_enable_display

```c
int usbc2hd4_enable_display(struct usbc2hd4_device *dev, 
                            uint8_t display_id);
```

**Description**: Enables video output on a display.

**Parameters**:
- `dev` - Device structure pointer
- `display_id` - Display index (0-3)

**Returns**:
- 0 on success
- Negative error code on failure

---

#### usbc2hd4_disable_display

```c
int usbc2hd4_disable_display(struct usbc2hd4_device *dev, 
                             uint8_t display_id);
```

**Description**: Disables video output on a display.

**Parameters**:
- `dev` - Device structure pointer
- `display_id` - Display index (0-3)

**Returns**:
- 0 on success
- Negative error code on failure

---

#### it66122_send_command

```c
int it66122_send_command(struct usbc2hd4_device *dev, 
                         uint8_t cmd,
                         const void *data, 
                         size_t data_len,
                         void *response, 
                         size_t *response_len);
```

**Description**: Sends a command to the IT66122 chip via USB control transfer.

**Parameters**:
- `dev` - Device structure pointer
- `cmd` - Command ID (see IT66122 command constants)
- `data` - Command data buffer (NULL if no data)
- `data_len` - Command data length
- `response` - Response buffer (NULL if no response expected)
- `response_len` - Pointer to response length (updated with actual length)

**Returns**:
- 0 on success
- Negative error code on failure

**Maximum Sizes**:
- Command data: 255 bytes
- Response: 256 bytes

---

## User-Space Library API

### Constants

```c
#define USBC2HD4_DEVICE_PATH  "/dev/usbc2hd4"
```

### Data Types

#### usbc2hd4_handle_t

Opaque handle type for device context.

#### usbc2hd4_orientation_t

```c
typedef enum {
    USBC2HD4_ORIENTATION_LANDSCAPE = 0,
    USBC2HD4_ORIENTATION_PORTRAIT = 1,
} usbc2hd4_orientation_t;
```

#### usbc2hd4_display_info_t

```c
typedef struct {
    uint8_t id;                    // Display index 0-3
    int connected;                 // 1 if connected, 0 if not
    int enabled;                   // 1 if enabled, 0 if disabled
    uint16_t width;                // Current resolution width
    uint16_t height;               // Current resolution height
    uint8_t refresh_rate;          // Current refresh rate
    usbc2hd4_orientation_t orientation; // Current orientation
} usbc2hd4_display_info_t;
```

#### usbc2hd4_device_info_t

```c
typedef struct {
    char product_name[256];        // Product name
    char serial_number[256];       // Serial number
    uint16_t vendor_id;            // USB vendor ID
    uint16_t product_id;           // USB product ID
    uint8_t num_displays;          // Number of connected displays
    int major_version;             // Driver major version
    int minor_version;             // Driver minor version
} usbc2hd4_device_info_t;
```

#### usbc2hd4_resolution_t

```c
typedef struct {
    uint16_t width;
    uint16_t height;
    uint8_t refresh_rate;
} usbc2hd4_resolution_t;
```

### Library Functions

#### usbc2hd4_lib_version

```c
const char *usbc2hd4_lib_version(void);
```

**Description**: Returns the user-space library version string.

**Returns**: Version string (e.g., "0.1.0")

---

#### usbc2hd4_open

```c
usbc2hd4_handle_t usbc2hd4_open(void);
```

**Description**: Opens the first available USB-C to HDMI adapter.

**Returns**: 
- Device handle on success
- NULL on failure (check errno)

**Example**:
```c
usbc2hd4_handle_t dev = usbc2hd4_open();
if (!dev) {
    perror("usbc2hd4_open");
    return 1;
}
```

---

#### usbc2hd4_open_by_index

```c
usbc2hd4_handle_t usbc2hd4_open_by_index(int index);
```

**Description**: Opens a specific adapter by index.

**Parameters**:
- `index` - Device index (0 = first device, 1 = second, etc.)

**Returns**: Device handle or NULL on failure

---

#### usbc2hd4_close

```c
void usbc2hd4_close(usbc2hd4_handle_t handle);
```

**Description**: Closes device connection and frees resources.

**Parameters**:
- `handle` - Device handle from usbc2hd4_open()

---

#### usbc2hd4_get_device_info

```c
int usbc2hd4_get_device_info(usbc2hd4_handle_t handle, 
                             usbc2hd4_device_info_t *info);
```

**Description**: Retrieves device information.

**Parameters**:
- `handle` - Device handle
- `info` - Output device info structure

**Returns**:
- 0 on success
- Negative error code on failure

---

#### usbc2hd4_enumerate_displays

```c
int usbc2hd4_enumerate_displays(usbc2hd4_handle_t handle);
```

**Description**: Detects and enumerates all connected displays.

**Parameters**:
- `handle` - Device handle

**Returns**:
- Number of displays found
- Negative error code on failure

---

#### usbc2hd4_get_display_info

```c
int usbc2hd4_get_display_info(usbc2hd4_handle_t handle, 
                              uint8_t display_id,
                              usbc2hd4_display_info_t *info);
```

**Description**: Gets current information about a display.

**Parameters**:
- `handle` - Device handle
- `display_id` - Display index (0-3)
- `info` - Output display info structure

**Returns**:
- 0 on success
- Negative error code on failure

---

#### usbc2hd4_set_resolution

```c
int usbc2hd4_set_resolution(usbc2hd4_handle_t handle, 
                            uint8_t display_id,
                            uint16_t width, 
                            uint16_t height, 
                            uint8_t refresh_rate);
```

**Description**: Sets display resolution and refresh rate.

**Example**:
```c
// Set 2560x1440@60Hz on display 1
int ret = usbc2hd4_set_resolution(dev, 1, 2560, 1440, 60);
if (ret < 0) {
    fprintf(stderr, "Error: %s\n", usbc2hd4_get_error_string(ret));
}
```

---

#### usbc2hd4_set_orientation

```c
int usbc2hd4_set_orientation(usbc2hd4_handle_t handle, 
                             uint8_t display_id,
                             usbc2hd4_orientation_t orientation);
```

**Description**: Sets display orientation.

---

#### usbc2hd4_enable_display

```c
int usbc2hd4_enable_display(usbc2hd4_handle_t handle, 
                            uint8_t display_id);
```

**Description**: Enables video output on a display.

---

#### usbc2hd4_disable_display

```c
int usbc2hd4_disable_display(usbc2hd4_handle_t handle, 
                             uint8_t display_id);
```

**Description**: Disables video output on a display.

---

#### usbc2hd4_get_supported_resolutions

```c
int usbc2hd4_get_supported_resolutions(usbc2hd4_handle_t handle, 
                                      uint8_t display_id,
                                      usbc2hd4_resolution_t *resolutions,
                                      int max_count);
```

**Description**: Retrieves list of supported resolutions for a display.

**Parameters**:
- `handle` - Device handle
- `display_id` - Display index (0-3)
- `resolutions` - Output resolution array
- `max_count` - Maximum number of resolutions to return

**Returns**:
- Number of resolutions returned
- Negative error code on failure

---

#### usbc2hd4_get_error_string

```c
const char *usbc2hd4_get_error_string(int error_code);
```

**Description**: Converts error code to human-readable string.

**Parameters**:
- `error_code` - Error code from other library functions

**Returns**: Error description string (e.g., "Invalid argument", "Device not found")

---

## Common Error Codes

| Code | Name | Description |
|------|------|-------------|
| 0 | Success | Operation completed successfully |
| -1 | EPERM | Operation not permitted |
| -2 | ENOENT | No such file or directory |
| -5 | EIO | Input/output error |
| -12 | ENOMEM | Out of memory |
| -14 | EFAULT | Bad address |
| -16 | EBUSY | Device or resource busy |
| -19 | ENODEV | No such device |
| -22 | EINVAL | Invalid argument |
| -28 | ENOSPC | No space left on device |
| -61 | ENODATA | No data available |

---

## Example Programs

### Kernel Module Example

```c
static int usbc2hd4_probe(struct usb_interface *intf, 
                          const struct usb_device_id *id)
{
    struct usbc2hd4_device *dev;
    int retval;

    // Allocate and initialize device
    dev = kzalloc(sizeof(*dev), GFP_KERNEL);
    if (!dev)
        return -ENOMEM;

    dev->udev = usb_get_dev(interface_to_usbdev(intf));
    dev->intf = intf;
    mutex_init(&dev->lock);

    // Enumerate displays
    retval = usbc2hd4_enumerate_displays(dev);
    if (retval)
        pr_err("Failed to enumerate displays\n");

    // Setup hot-plug detection
    retval = usbc2hd4_setup_hotplug(dev);
    if (retval)
        pr_warn("Hot-plug detection not available\n");

    return 0;
}
```

### User-Space Library Example

See [examples/](../tools/examples/) directory for sample programs.
