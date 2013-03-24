#include "tuntap++.hh"
#include <boost/python.hpp>

BOOST_PYTHON_MODULE(pytuntap)
{
    using namespace tuntap;
    using namespace boost::python;

    std::string (tap::*get_name)() const                = &tap::name;
    void        (tap::*set_name)(std::string const &)   = &tap::name;
    std::string (tap::*get_hwaddr)() const              = &tap::hwaddr;
    void        (tap::*set_hwaddr)(std::string const &) = &tap::hwaddr;
    int         (tap::*get_mtu)() const                 = &tap::mtu;
    void        (tap::*set_mtu)(int)                    = &tap::mtu;

    def("tuntap_version", tuntap_version);

    class_<tun, boost::noncopyable>("Tun", init<>())
        .def("release", &tap::release)
        .def("up", &tap::up)
        .def("down", &tap::down)
        .def("ip", &tap::ip)
        .def("nonblocking", &tap::nonblocking)
        .add_property("name", get_name, set_name)
        .add_property("mtu", set_mtu, set_mtu)
    ;
    class_<tap, boost::noncopyable>("Tap", init<>())
        .def("release", &tap::release)
        .def("up", &tap::up)
        .def("down", &tap::down)
        .def("ip", &tap::ip)
        .def("nonblocking", &tap::nonblocking)
        .add_property("name", get_name, set_name)
        .add_property("hwaddr", set_hwaddr, set_hwaddr)
        .add_property("mtu", set_mtu, set_mtu)
    ;
}
