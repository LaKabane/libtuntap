# libtuntap regression tests CMakeLists.txt
# =========================================

# C based tests
# -------------
file(GLOB ALL_C_TESTS test*.c)
foreach(SOURCE_FILE ${ALL_C_TESTS})
  get_filename_component(TEST_NAME ${SOURCE_FILE} NAME_WE)

  add_executable(${TEST_NAME} ${SOURCE_FILE})
  target_link_libraries(${TEST_NAME} tuntap)

  add_test(NAME tuntap.${TEST_NAME}
           COMMAND $<TARGET_FILE:${TEST_NAME}>)
endforeach(SOURCE_FILE)

# Shell based tests (needs a posix shell)
# -----------------
if (UNIX)
  file(GLOB ALL_SH_TESTS test*.sh)
  foreach(SOURCE_FILE ${ALL_SH_TESTS})
    get_filename_component(TEST_NAME ${SOURCE_FILE} NAME_WE)
    get_filename_component(TEST_SRC_PATH ${SOURCE_FILE} DIRECTORY)

    string(REPLACE "test" "helper" HELPER_NAME ${TEST_NAME})

    if (EXISTS ${TEST_SRC_PATH}/${HELPER_NAME}.c)
        add_executable(${HELPER_NAME} ${TEST_SRC_PATH}/${HELPER_NAME}.c)
        target_link_libraries(${HELPER_NAME} tuntap)
    endif()
    add_test(NAME tuntap.${TEST_NAME}
            COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/${TEST_NAME}.sh)
  endforeach(SOURCE_FILE)
endif()

# "Will fail" tests
# -----------------

if(Darwin OR Windows)
  set_tests_properties(tuntap.test39 PROPERTIES WILL_FAIL true)
  set_tests_properties(tuntap.test40 PROPERTIES WILL_FAIL true)
endif()

# There is no debug mode for tap on NetBSD
if(NetBSD)
  set_tests_properties(tuntap.test39 PROPERTIES WILL_FAIL true)
endif()

# Persistence is not implemented on Darwin
if(Darwin)
  set_tests_properties(tuntap.test33 PROPERTIES WILL_FAIL true)
  set_tests_properties(tuntap.test34 PROPERTIES WILL_FAIL true)
  set_tests_properties(tuntap.test35 PROPERTIES WILL_FAIL true)
  set_tests_properties(tuntap.test36 PROPERTIES WILL_FAIL true)
endif()

# Only Linux has a tuntap_set_ifname() implemented
if(NOT Linux)
  set_tests_properties(tuntap.test41 PROPERTIES WILL_FAIL true)
endif()

# Only Linux has a tuntap_sys_set_ipv6() implemented
if(NOT Linux)
  set_tests_properties(tuntap.test15 PROPERTIES WILL_FAIL true)
  set_tests_properties(tuntap.test16 PROPERTIES WILL_FAIL true)
  set_tests_properties(tuntap.test19 PROPERTIES WILL_FAIL true)
  set_tests_properties(tuntap.test20 PROPERTIES WILL_FAIL true)
endif()

# Only FreeBSD and OpenBSD have a tuntap_set_descr() implemented
if(NOT FreeBSD AND NOT OpenBSD)
  set_tests_properties(tuntap.test44 PROPERTIES WILL_FAIL true)
  set_tests_properties(tuntap.test46 PROPERTIES WILL_FAIL true)
endif()

# Windows work-in-progress (tap)
if (Windows)
  set_tests_properties(tuntap.test05 PROPERTIES WILL_FAIL true)
  set_tests_properties(tuntap.test07 PROPERTIES WILL_FAIL true)
  set_tests_properties(tuntap.test09 PROPERTIES WILL_FAIL true)
  set_tests_properties(tuntap.test39 PROPERTIES WILL_FAIL true)
  set_tests_properties(tuntap.test41 PROPERTIES WILL_FAIL true)
  set_tests_properties(tuntap.test44 PROPERTIES WILL_FAIL true)
endif()

# Windows work-in-progress (tun)
if (Windows)
  set_tests_properties(tuntap.test02 PROPERTIES WILL_FAIL true)
  set_tests_properties(tuntap.test04 PROPERTIES WILL_FAIL true)
  set_tests_properties(tuntap.test06 PROPERTIES WILL_FAIL true)
  set_tests_properties(tuntap.test08 PROPERTIES WILL_FAIL true)
  set_tests_properties(tuntap.test10 PROPERTIES WILL_FAIL true)
  set_tests_properties(tuntap.test12 PROPERTIES WILL_FAIL true)
  set_tests_properties(tuntap.test14 PROPERTIES WILL_FAIL true)
  set_tests_properties(tuntap.test18 PROPERTIES WILL_FAIL true)
  set_tests_properties(tuntap.test22 PROPERTIES WILL_FAIL true)
  set_tests_properties(tuntap.test24 PROPERTIES WILL_FAIL true)
  set_tests_properties(tuntap.test26 PROPERTIES WILL_FAIL true)
  set_tests_properties(tuntap.test40 PROPERTIES WILL_FAIL true)
endif()
