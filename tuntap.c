/*
 * Copyright (c) 2012 Tristan Le Guern <leguern AT medu DOT se>
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

#if defined Windows
# include <winsock2.h>
# include <ws2tcpip.h>
#endif
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "tuntap.h"

struct device *
tuntap_init(void) {
	struct device *dev = NULL;

	if ((dev = (struct device *)malloc(sizeof(*dev))) == NULL)
		return NULL;

	(void)memset(dev->if_name, '\0', sizeof dev->if_name);
	(void)memset(dev->hwaddr, '\0', sizeof dev->hwaddr);
	dev->tun_fd = TUNFD_INVALID_VALUE;
	dev->ctrl_sock = -1;
	dev->flags = 0;

	tuntap_log = tuntap_log_default;
	return dev;
}

void
tuntap_destroy(struct device *dev) {
	tuntap_sys_destroy(dev);
	tuntap_release(dev);
}

char *
tuntap_get_ifname(struct device *dev) {
	return dev->if_name;
}

int
tuntap_set_ip(struct device *dev, const char *addr, int netmask) {
	struct sockaddr_in sin;
	struct sockaddr_in6 sin6;
	uint32_t mask;
	int errval;

	/* Only accept started device */
	if (dev->tun_fd == TUNFD_INVALID_VALUE) {
		tuntap_log(TUNTAP_LOG_NOTICE, "Device is not started");
		return 0;
	}

	if (addr == NULL) {
		tuntap_log(TUNTAP_LOG_ERR, "Invalid parameter 'addr'");
		return -1;
	}

	if (netmask < 0 || netmask > 128) {
		tuntap_log(TUNTAP_LOG_ERR, "Invalid parameter 'netmask'");
		return -1;
	}

	/* Netmask */
	mask = ~0;
	mask = ~(mask >> netmask);
	mask = htonl(mask);

	/*
	 * Destination address parsing: we try IPv4 first and fall back to
	 * IPv6 if inet_pton return 0
	 */
	(void)memset(&sin, '\0', sizeof sin);
	(void)memset(&sin6, '\0', sizeof sin6);

	errval = inet_pton(AF_INET, addr, &(sin.sin_addr));
	if (errval == 1) {
		sin.sin_family = AF_INET;
		return tuntap_sys_set_ipv4(dev, &sin, mask);
	} else if (errval == 0) {
		if (inet_pton(AF_INET6, addr, &(sin6.sin6_addr)) == -1) {
			tuntap_log(TUNTAP_LOG_ERR, "Invalid parameters");
			return -1;
		}
		sin.sin_family = AF_INET6;
		return tuntap_sys_set_ipv6(dev, &sin6, mask);
	} else if (errval == -1) {
		tuntap_log(TUNTAP_LOG_ERR, "Invalid parameters");
		return -1;
	}

	/* NOTREACHED */
	return -1;
}
