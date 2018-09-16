# libtuntap C++ binding CMakeLists.txt
# ====================================

# tuntap++ declaration
# --------------------
add_library(tuntap++ tuntap++.cc tuntap++.hh)

target_include_directories(tuntap++ PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(tuntap++ PUBLIC tuntap)

set_target_properties(tuntap++ PROPERTIES
                      POSITION_INDEPENDENT_CODE TRUE
                      VERSION 2.1
                      CXX_STANDARD 11)

# install rules for shared and static
# -----------------------------------
install(TARGETS tuntap++
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        PUBLIC_HEADER DESTINATION include)

install(FILES tuntap++.hh
        DESTINATION include)
