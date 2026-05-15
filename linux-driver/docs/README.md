# USBC2HD4 Linux Kernel Driver

## Overview

This is a Linux kernel driver for the StarTech USB-C to Quad HDMI Display Adapter (USBC2HD4). The device uses a Trigger T6-688SL chipset with an IT66122 USB to HDMI bridge controller to provide up to 4 independent HDMI displays over a single USB-C connection.

### Device Specifications

- **Vendor ID**: 0x0BDA (Realtek)
- **Product ID**: 0x8153 (Trigger T6-688SL)
- **Maximum Displays**: 4 HDMI outputs
- **Connection**: USB-C
- **Bridge Controller**: IT66122

## Architecture

### Kernel Module

The kernel driver (`usbc2hd4.ko`) provides:

- USB device detection and probe
- Character device interface (`/dev/usbc2hd4*`)
- Display enumeration and status monitoring
- Resolution and orientation configuration
- Hot-plug event detection and handling
- IT66122 chip communication protocol

### User-Space Library

The user-space library (`libusbc2hd4.so`) provides:

- C API for display management
- Device enumeration
- Resolution configuration
- Display enable/disable
- Error handling and reporting

## Building

### Kernel Module

#### Prerequisites

- Linux kernel headers: `sudo apt-get install linux-headers-$(uname -r)`
- Build tools: `sudo apt-get install build-essential`

#### Build and Install

```bash
cd kernel/usbc2hd4
make check              # Verify kernel headers
make                    # Build the module
sudo make install       # Install the module
sudo depmod -a          # Update module dependencies
```

#### Loading the Module

```bash
# Manual load
sudo insmod usbc2hd4.ko

# Or via modprobe (after install)
sudo modprobe usbc2hd4

# Verify loaded
lsmod | grep usbc2hd4
dmesg | tail -20        # View kernel messages
```

#### Unloading

```bash
sudo modprobe -r usbc2hd4
```

### User-Space Library

#### Prerequisites

- CMake 3.10+: `sudo apt-get install cmake`
- Build tools: `sudo apt-get install build-essential`

#### Build and Install

```bash
mkdir build
cd build
cmake ..
make
sudo make install
```

This will:
- Build shared library: `lib/libusbc2hd4.so`
- Build static library: `lib/libusbc2hd4.a`
- Install headers to `/usr/local/include/usbc2hd4_lib.h`
- Install pkg-config file for easy linking

#### Using the Library

In your C code:

```c
#include <usbc2hd4_lib.h>

// Open device
usbc2hd4_handle_t dev = usbc2hd4_open();
if (!dev) {
    perror("Failed to open device");
    return 1;
}

// Enumerate displays
int num = usbc2hd4_enumerate_displays(dev);
printf("Found %d displays\n", num);

// Set resolution on first display
usbc2hd4_set_resolution(dev, 0, 1920, 1080, 60);

// Close device
usbc2hd4_close(dev);
```

Compile with:

```bash
gcc -o myapp myapp.c $(pkg-config --cflags --libs usbc2hd4)
# or
gcc -o myapp myapp.c -I/usr/local/include -L/usr/local/lib -lusbc2hd4
```

## Device Files

After loading the module, device files are created:

```
/dev/usbc2hd4           # First device
/dev/usbc2hd4_0         # Device 0 (primary)
/dev/usbc2hd4_1         # Device 1 (if multiple adapters)
/dev/usbc2hd4_2         # etc.
```

Check current displays:

```bash
cat /sys/class/usbc2hd4/usbc2hd4_0/displays
```

## Source Files

### Kernel Module

| File | Purpose |
|------|---------|
| `usbc2hd4.h` | Header file with data structures and function prototypes |
| `main.c` | Module initialization, module parameters, device class setup |
| `usb_probe.c` | USB device detection, probe and disconnect callbacks |
| `it66122.c` | IT66122 chip communication protocol implementation |
| `display_enum.c` | Display enumeration, info retrieval, resolution/orientation configuration |
| `hotplug.c` | Hot-plug event detection via interrupt URB |
| `Makefile` | Kernel module build configuration |

### User-Space Library

| File | Purpose |
|------|---------|
| `include/usbc2hd4_lib.h` | Public API header |
| `src/usbc2hd4_lib.c` | Library implementation |
| `CMakeLists.txt` | CMake build configuration |
| `usbc2hd4.pc.in` | pkg-config template |

## Kernel Module Functions

### Core Functions

- `usbc2hd4_probe()` - USB device probe callback
- `usbc2hd4_disconnect()` - USB device disconnect callback
- `usbc2hd4_enumerate_displays()` - Detect all connected displays
- `usbc2hd4_get_display_info()` - Get display status and capabilities
- `usbc2hd4_set_resolution()` - Configure display resolution
- `usbc2hd4_set_orientation()` - Set display orientation
- `usbc2hd4_enable_display()` - Enable display output
- `usbc2hd4_disable_display()` - Disable display output
- `it66122_send_command()` - IT66122 chip command protocol

### Hot-Plug Support

- `usbc2hd4_setup_hotplug()` - Initialize interrupt URB monitoring
- `usbc2hd4_shutdown_hotplug()` - Cleanup hot-plug resources
- `usbc2hd4_hotplug_work()` - Handle display connection changes

## IT66122 Command Protocol

The IT66122 chip uses a USB control transfer protocol:

| Command | Value | Description |
|---------|-------|-------------|
| GET_STATUS | 0x01 | Get chip status |
| SET_MODE | 0x02 | Set operating mode |
| GET_DISPLAY_INFO | 0x03 | Query connected display info |
| SET_RESOLUTION | 0x04 | Configure resolution/refresh rate |
| SET_ORIENTATION | 0x05 | Set display orientation |
| ENABLE_DISPLAY | 0x06 | Enable display output |
| DISABLE_DISPLAY | 0x07 | Disable display output |
| ENUMERATE_DISPLAYS | 0x08 | Detect all connected displays |

## Debugging

### Enable Debug Output

```bash
# Rebuild kernel module with debug symbols
cd kernel/usbc2hd4
make debug
sudo make install
sudo modprobe usbc2hd4
```

### View Kernel Messages

```bash
# Real-time kernel log
sudo journalctl -f

# Or use dmesg
dmesg | grep USBC2HD4
```

### USB Debugging

```bash
# Monitor USB traffic
sudo lsusb -v

# Check device details
sudo lsusb -d 0bda:8153 -v
```

## Troubleshooting

### Module Won't Load

```bash
# Verify kernel headers installed
dpkg -l | grep linux-headers

# Check kernel version
uname -r

# Install correct headers
sudo apt-get install linux-headers-$(uname -r)
```

### Device Not Detected

```bash
# Check USB connection
lsusb | grep 0bda

# Verify module loaded
lsmod | grep usbc2hd4

# Check device files
ls -la /dev/usbc2hd4*
```

### No Displays Found

1. Verify USB-C cable connection
2. Check power supply to adapter
3. Ensure HDMI cables are connected
4. Try hot-plugging displays
5. Check kernel logs for errors

## Performance Notes

- Display enumeration is performed on module load and after hot-plug events
- Resolution changes are atomic on supported modes
- Maximum USB bandwidth is shared among all displays
- Hot-plug detection may take up to 1 second

## Future Enhancements

- EDID parsing for automatic resolution detection
- Display mirroring support
- Color depth configuration (8-bit, 10-bit, 12-bit)
- HDCP support
- Audio passthrough configuration
- Sysfs interface for display management
- udev rules for automatic permissions

## License

This driver is released under the GPL v2 license. See COPYING file for details.

## Support

For issues, questions, or contributions:
1. Check the troubleshooting section above
2. Review kernel logs for error messages
3. Verify hardware compatibility
4. Contact StarTech technical support with device serial number

## Related Documentation

- [IT66122 Datasheet](../usbc2hd4_datasheet.pdf)
- [Linux Kernel Driver Development](https://www.kernel.org/doc/html/latest/driver-api/)
- [USB Device Drivers](https://www.kernel.org/doc/html/latest/driver-api/usb/)
