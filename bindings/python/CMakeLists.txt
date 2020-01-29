# libtuntap python binding CMakeLists.txt
# =======================================

find_package(Python REQUIRED COMPONENTS Development)

set(PY_VERSION ${Python_VERSION_MAJOR})
find_package(Boost REQUIRED COMPONENTS python${PY_VERSION})

Python_add_library(_pytuntap MODULE
    pytuntap.cc
)

target_link_libraries(_pytuntap PRIVATE
    Boost::python${PY_VERSION}
    tuntap++
)

set_target_properties(_pytuntap PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY pytuntap)

add_custom_command(OUTPUT pytuntap/__init__.py
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/__init__.py" $<TARGET_FILE_DIR:_pytuntap>
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/__init__.py"
)

install(TARGETS _pytuntap
    LIBRARY DESTINATION lib/pytuntap/)

install(FILES __init__.py
    DESTINATION lib/pytuntap)
