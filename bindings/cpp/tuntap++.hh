#pragma once
#ifndef LIBTUNTAP_ALY0MA60
#define LIBTUNTAP_ALY0MA60

#include <string>

#include <tuntap.h>

namespace tuntap {

class tuntap
{
 public:
  tuntap(int, int = TUNTAP_ID_ANY);
  ~tuntap();
  tuntap(tuntap const &) = delete;
  tuntap & operator = (tuntap const &) = delete;
  tuntap(tuntap &&);

  // Properties
  std::string name() const;
  void name(std::string const &);
  int mtu() const ;
  void mtu(int);
  t_tun native_handle() const;

  // Network
  void up();
  void down();
  void ip(std::string const &presentation, int netmask);

  //IO
  int read(void *buf, size_t len);
  int write(void *buf, size_t len);

  // System
  void release();
  void nonblocking(bool);
 private:
  struct device* _dev;
  bool _started;
};

} /* tuntap */


#endif /* end of include guard: LIBTUNTAP_ALY0MA60 */
