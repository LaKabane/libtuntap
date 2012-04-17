# libtuntap CMakeLists.txt 
# ========================

project(libtuntap)

cmake_minimum_required(VERSION 2.8)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_INSTALL_PREFIX "/usr/local/")
set(${CMAKE_SYSTEM_NAME} True)

# Global definitions
# ------------------
add_definitions(-D${CMAKE_SYSTEM_NAME})

# OS families specific things
# ---------------------------
if(UNIX)
	include_directories(
		/usr/include/
		/usr/local/include
	)

	set(CMAKE_C_FLAGS "-Wall -Wextra -ansi")
	set(SOURCES_LIST tuntap.c)

endif(UNIX)

if(WINDOWS)
	message("Windows is not supported yet")
endif(WINDOWS)

# OS specific things
# ------------------
if(Linux)
	add_definitions(-D_GNU_SOURCE)
	set(SOURCES_LIST ${SOURCES_LIST} tuntap-linux.c)
	#find_package(Bsd)
	#if (BSD_FOUND)
	#	target_link_libraries(tNETacle ${BSD_LIBRARIES})
	#endif(BSD_FOUND)
endif(Linux)

if(OpenBSD)
	set(SOURCES_LIST ${SOURCES_LIST} tuntap-openbsd.c)
endif(OpenBSD)

if(Darwin OR NetBSD OR FreeBSD OR SunOS)
	message("Your operating system is not supported yet")
endif()

# Target
# ------
add_library(tuntap ${SOURCES_LIST})

# Install rules
# -------------
if(UNIX)
	install(TARGETS tuntap DESTINATION lib)
	install(FILES tuntap.h DESTINATION include)
endif(UNIX)

