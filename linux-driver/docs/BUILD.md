# USBC2HD4 Kernel Module Compilation Instructions

## Prerequisites

Before building, ensure your system has:

1. **Kernel Headers**
   ```bash
   sudo apt-get update
   sudo apt-get install linux-headers-$(uname -r)
   ```

2. **Build Tools**
   ```bash
   sudo apt-get install build-essential
   ```

3. **CMake** (for user-space library)
   ```bash
   sudo apt-get install cmake
   ```

## Building the Kernel Module

### Method 1: Using Makefile (Recommended for kernel module only)

```bash
cd kernel/usbc2hd4
make                    # Build
sudo make install       # Install
sudo depmod -a          # Update module database
```

### Method 2: Using CMake (for entire project)

```bash
mkdir build
cd build
cmake ..
make
sudo make install
```

### Method 3: Using Build Script (Recommended)

```bash
bash build/build.sh              # Build all
bash build/build.sh kernel       # Build kernel module only
bash build/build.sh userspace    # Build user-space library only
```

## Build Options

### Debug Build
```bash
cd kernel/usbc2hd4
make debug           # With symbols and optimization disabled
```

### Parallel Build
```bash
make -j4             # Use 4 parallel jobs
make -j$(nproc)      # Use all CPU cores
```

### Clean Build
```bash
make clean           # Remove build artifacts
make clean all       # Clean then rebuild
```

## Verifying the Build

```bash
# Check if kernel module was built
ls -la kernel/usbc2hd4/usbc2hd4.ko

# Check user-space library
ls -la build/lib/libusbc2hd4.*

# Run tests
bash build/test.sh
```

## Installation

### Automatic Installation
```bash
sudo bash build/install.sh --all          # Install everything
sudo bash build/install.sh --kernel       # Kernel module only
sudo bash build/install.sh --userspace    # Library only
```

### Manual Installation

**Kernel Module:**
```bash
cd kernel/usbc2hd4
sudo make install
sudo depmod -a
```

**User-Space Library:**
```bash
cd build
sudo make -C lib install
sudo ldconfig
```

## Loading the Module

### Via modprobe (Recommended)
```bash
# After installation
sudo modprobe usbc2hd4

# Unload
sudo modprobe -r usbc2hd4
```

### Manual insmod
```bash
sudo insmod kernel/usbc2hd4/usbc2hd4.ko

# Unload
sudo rmmod usbc2hd4
```

## Verification

```bash
# Check if module is loaded
lsmod | grep usbc2hd4

# View kernel messages
dmesg | grep USBC2HD4

# Check device files
ls -la /dev/usbc2hd4*

# View module info
modinfo usbc2hd4

# Monitor in real-time
journalctl -f -k | grep USBC2HD4
```

## Troubleshooting

### Build Fails with "Kernel headers not found"

```bash
# Install headers
sudo apt-get install linux-headers-$(uname -r)

# Verify installation
ls -d /lib/modules/$(uname -r)/build

# Rebuild
make clean
make
```

### Permission Denied on Install

```bash
# Ensure you're using sudo
sudo make install

# Or check file permissions
ls -la kernel/usbc2hd4/
```

### Module Won't Load

```bash
# Check for errors
dmesg | tail -20

# Try verbose loading
sudo modprobe -v usbc2hd4

# Check if module is compiled for this kernel
file kernel/usbc2hd4/usbc2hd4.ko
```

## Build System Files

### Kernel Module Build
- `kernel/usbc2hd4/Makefile` - Kernel module build configuration
- Uses Linux kernel build system (kbuild)

### User-Space Library Build
- `lib/CMakeLists.txt` - User-space library build configuration
- Uses CMake build system
- Generates pkg-config file for easy linking

### Top-Level Build
- `CMakeLists.txt` - Project-wide CMake configuration
- Orchestrates kernel and user-space builds

## Build Variables

### Makefile Variables
```bash
KDIR          # Kernel source directory (default: /lib/modules/$(uname -r)/build)
PWD           # Build directory (automatically set)
CFLAGS        # C compiler flags
```

### CMake Variables
```bash
CMAKE_BUILD_TYPE    # Debug or Release (default: Release)
CMAKE_INSTALL_PREFIX # Installation prefix (default: /usr/local)
```

## Advanced Building

### Cross-Compilation

For ARM or other architectures:

```bash
export ARCH=arm64
export CROSS_COMPILE=aarch64-linux-gnu-
make
```

### With Custom Kernel
```bash
make KDIR=/path/to/kernel/source
```

### Install to Custom Location
```bash
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=/opt/usbc2hd4 ..
make
sudo make install
```

## Build Status Indicators

### Successful Build
```
usbc2hd4.ko created in kernel/usbc2hd4/
libusbc2hd4.so created in build/lib/
No compilation warnings or errors
```

### Check Build Log
```bash
# Save build output to file
make 2>&1 | tee build.log

# Review warnings/errors
grep -i "warning\|error" build.log
```

## Next Steps

After building:
1. [Install the driver](../docs/README.md#building)
2. [Load the kernel module](../docs/README.md#loading-the-module)
3. [Test functionality](../build/test.sh)
4. [Try examples](../tools/examples/)

