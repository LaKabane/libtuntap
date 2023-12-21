#include "tuntap++.hh"

#include <string>
#include <algorithm>
#include <stdexcept>

namespace tuntap {

tuntap::tuntap(int mode, int id)
    : _dev{tuntap_init()}, _started{true}
{
    if (mode != TUNTAP_MODE_ETHERNET && mode != TUNTAP_MODE_TUNNEL) {
        throw std::invalid_argument("Unknown tuntap mode");
    }
    if (id < 0 || id > TUNTAP_ID_MAX) {
        throw std::invalid_argument("Tunnel ID is invalid");
    }
    if (tuntap_start(_dev, mode, id)) {
        throw std::runtime_error("tuntap_start failed");
    }
}

tuntap::~tuntap()
{
    if (_started) {
        tuntap_destroy(_dev);
    }
}

tuntap::tuntap(tuntap &&t)
    : _dev(nullptr)
{
    std::swap(t._dev, this->_dev);
}

void
tuntap::release()
{
    tuntap_release(_dev);
    _started = false;
}

std::string
tuntap::name() const
{
    return std::string(tuntap_get_ifname(_dev));
}

void
tuntap::name(std::string const &s)
{
    if (tuntap_set_ifname(_dev, s.c_str())) {
        throw std::runtime_error("Failed to set ifname");
    }
}

t_tun
tuntap::native_handle() const
{
    return tuntap_get_fd(this->_dev);
}

void
tuntap::up()
{
    if (tuntap_up(_dev)) {
        throw std::runtime_error("Failed to bring up tuntap device");
    }
}

void
tuntap::down()
{
    if (tuntap_down(_dev)) {
        throw std::runtime_error("Failed to bring down tuntap device");
    }
}

int
tuntap::mtu() const
{
    return tuntap_get_mtu(_dev);
}

void
tuntap::mtu(int m)
{
    if (tuntap_set_mtu(_dev, m)) {
        throw std::runtime_error("Failed to set mtu for tuntap device");
    }
}

void
tuntap::ip(std::string const &s, int netmask)
{
    if (tuntap_set_ip(_dev, s.c_str(), netmask)) {
        throw std::runtime_error("Failed to set ip for tuntap device");
    }
}

int
tuntap::read(void *buf, size_t len)
{
	return tuntap_read(_dev, buf, len);
}

int
tuntap::write(void *buf, size_t len)
{
	return tuntap_write(_dev, buf, len);
}

void
tuntap::nonblocking(bool b)
{
    if (tuntap_set_nonblocking(_dev, int(b))) {
        throw std::runtime_error("Failed to change non-blocking state for tuntap device");
    }
}

} /* tuntap */
