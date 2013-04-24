/*
 * Copyright (c) 2012-2013 Tristan Le Guern <leguern AT medu DOT se>
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

#include <sys/types.h>
#include <sys/ioctl.h>

#if defined Windows
# include <winsock2.h>
# include <ws2tcpip.h>
#else
# include <netinet/in.h>
# include <arpa/inet.h>
#endif

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#include "tuntap_private.h"

#if defined Linux
struct in6_ifreq {
	struct in6_addr	ifr6_addr;
	__u32	ifr6_prefixlen;
	int	ifr6_ifindex;
};
#endif

static int
tuntap_sys_del_ipv4(struct device *dev, t_tun_in_addr *s4, uint32_t bits) {
	(void)dev;
	(void)s4;
	(void)bits;
	tuntap_log(TUNTAP_LOG_NOTICE,
	    "Your system does not support tuntap_del_ip() with IPv4");
	return -1;
}

static int
tuntap_sys_del_ipv6(struct device *dev, t_tun_in6_addr *s6, uint32_t bits) {
#if defined Linux
	struct ifreq ifr;
	struct in6_ifreq ifr6;
	int fd;

	fd = socket(AF_INET6, SOCK_DGRAM, 0);
	if (fd == -1) {
		tuntap_log(TUNTAP_LOG_ERR, "Your system does not support IPv6");
		return -1;
	}

	/* There is no ifr_name for ifr6, so get the interface index */
	(void)memset(&ifr, '\0', sizeof ifr);
	(void)memcpy(ifr.ifr_name, dev->if_name, sizeof dev->if_name);
	if (ioctl(dev->ctrl_sock, SIOGIFINDEX, &ifr) < 0) {
		tuntap_log(TUNTAP_LOG_ERR, "Can't get interface index");
		(void)close(fd);
		return -1;
	}

	ifr6.ifr6_ifindex = ifr.ifr_ifindex;
	ifr6.ifr6_prefixlen = bits;
	(void)memcpy(&ifr6.ifr6_addr, s6, sizeof(struct in6_addr));
	if (ioctl(fd, SIOCDIFADDR, &ifr6) < 0) {
		tuntap_log(TUNTAP_LOG_ERR, "Can't delete IP address");
		(void)close(fd);
		return -1;
	}
	(void)close(fd);
	return 0;
#else
	(void)dev;
	(void)s6;
	(void)bits;
	tuntap_log(TUNTAP_LOG_NOTICE, 
	    "Your system does not support tuntap_del_ip() with IPv6");
	return -1;
#endif
}

int
tuntap_del_ip(struct device *dev, const char *addr, int addrlen) {
	t_tun_in_addr baddr4;
	t_tun_in6_addr baddr6;
	uint32_t v4mask;
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

	if (addrlen < 0 || addrlen > 128) {
		tuntap_log(TUNTAP_LOG_ERR, "Invalid parameter 'addrlen'");
		return -1;
	}

	/* Netmask */
	v4mask = ~0;
	v4mask = ~(v4mask >> addrlen);
	v4mask = htonl(v4mask);

	/*
	 * Destination address parsing: we try IPv4 first and fall back to
	 * IPv6 if inet_pton return 0
	 */
	(void)memset(&baddr4, '\0', sizeof baddr4);
	(void)memset(&baddr6, '\0', sizeof baddr6);

	errval = inet_pton(AF_INET, addr, &(baddr4));
	if (errval == 1) {
		return tuntap_sys_del_ipv4(dev, &baddr4, v4mask);
	} else if (errval == 0) {
		if (inet_pton(AF_INET6, addr, &(baddr6)) == -1) {
			tuntap_log(TUNTAP_LOG_ERR, "Invalid parameters");
			return -1;
		}
		return tuntap_sys_del_ipv6(dev, &baddr6, addrlen);
	} else if (errval == -1) {
		tuntap_log(TUNTAP_LOG_ERR, "Invalid parameters");
		return -1;
	}

	/* NOTREACHED */
	return -1;
}
