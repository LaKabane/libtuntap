#pragma once
#ifndef LIBTUNTAP_ALY0MA60
#define LIBTUNTAP_ALY0MA60

#if __cplusplus >= 202002L
    #define LIBTUNTAP_ALY0MA60_CONSTEXPR constexpr
#else
    #define LIBTUNTAP_ALY0MA60_CONSTEXPR
#endif

#include <string>
#include <memory>
#include <stdexcept>
#include <cstddef>

#include <tuntap.h>

namespace tuntap {

class tuntap
{
 public:
  tuntap(int, int = TUNTAP_ID_ANY);
  tuntap(tuntap const &) = delete;
  tuntap & operator = (tuntap const &) = delete;
  tuntap(tuntap &&) noexcept;

  // Properties
  std::string name() const noexcept;
  void name(std::string const &);
  int mtu() const noexcept;
  void mtu(int);
  t_tun native_handle() const noexcept;

  // Network
  void up();
  void down();
  void ip(std::string const &presentation, int netmask);

  //IO
  int read(void *buf, std::size_t len) noexcept;
  int write(void *buf, std::size_t len) noexcept;

  // System
  void release() noexcept;
  void nonblocking(bool);
 private:
  class TunTapDestroyer final {
      public:
          LIBTUNTAP_ALY0MA60_CONSTEXPR void operator()(device * dev) const noexcept {
              if (dev)
                  ::tuntap_destroy(dev);
          }
  };
  std::unique_ptr<device, TunTapDestroyer> _dev;
};

} /* tuntap */


#endif /* end of include guard: LIBTUNTAP_ALY0MA60 */
