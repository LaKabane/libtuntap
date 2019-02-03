# libtuntap python binding CMakeLists.txt
# =======================================

find_package(PythonLibs)
find_package(Boost COMPONENTS system thread python3)

if(${PYTHONLIBS_FOUND} AND ${Boost_FOUND})

    add_library(pytuntap SHARED
        pytuntap.cc
    )

    target_include_directories(pytuntap PRIVATE
        ${PYTHON_INCLUDE_PATH}
        ${Boost_INCLUDE_PATH})

    target_link_libraries(pytuntap
        ${PYTHON_LIBRARIES}
        ${Boost_LIBRARIES}
        tuntap++
    )

    add_custom_command(TARGET pytuntap POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/__init__.py"
        $<TARGET_FILE_DIR:pytuntap>
    )

    set_target_properties(pytuntap PROPERTIES
        OUTPUT_NAME "_pytuntap"
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/bindings/python/pytuntap)
endif()

