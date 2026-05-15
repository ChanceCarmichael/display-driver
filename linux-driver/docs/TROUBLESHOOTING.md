# Troubleshooting Guide

## Common Issues and Solutions

### Build Issues

#### Error: "Kernel headers not found"

**Cause**: Linux kernel headers for your version are not installed.

**Solution**:
```bash
# Find your kernel version
uname -r

# Install headers
sudo apt-get update
sudo apt-get install linux-headers-$(uname -r)

# Verify
dpkg -l | grep linux-headers
```

#### Error: "make: command not found"

**Cause**: Build tools are not installed.

**Solution**:
```bash
sudo apt-get update
sudo apt-get install build-essential cmake
```

#### Error: "CMake not found"

**Cause**: CMake is not installed.

**Solution**:
```bash
sudo apt-get install cmake
cmake --version  # Verify (should be 3.10+)
```

#### Compilation warnings or errors

**Solution**:
1. Clean build: `make clean`
2. Check for incomplete headers: `make check`
3. Review full output: `make 2>&1 | tee build.log`

---

### Installation Issues

#### Error: "Permission denied" when installing

**Cause**: Installation requires root privileges.

**Solution**:
```bash
# Use sudo for installation
sudo bash build/install.sh --all

# Or make with sudo
sudo make install
```

#### Error: "Cannot install to /usr/local"

**Cause**: Permission issue with installation directory.

**Solution**:
```bash
# Create build directory in writable location
mkdir ~/usbc2hd4_build
cd ~/usbc2hd4_build

# Build with custom install prefix
cmake -DCMAKE_INSTALL_PREFIX=$HOME/.local ..
make
sudo make install  # May not need sudo for user directory

# Add to library path
export LD_LIBRARY_PATH=$HOME/.local/lib:$LD_LIBRARY_PATH
export PKG_CONFIG_PATH=$HOME/.local/lib/pkgconfig:$PKG_CONFIG_PATH
```

---

### Module Loading Issues

#### Error: "Failed to load module" or "Operation not permitted"

**Cause**: Module not found or signature mismatch.

**Solution**:
```bash
# Rebuild for current kernel
make clean
make

# Check if kernel version matches
file kernel/usbc2hd4/usbc2hd4.ko | grep $(uname -r)

# Try manual insmod
sudo insmod kernel/usbc2hd4/usbc2hd4.ko

# Check errors
dmesg | tail -20
```

#### Error: "Cannot find a matching Kernel version"

**Cause**: Module was built for different kernel version.

**Solution**:
```bash
# Check kernel version
uname -r

# Check what module was built for
modinfo kernel/usbc2hd4/usbc2hd4.ko | grep vermagic

# Rebuild for current kernel
make clean
make
sudo make install
```

#### Module loaded but no device appears

**Cause**: Device not found or not connected.

**Solution**:
```bash
# Verify module is loaded
lsmod | grep usbc2hd4

# Check kernel messages
dmesg | grep -i usbc2hd4

# Check USB connection
lsusb | grep 0bda   # Look for vendor ID 0bda

# Check device files
ls -la /dev/usbc2hd4*
```

---

### Device Connection Issues

#### USB device not detected

**Cause**: Device not connected or USB issue.

**Solution**:
```bash
# Check USB connection
lsusb

# Look for USB-C adapter
lsusb -v | grep -A 5 "USB-C\|HDMI\|Realtek"

# Check USB port
sudo usb-devices | grep -i "usbc2hd4\|0bda:8153"

# Try different USB port
# 1. Unplug adapter
# 2. Wait 2 seconds
# 3. Plug into different USB 3.0 port (if available)

# Try different USB cable
```

#### "Device connected but no displays enumerated"

**Cause**: No HDMI cables connected or display issue.

**Solution**:
```bash
# Verify displays connected to adapter HDMI ports
# Ensure displays are powered on

# Enumerate displays
ls -la /sys/class/usbc2hd4/

# Try enabling displays
# (May require user-space tool once developed)

# Check dmesg for errors
dmesg | grep -i "display\|hdmi\|hotplug"
```

---

### Runtime Issues

#### Display configuration changes not working

**Cause**: Command sent but not accepted by device.

**Solution**:
1. Verify display is connected: Check LED indicators on adapter
2. Try enabling display first: `usbc2hd4_enable_display()`
3. Then set resolution: `usbc2hd4_set_resolution()`
4. Check for errors in application
5. Review kernel logs: `dmesg | grep USBC2HD4`

#### High CPU usage or system hangs

**Cause**: Driver polling or synchronization issue.

**Solution**:
```bash
# Identify process using CPU
top -p $(pidof <app_name>)

# Unload driver and try again
sudo modprobe -r usbc2hd4

# Rebuild with debug info
make debug

# Capture trace
sudo strace -o trace.log -p <pid>
```

#### Library functions return errors

**Cause**: Device not properly initialized or not connected.

**Solution**:
```c
// Check error codes
int ret = usbc2hd4_set_resolution(dev, 0, 1920, 1080, 60);
if (ret < 0) {
    fprintf(stderr, "Error: %s\n", usbc2hd4_get_error_string(ret));
}

// Typical error codes:
// -ENODEV: Device not connected or driver not loaded
// -EINVAL: Invalid parameters
// -EBADF: Device file not properly opened
```

---

### Uninstall Issues

#### Can't uninstall module

**Cause**: Module is still in use.

**Solution**:
```bash
# Unload module first
sudo modprobe -r usbc2hd4

# Kill any processes using the module
ps aux | grep usbc2hd4

# Try manual unload
sudo rmmod usbc2hd4

# Clean up manually
sudo rm -f /lib/modules/*/extra/usbc2hd4.ko
sudo depmod -a
```

---

## Debugging Tips

### Enable Debug Output

```bash
# Rebuild with debug symbols
make debug

# View kernel logs in real-time
sudo journalctl -f -k

# Or use dmesg
watch 'dmesg | tail -20'
```

### Check Module Status

```bash
# Complete module information
modinfo usbc2hd4

# Module dependencies
modinfo -d usbc2hd4

# Module parameters
cat /sys/module/usbc2hd4/parameters/*
```

### USB Debugging

```bash
# View all USB devices
lsusb -tree

# Detailed device info
lsusb -v -d 0bda:8153

# Monitor USB traffic (requires usbmon)
sudo modprobe usbmon
sudo cat /sys/kernel/debug/usb/usbmon/1u | grep 0bda

# USB statistics
cat /proc/bus/usb/devices | grep -A 10 "0bda"
```

### Performance Analysis

```bash
# Check module memory usage
cat /proc/modules | grep usbc2hd4

# Monitor interrupt activity
watch -n1 'grep usbc2hd4 /proc/interrupts'

# Check workqueue status
cat /proc/workqueue
```

---

## Getting Help

If issues persist:

1. **Collect diagnostic information**:
   ```bash
   uname -a
   lsb_release -a
   dmesg | grep USBC2HD4 > debug.log
   lsusb -v >> debug.log
   ```

2. **Check documentation**:
   - [README.md](README.md) - Getting started
   - [API.md](API.md) - Function reference
   - [BUILD.md](BUILD.md) - Build details

3. **Review kernel messages**:
   ```bash
   dmesg | grep -i "usbc2hd4\|hdmi\|it66122" | tail -50
   ```

4. **Test with examples**:
   - Compile examples: `gcc -o enumerate tools/examples/enumerate_example.c -lusbc2hd4`
   - Run: `./enumerate`

5. **Contact support** with:
   - Kernel version: `uname -r`
   - Device serial (from lsusb)
   - Full error messages from dmesg
   - Build log if compilation failed

