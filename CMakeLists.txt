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
	set(SOURCES_LIST
		tuntap.c
		tuntap_log.c
	)

endif(UNIX)

if(WINDOWS)
	message(FATAL_ERROR "Windows is not supported yet")
endif(WINDOWS)

# OS specific things
# ------------------
if(Linux)
	add_definitions(-D_GNU_SOURCE)
	set(SOURCES_LIST ${SOURCES_LIST} tuntap-linux.c)
elseif (OpenBSD)
	set(SOURCES_LIST ${SOURCES_LIST} tuntap-openbsd.c)
elseif (NetBSD)
	set(SOURCES_LIST ${SOURCES_LIST} tuntap-netbsd.c)
elseif (Darwin)
	set(SOURCES_LIST ${SOURCES_LIST} tuntap-darwin.c)
elseif()
	message(FATAL_ERROR "Your operating system is not supported yet")
endif()

# Library definition
# ------------------
add_library(tuntap ${SOURCES_LIST})

# Install rules
# -------------
if(UNIX)
	install(TARGETS tuntap DESTINATION lib)
	install(FILES tuntap.h DESTINATION include)
endif(UNIX)

include(CMakeLists.txt.local OPTIONAL)
