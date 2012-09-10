# libtuntap CMakeLists.txt 
# ========================

project(libtuntap)
cmake_minimum_required(VERSION 2.8)

# CMake Configuration
# ------------------
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_INSTALL_PREFIX "/usr/local/")
set(${CMAKE_SYSTEM_NAME} True)

# Global CPP definitions
# ----------------------
add_definitions(-D${CMAKE_SYSTEM_NAME})

# Portable source files
# ------------------
set(SOURCES_LIST
	tuntap_log.c
)

# OS families specific things
# ---------------------------
if(UNIX)
	# Unix specific include directories
	# ------------------
	include_directories(
		/usr/include/
		/usr/local/include
	)

	# Unix specific definitions
	# ------------------
  	add_definitions(-DUnix)

	# Unix specific source files
	# ------------------
	set(SOURCES_LIST ${SOURCES_LIST} tuntap-unix.c )

endif(UNIX)

if(WINDOWS)
 	# Windows specific definitions
	# ------------------
 	add_definitions(-DWindows)

	# Windows specific source files
	# ------------------
	set(SOURCES_LIST ${SOURCES_LIST} tuntap-windows.c )

endif(WINDOWS)

# OS specific things
# ------------------
if(Linux)
	set(CMAKE_INSTALL_PREFIX "/usr/")
	add_definitions(-D_GNU_SOURCE)
	set(SOURCES_LIST ${SOURCES_LIST} tuntap-unix-linux.c)
elseif (OpenBSD)
	set(SOURCES_LIST ${SOURCES_LIST} tuntap-unix-openbsd.c)
elseif (NetBSD)
	set(SOURCES_LIST ${SOURCES_LIST} tuntap-unix-netbsd.c)
elseif (Darwin)
	set(SOURCES_LIST ${SOURCES_LIST} tuntap-unix-darwin.c)
elseif()
	message(FATAL_ERROR "Your operating system is not supported yet")
endif()

# Library definition
# ------------------
add_library(tuntap SHARED ${SOURCES_LIST})
set_target_properties(tuntap PROPERTIES VERSION 1.0)

add_library(tuntap-static STATIC ${SOURCES_LIST})
set_target_properties(tuntap-static PROPERTIES OUTPUT_NAME "tuntap")
set_target_properties(tuntap-static PROPERTIES PREFIX "lib")

# Install rules
# -------------
if(UNIX)
	install(TARGETS tuntap DESTINATION lib)
	install(TARGETS tuntap-static DESTINATION lib)
	install(FILES tuntap.h DESTINATION include)
endif(UNIX)

include(CMakeLists.txt.local OPTIONAL)
