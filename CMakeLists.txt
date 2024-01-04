# libtuntap CMakeLists.txt
# ========================
cmake_minimum_required(VERSION 3.5)

project(libtuntap)

include(CTest)

# CMake global options
# --------------------
option(ENABLE_CXX "Enable the C++ wrapper library" ON)
option(ENABLE_PYTHON "Enable the Python wrapper library" OFF)

include(CMakeLists.txt.local OPTIONAL)
include(GenerateExportHeader)

if(ENABLE_PYTHON AND NOT ENABLE_CXX)
    set(ENABLE_CXX ON)
    message(WARNING "ENABLE_CXX also set to ON")
endif()

# CMake Configuration
# -------------------
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(${CMAKE_SYSTEM_NAME} True)

# Tuntap library declaration and portable source files
# --------------------------
add_library(tuntap
            tuntap.c
            tuntap.h
            tuntap_log.c
)
# Global CPP definitions
# ----------------------
target_compile_definitions(tuntap PUBLIC -D${CMAKE_SYSTEM_NAME})

target_include_directories(tuntap PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

set_target_properties(tuntap PROPERTIES
    VERSION 2.2
    POSITION_INDEPENDENT_CODE TRUE
    C_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON)

generate_export_header(tuntap EXPORT_FILE_NAME tuntap-export.h)
target_include_directories(tuntap PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_sources(tuntap PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/tuntap-export.h)

# OS families specific things
# ---------------------------
if(UNIX)
    # Unix specific include directories
    # ---------------------------------
    target_include_directories(tuntap PUBLIC
        /usr/include/
        /usr/local/include/
    )

    # Unix specific definitions
    # -------------------------
    target_compile_definitions(tuntap PUBLIC -DUnix)

    # Unix specific source files
    # --------------------------
    target_sources(tuntap PRIVATE tuntap-unix.c )
endif(UNIX)

if(Windows)
    # Windows specific definitions
    # ----------------------------
    target_compile_definitions(tuntap PUBLIC -DWindows)

    # Windows specific source files
    # -----------------------------
    target_sources(tuntap PRIVATE tuntap-windows.c )
endif(Windows)

# OS specific things
# ------------------
if(UNIX)
    if(Linux)
        target_compile_definitions(tuntap PUBLIC -D_GNU_SOURCE)
        target_sources(tuntap PRIVATE tuntap-unix-linux.c)
    elseif (OpenBSD)
        target_sources(tuntap PRIVATE tuntap-unix-openbsd.c)
        target_sources(tuntap PRIVATE tuntap-unix-bsd.c)
    elseif (NetBSD)
        target_sources(tuntap PRIVATE tuntap-unix-netbsd.c)
        target_sources(tuntap PRIVATE tuntap-unix-bsd.c)
    elseif (FreeBSD)
        target_sources(tuntap PRIVATE tuntap-unix-freebsd.c)
        target_sources(tuntap PRIVATE tuntap-unix-bsd.c)
    elseif (Darwin)
        target_sources(tuntap PRIVATE tuntap-unix-darwin.c)
        target_sources(tuntap PRIVATE tuntap-unix-bsd.c)
    elseif (DragonFly)
        target_sources(tuntap PRIVATE tuntap-unix-freebsd.c)
        target_sources(tuntap PRIVATE tuntap-unix-bsd.c)
    else()
        message(FATAL_ERROR "Your operating system is not supported yet")
    endif()
endif(UNIX)

if(Windows)
    target_link_libraries(tuntap Ws2_32.lib)
endif(Windows)

# C++ Binding definition
# ----------------------
if(ENABLE_CXX)
    add_subdirectory(bindings/cpp/)
endif(ENABLE_CXX)

# Python Binding definition
# -------------------------
if(ENABLE_PYTHON)
    add_subdirectory(bindings/python/)
endif (ENABLE_PYTHON)

# Install rules
# -------------
if(UNIX)
        install(TARGETS tuntap
                LIBRARY DESTINATION lib
                ARCHIVE DESTINATION lib)
        install(FILES tuntap.h ${CMAKE_CURRENT_BINARY_DIR}/tuntap-export.h DESTINATION include)
endif(UNIX)

# Tests rules
# -----------
if (BUILD_TESTING)
    add_subdirectory(regress)
endif ()

