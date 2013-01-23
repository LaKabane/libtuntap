# libtuntap CMakeLists.txt 
# ========================

project(libtuntap)
cmake_minimum_required(VERSION 2.8)

# CMake global options
# ------------------
option(ENABLE_REGRESS "Enable the regression tests" OFF)

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
	tuntap_destroy.c
	tuntap_log.c
	tuntap_init.c
	tuntap_version.c
)

# OS families specific things
# ---------------------------
if(UNIX)
	# Unix specific include directories
	# ------------------
	include_directories(
		"${CMAKE_CURRENT_SOURCE_DIR}"
		/usr/include/
		/usr/local/include
	)

	# Unix specific definitions
	# ------------------
  	add_definitions(-DUnix)

	# Unix source files
	# -----------------
	set(SOURCES_LIST ${SOURCES_LIST}
		tuntap_down.c
		tuntap_get_hwaddr.c
		tuntap_get_ifname.c
		tuntap_get_mtu.c
		tuntap_get_readable.c
		tuntap_read.c
		tuntap_release.c
		tuntap_set_debug.c
		tuntap_set_descr.c
		tuntap_set_hwaddr.c
		tuntap_set_ifname.c
		tuntap_set_ip.c
		tuntap_set_mtu.c
		tuntap_set_nonblocking.c
		tuntap_start.c
		tuntap_up.c
		tuntap_write.c
	)
endif(UNIX)

if(Windows)
 	# Windows specific definitions
	# ------------------
 	add_definitions(-DWindows)
	# Windows source files
	# --------------------
	set(SOURCES_LIST ${SOURCES_LIST}
		tuntap-sys-windows.c
	)
endif(Windows)

# OS specific things
# ------------------
if(UNIX)
if(Linux)
	set(CMAKE_INSTALL_PREFIX "/usr/")
	add_definitions(-D_GNU_SOURCE)
	set(SOURCES_LIST ${SOURCES_LIST} tuntap-sys-linux.c)
elseif (OpenBSD)
	set(SOURCES_LIST ${SOURCES_LIST} tuntap-sys-openbsd.c)
elseif (NetBSD)
	set(SOURCES_LIST ${SOURCES_LIST} tuntap-sys-netbsd.c)
elseif (FreeBSD)
	set(SOURCES_LIST ${SOURCES_LIST} tuntap-sys-freebsd.c)
elseif (Darwin)
	set(SOURCES_LIST ${SOURCES_LIST} tuntap-sys-darwin.c)
elseif (DragonFly)
	set(SOURCES_LIST ${SOURCES_LIST} tuntap-sys-freebsd.c)
else()
	message(FATAL_ERROR "Your operating system is not supported yet")
endif()
endif(UNIX)

# Library definition
# ------------------
add_library(tuntap SHARED ${SOURCES_LIST} tuntap.h)
set_target_properties(tuntap PROPERTIES VERSION 2.1)

add_library(tuntap-static STATIC ${SOURCES_LIST})
set_target_properties(tuntap-static PROPERTIES OUTPUT_NAME "tuntap")
if(UNIX)
    set_target_properties(tuntap-static PROPERTIES PREFIX "lib")
endif(UNIX)

if (Windows)
	target_link_libraries(tuntap Ws2_32.lib)
	target_link_libraries(tuntap-static Ws2_32.lib)
endif(Windows)

# Install rules
# -------------
if(UNIX)
	install(TARGETS tuntap DESTINATION lib)
	install(TARGETS tuntap-static DESTINATION lib)
	install(FILES tuntap.h DESTINATION include)
endif(UNIX)

include(CMakeLists.txt.local OPTIONAL)

# Tests rules
# -----------
if (ENABLE_REGRESS)
  include(regress/CMakeLists.txt)
endif (ENABLE_REGRESS)

