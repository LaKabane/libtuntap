# libtuntap C++ binding regression tests CMakeLists.txt
# =====================================================

include_directories(${CMAKE_SOURCE_DIR}/bindings/cpp/)
link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

# C++ based tests
# ---------------
file(GLOB ALL_CXX_TESTS regress/bindings/cpp/test++*.cc)
foreach(SOURCE_FILE ${ALL_CXX_TESTS})
    string(REPLACE ".cc" "" PATH_WO_SUFFIX ${SOURCE_FILE})
    string(REPLACE ${CMAKE_CURRENT_SOURCE_DIR}/ ""
      TEST_SRC_PATH ${SOURCE_FILE})
    string(REPLACE ${CMAKE_CURRENT_SOURCE_DIR}/regress/bindings/cpp/ ""
      TEST_NAME ${PATH_WO_SUFFIX})
    
    add_executable(${TEST_NAME} ${TEST_SRC_PATH})
    target_link_libraries(${TEST_NAME} tuntap++)
    add_test(${TEST_NAME} ${EXECUTABLE_OUTPUT_PATH}/${TEST_NAME})
endforeach(SOURCE_FILE)

