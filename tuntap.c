/*
 * Copyright (c) 2012 Tristan Le Guern <tleguern@bouledef.eu>
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

#include <sys/types.h>

#if defined Windows
# include <winsock2.h>
# include <ws2tcpip.h>
#else
# include <netinet/in.h>
# include <arpa/inet.h>
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "tuntap.h"
#include "private.h"

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
tuntap_version(void) {
    return TUNTAP_VERSION;
}

int
tuntap_set_ip(struct device *dev, const char *addr, int netmask) {
	t_tun_in_addr baddr4;
	t_tun_in6_addr baddr6;
	uint32_t mask;

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


	/*
	 * Destination address parsing: we try IPv4 first and fall back to
	 * IPv6 if inet_pton return 0
	 */
	(void)memset(&baddr4, '\0', sizeof baddr4);
	(void)memset(&baddr6, '\0', sizeof baddr6);

	if (inet_pton(AF_INET, addr, &(baddr4)) == 1) {
		/* Netmask */
		if (netmask == 32) {
			mask = htonl(INADDR_NONE);
		} else {
			mask = ~0;
			mask = ~(mask >> netmask);
			mask = htonl(mask);
		}
		return tuntap_sys_set_ipv4(dev, &baddr4, mask);
	} else if (inet_pton(AF_INET6, addr, &(baddr6)) == 1) {
		/* ipv6 prefix no need to convert */
		return tuntap_sys_set_ipv6(dev, &baddr6, netmask);
	} else {
		tuntap_log(TUNTAP_LOG_ERR, "Invalid parameters");
		return -1;
	}

	/* NOTREACHED */
	return -1;
}

t_tun
tuntap_get_fd(struct device *dev) {
	return dev->tun_fd;
}
