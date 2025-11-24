# libtuntap CMakeLists.txt
# ========================
cmake_minimum_required(VERSION 3.5)

project(libtuntap VERSION 2.2)

include(CTest)
include(CMakePackageConfigHelpers)
include(GenerateExportHeader)

# CMake global options
# --------------------
option(ENABLE_CXX "Enable the C++ wrapper library" ON)

# Tuntap library declaration and portable source files
# --------------------------
add_library(tuntap)
add_library(tuntap::tuntap ALIAS tuntap)

target_sources(tuntap
    PRIVATE tuntap.c private.h tuntap_log.c
    PUBLIC FILE_SET HEADERS FILES tuntap.h)

set_target_properties(tuntap PROPERTIES
    VERSION ${CMAKE_PROJECT_VERSION}
    POSITION_INDEPENDENT_CODE TRUE
    C_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON)

target_compile_definitions(tuntap PUBLIC ${CMAKE_SYSTEM_NAME})
generate_export_header(tuntap EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/tuntap-export.h)
target_sources(tuntap PUBLIC FILE_SET HEADERS FILES tuntap-export.h)

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
    target_compile_definitions(tuntap PUBLIC Unix)

    # Unix specific source files
    # --------------------------
    target_sources(tuntap PRIVATE tuntap-unix.c)
endif(UNIX)

if(Windows)
    # Windows specific definitions
    # ----------------------------
    target_compile_definitions(tuntap PUBLIC Windows)

    # Windows specific source files
    # -----------------------------
    target_sources(tuntap PRIVATE tuntap-windows.c)
endif(Windows)

# OS specific things
# ------------------
if(UNIX)
    if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        target_compile_definitions(tuntap PUBLIC _GNU_SOURCE)
        target_sources(tuntap PRIVATE tuntap-unix-linux.c)
    elseif(${CMAKE_SYSTEM_NAME} STREQUAL "OpenBSD")
        target_sources(tuntap PRIVATE tuntap-unix-openbsd.c)
        target_sources(tuntap PRIVATE tuntap-unix-bsd.c)
    elseif(${CMAKE_SYSTEM_NAME} STREQUAL "NetBSD")
        target_sources(tuntap PRIVATE tuntap-unix-netbsd.c)
        target_sources(tuntap PRIVATE tuntap-unix-bsd.c)
    elseif(${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD")
        target_sources(tuntap PRIVATE tuntap-unix-freebsd.c)
        target_sources(tuntap PRIVATE tuntap-unix-bsd.c)
    elseif(${CMAKE_SYSTEM_NAME} STREQUAL "DragonFly")
        target_sources(tuntap PRIVATE tuntap-unix-freebsd.c)
        target_sources(tuntap PRIVATE tuntap-unix-bsd.c)
    elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
        target_sources(tuntap PRIVATE tuntap-unix-darwin.c)
        target_sources(tuntap PRIVATE tuntap-unix-bsd.c)
    else()
        message(FATAL_ERROR "Your operating system is not supported yet")
    endif()
endif(UNIX)

if(Windows)
    target_link_libraries(tuntap PUBLIC Ws2_32.lib)
endif(Windows)

# C++ Binding definition
# ----------------------
if(ENABLE_CXX)
    add_subdirectory(bindings/cpp)
endif(ENABLE_CXX)

# Install rules
# -------------

configure_package_config_file(tuntap-config.cmake.in tuntap-config.cmake
    INSTALL_DESTINATION lib/cmake/tuntap)

write_basic_package_version_file(tuntap-config-version.cmake
    VERSION ${CMAKE_PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion)

install(TARGETS tuntap
    EXPORT tuntap-targets
    FILE_SET HEADERS DESTINATION include)

install(EXPORT tuntap-targets
    DESTINATION lib/cmake/tuntap
    NAMESPACE tuntap::)

install(FILES
      "${CMAKE_CURRENT_BINARY_DIR}/tuntap-config.cmake"
      "${CMAKE_CURRENT_BINARY_DIR}/tuntap-config-version.cmake"
    DESTINATION lib/cmake/tuntap
)

# Tests rules
# -----------
if (BUILD_TESTING)
    add_subdirectory(regress)
endif ()
