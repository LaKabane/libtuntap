# libtuntap python binding regression tests CMakeLists.txt
# ========================================================

include_directories(${CMAKE_SOURCE_DIR})

# Python based tests
# ------------------
file(GLOB ALL_PY_TESTS regress/bindings/python/testpy*.py)
foreach(SOURCE_FILE ${ALL_PY_TESTS})
    string(REPLACE ".py" "" PATH_WO_SUFFIX ${SOURCE_FILE})
    string(REPLACE ${CMAKE_CURRENT_SOURCE_DIR}/ ""
      TEST_SRC_PATH ${SOURCE_FILE})
    string(REPLACE ${CMAKE_CURRENT_SOURCE_DIR}/regress/bindings/python/ ""
      TEST_NAME ${PATH_WO_SUFFIX})
 
    # XXX: As for .sh test files, this is cancer
    file(COPY ${TEST_SRC_PATH}
         DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
         FILE_PERMISSIONS OWNER_READ OWNER_EXECUTE
    )
    file(RENAME ${CMAKE_CURRENT_BINARY_DIR}/${TEST_NAME}.py
        ${CMAKE_CURRENT_BINARY_DIR}/${TEST_NAME}
    )

    add_test(${TEST_NAME} ${CMAKE_CURRENT_BINARY_DIR}/${TEST_NAME})
endforeach(SOURCE_FILE)

