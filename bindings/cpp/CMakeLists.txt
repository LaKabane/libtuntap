# libtuntap C++ binding CMakeLists.txt
# ====================================

# tuntap++ declaration
# --------------------
add_library(tuntap++)
target_sources(tuntap++ PRIVATE tuntap++.cc
    PUBLIC FILE_SET HEADERS FILES tuntap++.hh)

target_link_libraries(tuntap++ PUBLIC tuntap::tuntap)

set_target_properties(tuntap++ PROPERTIES
                      POSITION_INDEPENDENT_CODE TRUE
                      VERSION 2.2
                      CXX_STANDARD 11)

# install rules
# -----------------------------------
install(TARGETS tuntap++
        EXPORT tuntap-targets
        FILE_SET HEADERS DESTINATION include)