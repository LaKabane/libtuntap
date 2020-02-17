# libtuntap java binding CMakeLists.txt
# =====================================

add_library(com_github_smallru8_driver_tuntap_TunTap.h
	    com_github_smallru8_driver_tuntap_TunTap.cpp
)

find_package(JNI REQUIRED)
include_directories(${JAVA_INCLUDE_PATH})
include_directories(${JAVA_INCLUDE_PATH2})

target_sources(tuntap PRIVATE converter.cpp )
