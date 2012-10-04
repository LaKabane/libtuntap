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
	tuntap.c
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

if(Windows)
 	# Windows specific definitions
	# ------------------
 	add_definitions(-DWindows)

	# Windows specific source files
	# ------------------
	set(SOURCES_LIST ${SOURCES_LIST} tuntap-windows.c )

endif(Windows)

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

# Tests rules
# -----------
if (ENABLE_REGRESS)
  enable_testing()
  include_directories(${CMAKE_CURRENT_SOURCE_DIR})
  link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

  file(GLOB ALL_TESTS regress/test*.c)
  foreach(SOURCE_FILE ${ALL_TESTS})
    message(${SOURCE_FILE})
    string(REPLACE ".c" "" WO_SUFFIX ${SOURCE_FILE})
    string(REPLACE ${CMAKE_CURRENT_SOURCE_DIR}/ "" SRC_BASENAME ${SOURCE_FILE})
    string(REPLACE ${CMAKE_CURRENT_SOURCE_DIR}/regress/ "" BIN_BASENAME ${WO_SUFFIX})
    message(${SRC_BASENAME} ${BIN_BASENAME})
    add_executable(${BIN_BASENAME} ${SRC_BASENAME})
    target_link_libraries(${BIN_BASENAME} tuntap)
  endforeach(SOURCE_FILE)
endif (ENABLE_REGRESS)

include(CMakeLists.txt.local OPTIONAL)
