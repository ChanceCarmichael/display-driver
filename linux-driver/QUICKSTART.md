# Quick Reference

## Getting Started in 3 Steps

### Step 1: Build
```bash
cd linux-driver
make build
# or: bash build/build.sh all
```

### Step 2: Test
```bash
make test
# Verifies project structure and prerequisites
```

### Step 3: Install
```bash
make install
# or: sudo bash build/install.sh --all
```

---

## Common Commands

### Build
```bash
make                    # Build everything (kernel + library)
make kernel             # Build kernel module only
make userspace          # Build library only
make clean              # Remove build artifacts
```

### Install
```bash
make install            # Install everything (requires sudo)
make install-kernel     # Install kernel module
make install-userspace  # Install library
make uninstall          # Remove installations
```

### Debug & Test
```bash
make test               # Run validation tests
make quick-start        # Interactive setup guide
make help               # Show all targets
make docs               # List documentation files
```

---

## Module Usage

### Load Module
```bash
sudo modprobe usbc2hd4          # Load
lsmod | grep usbc2hd4           # Verify
dmesg | grep USBC2HD4           # View logs
```

### Unload Module
```bash
sudo modprobe -r usbc2hd4       # Unload
```

### Check Status
```bash
ls -la /dev/usbc2hd4*           # Check device files
dmesg | tail -20                # View kernel messages
```

---

## Library Usage

### Link in Your Code
```bash
# Compile with:
gcc -o myapp myapp.c -lusbc2hd4

# Or using pkg-config:
gcc -o myapp myapp.c $(pkg-config --cflags --libs usbc2hd4)
```

### Basic Example
```c
#include <usbc2hd4_lib.h>

usbc2hd4_handle_t dev = usbc2hd4_open();
if (!dev) return 1;

int num = usbc2hd4_enumerate_displays(dev);
usbc2hd4_set_resolution(dev, 0, 1920, 1080, 60);

usbc2hd4_close(dev);
```

---

## Project Files

| File | Purpose |
|------|---------|
| `kernel/usbc2hd4/` | Kernel module source |
| `lib/` | User-space library |
| `build/` | Build scripts |
| `docs/` | Documentation |
| `tools/examples/` | Example programs |
| `Makefile` | Convenience wrapper |
| `CMakeLists.txt` | CMake configuration |
| `.gitignore` | Git configuration |

---

## Documentation

| Document | Contents |
|----------|----------|
| [README.md](docs/README.md) | Overview, building, installation |
| [API.md](docs/API.md) | Function reference, examples |
| [BUILD.md](docs/BUILD.md) | Detailed build instructions |
| [TROUBLESHOOTING.md](docs/TROUBLESHOOTING.md) | Common issues & fixes |
| [STRUCTURE.md](docs/STRUCTURE.md) | Project organization |

---

## Troubleshooting

### Build Issues
```bash
# Missing kernel headers
sudo apt-get install linux-headers-$(uname -r)

# Missing build tools
sudo apt-get install build-essential cmake
```

### Module Won't Load
```bash
# Check kernel version
uname -r

# Rebuild for current kernel
make clean
make

# Try manual load
sudo insmod kernel/usbc2hd4/usbc2hd4.ko
```

### Device Not Found
```bash
# Verify USB connection
lsusb | grep 0bda

# Check kernel logs
dmesg | grep -i usbc2hd4
```

---

## Key Functions

### Kernel Module
- `usbc2hd4_enumerate_displays()` - Find displays
- `usbc2hd4_get_display_info()` - Get display info
- `usbc2hd4_set_resolution()` - Set resolution
- `usbc2hd4_enable_display()` - Enable output
- `usbc2hd4_disable_display()` - Disable output

### User-Space Library
- `usbc2hd4_open()` - Open device
- `usbc2hd4_close()` - Close device
- `usbc2hd4_enumerate_displays()` - Find displays
- `usbc2hd4_set_resolution()` - Configure resolution
- `usbc2hd4_get_error_string()` - Get error message

---

## Next Steps

1. **Read**: [README.md](docs/README.md) for detailed guide
2. **Try Examples**: `tools/examples/enumerate_example.c`
3. **Review API**: [API.md](docs/API.md) for all functions
4. **Troubleshoot**: [TROUBLESHOOTING.md](docs/TROUBLESHOOTING.md) if issues

---

## Support Resources

- Kernel Documentation: `/lib/modules/$(uname -r)/build/Documentation`
- USB Driver Info: `https://www.kernel.org/doc/html/latest/driver-api/usb/`
- IT66122 Datasheet: See `../usbc2hd4_datasheet.pdf`

---

## Module Info

- **Device**: StarTech USBC2HD4 (USB-C to Quad HDMI)
- **Chipset**: Trigger T6-688SL (IT66122)
- **Vendor ID**: 0x0BDA (Realtek)
- **Product ID**: 0x8153
- **Max Displays**: 4 HDMI outputs
- **License**: GPL v2

---

For complete documentation, see the `docs/` directory or run `make help`.

