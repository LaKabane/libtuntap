/*
 * Copyright (c) 2012,2013 Tristan Le Guern <leguern AT medu DOT se>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "tuntap.h"
#include "tuntap_private.h"

#include <sys/types.h>
#include <sys/ioctl.h>

#if defined Linux
# include <netinet/ether.h>
#else
# include <netinet/if_ether.h>
#endif

#include <string.h>

static char *
tuntap_sys_get_hwaddr(struct device *dev) {
	struct ether_addr eth_addr;

#if defined OpenBSD
	if (ioctl(fd, SIOCGIFADDR, &eth_addr) == -1) {
		tuntap_log(TUNTAP_LOG_WARN, "Can't get link-layer address");
		return NULL;
	}
#elif defined Linux
	struct ifreq ifr_hw;

	(void)memcpy(ifr_hw.ifr_name, dev->if_name, sizeof(dev->if_name));
	if (ioctl(dev->tun_fd, SIOCGIFHWADDR, &ifr_hw) == -1) {
		tuntap_log(TUNTAP_LOG_WARN, "Can't get link-layer address");
		return NULL;
	}
	(void)memcpy(&eth_addr, ifr_hw.ifr_hwaddr.sa_data, ETHER_ADDR_LEN);
#else
	(void)memcpy(&eth_addr, dev->hwaddr, sizeof dev->hwaddr);
#endif
	return ether_ntoa(&eth_addr);
}

char *
tuntap_get_hwaddr(struct device *dev) {
	if (dev->tun_fd == -1) {
		tuntap_log(TUNTAP_LOG_NOTICE, "Device is not started");
		return 0;
	}

	return tuntap_sys_get_hwaddr(dev);
}

