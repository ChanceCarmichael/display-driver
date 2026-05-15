# Project Structure

```
linux-driver/
├── README.md                          # Main documentation
├── CMakeLists.txt                     # Top-level CMake configuration
│
├── kernel/                            # Kernel module source
│   └── usbc2hd4/
│       ├── Makefile                   # Kernel module build
│       ├── usbc2hd4.h                 # Header file with structures
│       ├── main.c                     # Module initialization
│       ├── usb_probe.c                # USB device detection
│       ├── it66122.c                  # IT66122 chip communication
│       ├── display_enum.c             # Display enumeration
│       └── hotplug.c                  # Hot-plug detection
│
├── lib/                               # User-space library
│   ├── CMakeLists.txt                 # Library build configuration
│   ├── usbc2hd4.pc.in                 # pkg-config template
│   ├── include/
│   │   └── usbc2hd4_lib.h             # Public API header
│   └── src/
│       └── usbc2hd4_lib.c             # Library implementation
│
├── build/                             # Build and deployment scripts
│   ├── build.sh                       # Main build script
│   ├── install.sh                     # Installation script
│   ├── test.sh                        # Test suite
│   └── quick_start.sh                 # Quick start guide
│
├── tools/                             # Utilities and examples
│   └── examples/
│       ├── enumerate_example.c        # Display enumeration example
│       └── set_resolution_example.c   # Resolution setting example
│
└── docs/                              # Documentation
    ├── README.md                      # Getting started
    ├── API.md                         # API reference
    ├── BUILD.md                       # Build instructions
    └── STRUCTURE.md                   # This file
```

## Component Descriptions

### Kernel Module (`kernel/usbc2hd4/`)

The kernel driver provides USB device detection, display enumeration, and hardware communication.

**Key Files:**
- **usbc2hd4.h**: Data structures and function prototypes
  - `struct usbc2hd4_device` - Device context
  - `struct display_info` - Display information
  - Device constants and command codes

- **main.c**: Module lifecycle
  - `usbc2hd4_module_init()` - Module initialization
  - `usbc2hd4_module_exit()` - Module cleanup
  - Character device registration
  - Module parameters

- **usb_probe.c**: USB device management
  - `usbc2hd4_probe()` - Device attachment
  - `usbc2hd4_disconnect()` - Device removal
  - Device structure initialization

- **it66122.c**: IT66122 chip communication
  - `it66122_send_command()` - USB control transfer protocol
  - Command/response handling
  - USB buffer management

- **display_enum.c**: Display functions
  - `usbc2hd4_enumerate_displays()`
  - `usbc2hd4_get_display_info()`
  - `usbc2hd4_set_resolution()`
  - `usbc2hd4_set_orientation()`
  - `usbc2hd4_enable_display()`
  - `usbc2hd4_disable_display()`

- **hotplug.c**: Hot-plug support
  - `usbc2hd4_setup_hotplug()` - Initialize interrupt monitoring
  - `usbc2hd4_hotplug_irq()` - Interrupt handler
  - `usbc2hd4_hotplug_work()` - Event handling

### User-Space Library (`lib/`)

Provides C API for user-space applications.

**Key Files:**
- **usbc2hd4_lib.h**: Public API
  - Device handle type
  - Data structures (display info, device info, resolutions)
  - Function declarations
  - Error codes

- **usbc2hd4_lib.c**: Implementation
  - Device file access
  - ioctl wrappers (TODO: implement)
  - Error handling
  - Memory management

- **CMakeLists.txt**: Build configuration
  - Shared library target
  - Static library target
  - Installation rules
  - pkg-config file generation

- **usbc2hd4.pc.in**: pkg-config metadata
  - Library name and version
  - Installation paths
  - Compilation flags

### Build System (`build/`)

Scripts for building, testing, and installing.

**Key Files:**
- **build.sh**: Orchestrates kernel and user-space builds
  - Checks prerequisites
  - Builds kernel module
  - Builds user-space library
  - Parallel job control
  - Debug mode support

- **install.sh**: Deployment script
  - Module installation
  - Library installation
  - Uninstall support
  - Verification

- **test.sh**: Validation suite
  - Environment checks
  - File presence verification
  - C syntax validation
  - Configuration validation

- **quick_start.sh**: Interactive setup
  - Prerequisite checking
  - Build automation
  - Installation guidance
  - Documentation links

### Examples (`tools/examples/`)

Sample programs demonstrating library usage.

**Key Files:**
- **enumerate_example.c**: Display enumeration
  - Device opening
  - Display enumeration
  - Info retrieval

- **set_resolution_example.c**: Resolution configuration
  - Display enabling
  - Resolution setting
  - Error handling

### Documentation (`docs/`)

Complete reference and guides.

**Key Files:**
- **README.md**: Getting started guide
  - Architecture overview
  - Build instructions
  - Module loading
  - Troubleshooting

- **API.md**: Complete API reference
  - Data structures
  - Function prototypes
  - Parameter descriptions
  - Error codes
  - Example code

- **BUILD.md**: Detailed build guide
  - Prerequisites
  - Build methods
  - Verification
  - Troubleshooting

- **STRUCTURE.md**: This file
  - Project layout
  - Component descriptions
  - File organization

## File Organization Principles

### Separation of Concerns
- Kernel module: Device communication, USB protocol, hardware control
- User-space library: Application interface, device access, error handling
- Documentation: Usage guides, API reference, examples

### Modularity
- Each C module has specific responsibilities
- Clear interfaces between modules
- Minimal coupling

### Build System
- Top-level CMakeLists.txt for project coordination
- Kernel module uses kbuild (standard Linux approach)
- User-space uses CMake for cross-platform support

### Documentation
- Multiple documentation formats:
  - README.md: Quick start and overview
  - API.md: Function reference
  - BUILD.md: Build instructions
  - STRUCTURE.md: Project layout

## Adding New Functionality

### Adding a Kernel Function
1. Declare prototype in `kernel/usbc2hd4/usbc2hd4.h`
2. Implement in appropriate `.c` file
3. Add to `usbc2hd4_device` operations if needed
4. Update `lib/include/usbc2hd4_lib.h` wrapper

### Adding a Library Function
1. Declare in `lib/include/usbc2hd4_lib.h`
2. Implement in `lib/src/usbc2hd4_lib.c`
3. Add example if needed in `tools/examples/`
4. Document in `docs/API.md`

### Adding Documentation
1. Update relevant .md file in `docs/`
2. Add examples if needed
3. Update table of contents if new file

## Build Output

After successful build:

```
build/
├── lib/
│   ├── CMakeFiles/          # CMake intermediate files
│   ├── libusbc2hd4.so       # Shared library
│   ├── libusbc2hd4.a        # Static library
│   └── usbc2hd4.pc          # pkg-config file
│
kernel/usbc2hd4/
├── *.o                      # Object files
├── *.ko                     # Kernel module
├── Module.symvers           # Module symbols
└── .*.cmd                   # Build commands
```

## Configuration Files

- `.gitignore` - Files to exclude from version control
- `CMakeLists.txt` - CMake configuration
- `kernel/usbc2hd4/Makefile` - Kernel module build rules

