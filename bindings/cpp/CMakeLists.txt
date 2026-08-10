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
configure_file(../../libtuntap++.pc.in ${CMAKE_CURRENT_BINARY_DIR}/libtuntap++.pc @ONLY)

# install rules
# -----------------------------------
install(TARGETS tuntap++
        EXPORT tuntap-targets
        FILE_SET HEADERS DESTINATION include)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libtuntap++.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)
