cmake_minimum_required(VERSION 3.10)
project(usbc2hd4_lib VERSION 0.1.0 LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -O0 -DDEBUG")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2 -DNDEBUG")

# Default to Release build
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Source files
set(USBC2HD4_LIB_SOURCES
	src/usbc2hd4_lib.c
)

# Header files (for IDEs)
set(USBC2HD4_LIB_HEADERS
	include/usbc2hd4_lib.h
)

# Shared library
add_library(usbc2hd4 SHARED ${USBC2HD4_LIB_SOURCES} ${USBC2HD4_LIB_HEADERS})

# Set library properties
set_target_properties(usbc2hd4 PROPERTIES
	PUBLIC_HEADER include/usbc2hd4_lib.h
	SOVERSION 0
	VERSION 0.1.0
)

# Installation
install(TARGETS usbc2hd4
	LIBRARY DESTINATION lib
	PUBLIC_HEADER DESTINATION include
)

# Static library
add_library(usbc2hd4_static STATIC ${USBC2HD4_LIB_SOURCES} ${USBC2HD4_LIB_HEADERS})
set_target_properties(usbc2hd4_static PROPERTIES
	OUTPUT_NAME usbc2hd4
	PREFIX lib
)

install(TARGETS usbc2hd4_static
	ARCHIVE DESTINATION lib
)

# pkg-config file
configure_file(usbc2hd4.pc.in usbc2hd4.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/usbc2hd4.pc
	DESTINATION lib/pkgconfig
)

# Display configuration
message(STATUS "USBC2HD4 Library Configuration:")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
