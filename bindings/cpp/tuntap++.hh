#pragma once
#ifndef LIBTUNTAP_ALY0MA60
#define LIBTUNTAP_ALY0MA60

#if __cplusplus >= 202002L
#define LIBTUNTAP_ALY0MA60_CONSTEXPR constexpr
#else
#define LIBTUNTAP_ALY0MA60_CONSTEXPR
#endif

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>

#include <tuntap.h>

namespace tuntap
{

class tuntap
{
      public:
	tuntap(int, int = TUNTAP_ID_ANY);
	tuntap(tuntap const &) = delete;
	tuntap &operator=(tuntap const &) = delete;
	tuntap(tuntap &&) noexcept;

	// Properties
	std::string name() const noexcept;
	void name(std::string const &);
	std::string hwaddr() const noexcept;
	void hwaddr(std::string const &);
	std::string descr() const noexcept;
	void descr(std::string const &);
	int mtu() const noexcept;
	void mtu(int);
	t_tun native_handle() const noexcept;

	// Network
	void up();
	void down();
	void ip(std::string const &presentation, int netmask);

	// IO
	int read(void *buf, std::size_t len) noexcept;
	int write(void *buf, std::size_t len) noexcept;

	// System
	void release() noexcept;
	void nonblocking(bool);
	void debug(bool);

      private:
	class TunTapDestroyer final
	{
	      public:
		TunTapDestroyer(bool persist = false)
		    : persist(persist)
		{
		}
		LIBTUNTAP_ALY0MA60_CONSTEXPR void operator()(device *dev) const noexcept
		{
			if (dev) {
				if (persist) {
					::tuntap_release(dev);
				} else {
					::tuntap_destroy(dev);
				}
			}
		}

	      private:
		bool persist;
	};
	std::unique_ptr<device, TunTapDestroyer> _dev;
};

} // namespace tuntap

#endif /* end of include guard: LIBTUNTAP_ALY0MA60 */
