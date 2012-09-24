# libtuntap regression tests CMakeLists.txt 
# =========================================

project(libtuntap-regression)
cmake_minimum_required(VERSION 2.8)

set(${CMAKE_SYSTEM_NAME} True)

enable_testing()
set(TUNTAP tuntap)
include_directories(..)
# XXX: You must build libtuntap in the 'build' directory first
link_directories(${CMAKE_BINARY_DIR}/../../build/lib)

if(UNIX)
  add_definitions(-DUnix)
endif(UNIX)
if (Windows)
  add_definitions(-DWindows)
endif(Windows)

file(GLOB ALL_TESTS test*.c)
foreach(SOURCE_FILE ${ALL_TESTS})
  string(REPLACE ".c" "" TMP ${SOURCE_FILE})
  string(REPLACE ${CMAKE_CURRENT_SOURCE_DIR}/ "" TEST ${TMP})
  add_executable(${TEST} ${TEST}.c)
  target_link_libraries(${TEST} ${TUNTAP})
endforeach(SOURCE_FILE)

# Selectively add tests
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
add_test(test33 ${EXECUTABLE_OUTPUT_PATH}/test33)
add_test(test34 ${EXECUTABLE_OUTPUT_PATH}/test34)
add_test(test35 ${EXECUTABLE_OUTPUT_PATH}/test35)
add_test(test36 ${EXECUTABLE_OUTPUT_PATH}/test36)
add_test(test37 ${EXECUTABLE_OUTPUT_PATH}/test37)
add_test(test38 ${EXECUTABLE_OUTPUT_PATH}/test38)
