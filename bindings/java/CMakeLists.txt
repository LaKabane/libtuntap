# libtuntap java binding CMakeLists.txt
# =====================================

find_package(JNI REQUIRED)
include_directories(${JNI_INCLUDE_DIRS})
include_directories(${JAVA_INCLUDE_PATH})
include_directories(${JAVA_INCLUDE_PATH2})

add_library(tuntap4j
	com_github_smallru8_driver_tuntap_TunTap.h
	com_github_smallru8_driver_tuntap_TunTap.cpp
	converter.cpp
)
target_include_directories(tuntap4j PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/../..
	${CMAKE_CURRENT_BINARY_DIR}/../..
)
target_sources(tuntap4j PRIVATE
	${CMAKE_CURRENT_BINARY_DIR}/../../tuntap-export.h
)

set_target_properties(tuntap4j PROPERTIES
    JAVA_VISIBILITY_PRESET hidden)
