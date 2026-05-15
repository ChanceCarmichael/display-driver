cmake_minimum_required(VERSION 3.10)
project(usbc2hd4_driver VERSION 0.1.0 LANGUAGES C)

# Add subdirectories
add_subdirectory(lib)

# Display project configuration
message(STATUS "USBC2HD4 Driver Project Configuration:")
message(STATUS "  Project version: ${PROJECT_VERSION}")
message(STATUS "  Build directory: ${CMAKE_BINARY_DIR}")

# Add custom targets for kernel module
add_custom_target(kernel_module ALL
	COMMAND make -C ${CMAKE_CURRENT_SOURCE_DIR}/kernel/usbc2hd4
	COMMENT "Building kernel module"
)

add_custom_target(kernel_module_install
	COMMAND make -C ${CMAKE_CURRENT_SOURCE_DIR}/kernel/usbc2hd4 install
	COMMENT "Installing kernel module"
	REQUIRES kernel_module
)

add_custom_target(kernel_module_clean
	COMMAND make -C ${CMAKE_CURRENT_SOURCE_DIR}/kernel/usbc2hd4 clean
	COMMENT "Cleaning kernel module build"
)

# Print build instructions
message(STATUS "")
message(STATUS "Build Instructions:")
message(STATUS "  mkdir build && cd build")
message(STATUS "  cmake ..")
message(STATUS "  make")
message(STATUS "  sudo make install")
message(STATUS "")
message(STATUS "Kernel Module:")
message(STATUS "  make kernel_module")
message(STATUS "  sudo make kernel_module_install")
