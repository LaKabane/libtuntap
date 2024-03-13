#include "tuntap++.hh"

#include <string>
#include <algorithm>
#include <stdexcept>

namespace tuntap {

tuntap::tuntap(int mode, int id)
    : _dev{::tuntap_init()}
{
    if (mode != TUNTAP_MODE_ETHERNET && mode != TUNTAP_MODE_TUNNEL) {
        throw std::invalid_argument("Unknown tuntap mode");
    }
    if (id < 0 || id > TUNTAP_ID_MAX) {
        throw std::invalid_argument("Tunnel ID is invalid");
    }
    if (::tuntap_start(_dev.get(), mode, id)) {
        throw std::runtime_error("tuntap_start failed");
    }
}

tuntap::tuntap(tuntap &&t) noexcept
    : _dev()
{
    t._dev.swap(this->_dev);
}

void
tuntap::release() noexcept
{
    _dev.release();
}

std::string
tuntap::name() const noexcept
{
    return std::string(::tuntap_get_ifname(_dev.get()));
}

void
tuntap::name(std::string const &s)
{
    if (::tuntap_set_ifname(_dev.get(), s.c_str())) {
        throw std::runtime_error("Failed to set ifname");
    }
}

t_tun
tuntap::native_handle() const noexcept
{
    return ::tuntap_get_fd(_dev.get());
}

void
tuntap::up()
{
    if (::tuntap_up(_dev.get())) {
        throw std::runtime_error("Failed to bring up tuntap device");
    }
}

void
tuntap::down()
{
    if (::tuntap_down(_dev.get())) {
        throw std::runtime_error("Failed to bring down tuntap device");
    }
}

int
tuntap::mtu() const noexcept
{
    return ::tuntap_get_mtu(_dev.get());
}

void
tuntap::mtu(int m)
{
    if (m < 1 || m > 65535) {
        throw std::invalid_argument("Invalid mtu");
    }
    if (::tuntap_set_mtu(_dev.get(), m)) {
        throw std::runtime_error("Failed to set mtu for tuntap device");
    }
}

void
tuntap::ip(std::string const &s, int netmask)
{
    if (netmask > 128) {
        throw std::invalid_argument("Invalid netmask");
    }
    if (::tuntap_set_ip(_dev.get(), s.c_str(), netmask)) {
        throw std::runtime_error("Failed to set ip for tuntap device");
    }
}

int
tuntap::read(void *buf, size_t len) noexcept
{
	return ::tuntap_read(_dev.get(), buf, len);
}

int
tuntap::write(void *buf, size_t len) noexcept
{
	return ::tuntap_write(_dev.get(), buf, len);
}

void
tuntap::nonblocking(bool b)
{
    if (::tuntap_set_nonblocking(_dev.get(), static_cast<int>(b))) {
        throw std::runtime_error("Failed to change non-blocking state for tuntap device");
    }
}

} /* tuntap */
