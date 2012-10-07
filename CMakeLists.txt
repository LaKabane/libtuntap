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
	set(SOURCES_LIST ${SOURCES_LIST} tuntap-unix-bsd.c)
elseif (NetBSD)
	set(SOURCES_LIST ${SOURCES_LIST} tuntap-unix-netbsd.c)
	set(SOURCES_LIST ${SOURCES_LIST} tuntap-unix-bsd.c)
elseif (FreeBSD)
	set(SOURCES_LIST ${SOURCES_LIST} tuntap-unix-freebsd.c)
	set(SOURCES_LIST ${SOURCES_LIST} tuntap-unix-bsd.c)
elseif (Darwin)
	set(SOURCES_LIST ${SOURCES_LIST} tuntap-unix-darwin.c)
	set(SOURCES_LIST ${SOURCES_LIST} tuntap-unix-bsd.c)
elseif()
	message(FATAL_ERROR "Your operating system is not supported yet")
endif()

# Library definition
# ------------------
add_library(tuntap SHARED ${SOURCES_LIST})
set_target_properties(tuntap PROPERTIES VERSION 2.0)

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
  enable_testing()
  include_directories(${CMAKE_CURRENT_SOURCE_DIR})
  link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

  file(GLOB ALL_TESTS regress/test*.c)
  foreach(SOURCE_FILE ${ALL_TESTS})
    string(REPLACE ".c" "" WO_SUFFIX ${SOURCE_FILE})
    string(REPLACE ${CMAKE_CURRENT_SOURCE_DIR}/ "" SRC_BASENAME ${SOURCE_FILE})
    string(REPLACE ${CMAKE_CURRENT_SOURCE_DIR}/regress/ "" BIN_BASENAME ${WO_SUFFIX})
    add_executable(${BIN_BASENAME} ${SRC_BASENAME})
    target_link_libraries(${BIN_BASENAME} tuntap)
  endforeach(SOURCE_FILE)

  file(COPY regress/testPR DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)
  file(COPY regress/testPD DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)

  add_test(test01 ${EXECUTABLE_OUTPUT_PATH}/test01)
  add_test(test02 ${EXECUTABLE_OUTPUT_PATH}/test02)
  add_test(test03 ${EXECUTABLE_OUTPUT_PATH}/test03)
  add_test(test04 ${EXECUTABLE_OUTPUT_PATH}/test04)
  add_test(test05 ${EXECUTABLE_OUTPUT_PATH}/test05)
  add_test(test06 ${EXECUTABLE_OUTPUT_PATH}/test06)
  add_test(test07 ${EXECUTABLE_OUTPUT_PATH}/test07)
  add_test(test08 ${EXECUTABLE_OUTPUT_PATH}/test08)
  add_test(test09 ${EXECUTABLE_OUTPUT_PATH}/test09)
  add_test(test10 ${EXECUTABLE_OUTPUT_PATH}/test10)
  add_test(test11 ${EXECUTABLE_OUTPUT_PATH}/test11)
  add_test(test12 ${EXECUTABLE_OUTPUT_PATH}/test12)
  add_test(test13 ${EXECUTABLE_OUTPUT_PATH}/test13)
  add_test(test14 ${EXECUTABLE_OUTPUT_PATH}/test14)
  # test15 and test16 test IPv6 stuff, which are not implemented
  add_test(test17 ${EXECUTABLE_OUTPUT_PATH}/test17)
  add_test(test18 ${EXECUTABLE_OUTPUT_PATH}/test18)
  # test19 and test20 test IPv6 stuff, which are not implemented
  add_test(test21 ${EXECUTABLE_OUTPUT_PATH}/test21)
  add_test(test22 ${EXECUTABLE_OUTPUT_PATH}/test22)
  add_test(test23 ${EXECUTABLE_OUTPUT_PATH}/test23)
  add_test(test24 ${EXECUTABLE_OUTPUT_PATH}/test24)
  add_test(test25 ${EXECUTABLE_OUTPUT_PATH}/test25)
  add_test(test26 ${EXECUTABLE_OUTPUT_PATH}/test26)
  # test27 and test28 have been depracated
  add_test(test29 ${EXECUTABLE_OUTPUT_PATH}/test29)
  add_test(test30 ${EXECUTABLE_OUTPUT_PATH}/test30)
  add_test(test31 ${EXECUTABLE_OUTPUT_PATH}/test31)
  add_test(test32 ${EXECUTABLE_OUTPUT_PATH}/test32)
  add_test(test33 ${EXECUTABLE_OUTPUT_PATH}/testPD ${EXECUTABLE_OUTPUT_PATH}/test33 tap0)
  add_test(test34 ${EXECUTABLE_OUTPUT_PATH}/testPD ${EXECUTABLE_OUTPUT_PATH}/test34 tun0)
  add_test(test35 ${EXECUTABLE_OUTPUT_PATH}/testPR ${EXECUTABLE_OUTPUT_PATH}/test35 tap0)
  add_test(test36 ${EXECUTABLE_OUTPUT_PATH}/testPR ${EXECUTABLE_OUTPUT_PATH}/test36 tun1)
  add_test(test37 ${EXECUTABLE_OUTPUT_PATH}/test37)
  add_test(test38 ${EXECUTABLE_OUTPUT_PATH}/test38)
  # test39 tests debug mode, which is not compiled by default
  add_test(test39 ${EXECUTABLE_OUTPUT_PATH}/test39)
  add_test(test40 ${EXECUTABLE_OUTPUT_PATH}/test40)
  add_test(test41 ${EXECUTABLE_OUTPUT_PATH}/test41)
  add_test(test42 ${EXECUTABLE_OUTPUT_PATH}/test42)
  add_test(test43 ${EXECUTABLE_OUTPUT_PATH}/test43)
  add_test(test44 ${EXECUTABLE_OUTPUT_PATH}/test44)
  add_test(test45 ${EXECUTABLE_OUTPUT_PATH}/test45)

  # NetBSD and FreeBSD have the debug mode compiled by default
  if(NOT NetBSD AND NOT FreeBSD)
    set_tests_properties(test39 PROPERTIES WILL_FAIL true)
    set_tests_properties(test40 PROPERTIES WILL_FAIL true)
  endif()
  # Linux has a working tuntap_set_ifname()
  if(NOT Linux)
    set_tests_properties(test41 PROPERTIES WILL_FAIL true)
  endif()
  # FreeBSD and OpenBSD have a working tuntap_set_descr()
  if(NOT FreeBSD AND NOT OpenBSD)
    set_tests_properties(test44 PROPERTIES WILL_FAIL true)
  endif()
endif (ENABLE_REGRESS)

