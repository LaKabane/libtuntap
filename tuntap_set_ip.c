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
tuntap_sys_set_ipv4(struct device *dev, t_tun_in_addr *s4, uint32_t bits) {
#if defined Linux
	struct ifreq ifr;
	struct sockaddr_in mask;

	(void)memset(&ifr, '\0', sizeof ifr);
	(void)memcpy(ifr.ifr_name, dev->if_name, sizeof dev->if_name);

	/* Set the IP address first */
	(void)memcpy(&(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),
	    s4, sizeof(struct in_addr));
	ifr.ifr_addr.sa_family = AF_INET;
	if (ioctl(dev->ctrl_sock, SIOCSIFADDR, &ifr) == -1) {
		tuntap_log(TUNTAP_LOG_ERR, "Can't set IP address");
		return -1;
	}
	
	/* Reinit the struct ifr */
	(void)memset(&ifr.ifr_addr, '\0', sizeof ifr.ifr_addr);

	/* Then set the netmask */
	(void)memset(&mask, '\0', sizeof mask);
	mask.sin_family = AF_INET;
	mask.sin_addr.s_addr = bits;
	(void)memcpy(&ifr.ifr_netmask, &mask, sizeof ifr.ifr_netmask);
	if (ioctl(dev->ctrl_sock, SIOCSIFNETMASK, &ifr) == -1) {
		tuntap_log(TUNTAP_LOG_ERR, "Can't set netmask");
		return -1;
	}

	return 0;
#elif defined OpenBSD
	struct ifaliasreq ifa;
	struct ifreq ifr;
	struct sockaddr_in mask;
	struct sockaddr_in addr;

	(void)memset(&ifa, '\0', sizeof ifa);
	(void)strlcpy(ifa.ifra_name, dev->if_name, sizeof ifa.ifra_name);

	(void)memset(&ifr, '\0', sizeof ifr);
	(void)strlcpy(ifr.ifr_name, dev->if_name, sizeof ifr.ifr_name);

	/* Delete previously assigned address */
	(void)ioctl(dev->ctrl_sock, SIOCDIFADDR, &ifr);

	/*
	 * Fill-in the destination address and netmask,
         * but don't care of the broadcast address
	 */
	(void)memset(&addr, '\0', sizeof addr);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = s4->s_addr;
	addr.sin_len = sizeof addr;
	(void)memcpy(&ifa.ifra_addr, &addr, sizeof addr);

	(void)memset(&mask, '\0', sizeof mask);
	mask.sin_family = AF_INET;
	mask.sin_addr.s_addr = bits;
	mask.sin_len = sizeof mask;
	(void)memcpy(&ifa.ifra_mask, &mask, sizeof mask);

	/* Simpler than calling SIOCSIFADDR and/or SIOCSIFBRDADDR */
	if (ioctl(dev->ctrl_sock, SIOCAIFADDR, &ifa) == -1) {
		tuntap_log(TUNTAP_LOG_ERR, "Can't set IP/netmask");
		return -1;
	}
	return 0;
#elif defined NetBSD
	struct ifaliasreq ifa;
	struct ifreq ifr;
	struct sockaddr_in mask;
	struct sockaddr_in addr;

	(void)memset(&ifa, '\0', sizeof ifa);
	(void)strlcpy(ifa.ifra_name, dev->if_name, sizeof ifa.ifra_name);

	(void)memset(&ifr, '\0', sizeof ifr);
	(void)strlcpy(ifr.ifr_name, dev->if_name, sizeof ifr.ifr_name);

	/* Delete previously assigned address */
	(void)ioctl(dev->ctrl_sock, SIOCDIFADDR, &ifr);

	/*
	 * Fill-in the destination address and netmask,
         * but don't care of the broadcast address
	 */
	(void)memset(&addr, '\0', sizeof addr);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = s4->s_addr;
	addr.sin_len = sizeof addr;
	(void)memcpy(&ifa.ifra_addr, &addr, sizeof addr);

	(void)memset(&mask, '\0', sizeof mask);
	mask.sin_family = AF_INET;
	mask.sin_addr.s_addr = bits;
	mask.sin_len = sizeof mask;
	(void)memcpy(&ifa.ifra_mask, &mask, sizeof ifa.ifra_mask);

	/* Simpler than calling SIOCSIFADDR and/or SIOCSIFBRDADDR */
	if (ioctl(dev->ctrl_sock, SIOCAIFADDR, &ifa) == -1) {
		tuntap_log(TUNTAP_LOG_ERR, "Can't set IP/netmask");
		return -1;
	}
	return 0;
#elif defined FreeBSD || defined DragonFly
	struct ifreq ifr;
	struct sockaddr_in mask;
	struct sockaddr_in addr;

	(void)memset(&ifr, 0, sizeof ifr);
	(void)strlcpy(ifr.ifr_name, dev->if_name, sizeof ifr.ifr_name);

	/* Delete previously assigned address */
	(void)ioctl(dev->ctrl_sock, SIOCDIFADDR, &ifr);

	/* Set the address */
	(void)memset(&addr, 0, sizeof addr);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = s4->s_addr;
	addr.sin_len = sizeof addr;
	(void)memcpy(&ifr.ifr_addr, &addr, sizeof addr);

	if (ioctl(dev->ctrl_sock, SIOCSIFADDR, &ifr) == -1) {
		tuntap_log(TUNTAP_LOG_ERR, "Can't set IP address");
		return -1;
	}

	/* Then set the netmask */
	(void)memset(&mask, '\0', sizeof mask);
	mask.sin_family = AF_INET;
	mask.sin_addr.s_addr = bits;
	mask.sin_len = sizeof mask;
	(void)memcpy(&ifr.ifr_addr, &mask, sizeof ifr.ifr_addr);

	if (ioctl(dev->ctrl_sock, SIOCSIFNETMASK, &ifr) == -1) {
		tuntap_log(TUNTAP_LOG_ERR, "Can't set netmask");
		return -1;
	}
	return 0;
#elif defined Darwin
	struct ifaliasreq ifa;
	struct ifreq ifr;
	struct sockaddr_in addr;
	struct sockaddr_in mask;

	(void)memset(&ifa, '\0', sizeof ifa);
	(void)strlcpy(ifa.ifra_name, dev->if_name, sizeof ifa.ifra_name);

	(void)memset(&ifr, '\0', sizeof ifr);
	(void)strlcpy(ifr.ifr_name, dev->if_name, sizeof ifr.ifr_name);

	/* Delete previously assigned address */
	(void)ioctl(dev->ctrl_sock, SIOCDIFADDR, &ifr);

	/*
	 * Fill-in the destination address and netmask,
         * but don't care of the broadcast address
	 */
	(void)memset(&addr, '\0', sizeof addr);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = s4->s_addr;
	addr.sin_len = sizeof addr;
	(void)memcpy(&ifa.ifra_addr, &addr, sizeof addr);

	(void)memset(&mask, '\0', sizeof mask);
	mask.sin_family = AF_INET;
	mask.sin_addr.s_addr = bits;
	mask.sin_len = sizeof mask;
	(void)memcpy(&ifa.ifra_mask, &mask, sizeof ifa.ifra_mask);

	/* Simpler than calling SIOCSIFADDR and/or SIOCSIFBRDADDR */
	if (ioctl(dev->ctrl_sock, SIOCSIFADDR, &ifa) == -1) {
		tuntap_log(TUNTAP_LOG_ERR, "Can't set IP/netmask");
		return -1;
	}
	return 0;
#else
	tuntap_log(TUNTAP_LOG_NOTICE,
	    "Your system does not support tuntap_set_ip() with IPv4");
	return -1;
#endif
}

static int
tuntap_sys_set_ipv6(struct device *dev, t_tun_in6_addr *s6, uint32_t bits) {
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
	if (ioctl(fd, SIOCSIFADDR, &ifr6) < 0) {
		tuntap_log(TUNTAP_LOG_ERR, "Can't set IP address");
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
	    "Your system does not support tuntap_set_ip() with IPv6");
	return -1;
#endif
}

int
tuntap_set_ip(struct device *dev, const char *addr, int netmask) {
	t_tun_in_addr baddr4;
	t_tun_in6_addr baddr6;
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
	(void)memset(&baddr4, '\0', sizeof baddr4);
	(void)memset(&baddr6, '\0', sizeof baddr6);

	errval = inet_pton(AF_INET, addr, &(baddr4));
	if (errval == 1) {
		return tuntap_sys_set_ipv4(dev, &baddr4, mask);
	} else if (errval == 0) {
		if (inet_pton(AF_INET6, addr, &(baddr6)) == -1) {
			tuntap_log(TUNTAP_LOG_ERR, "Invalid parameters");
			return -1;
		}
		return tuntap_sys_set_ipv6(dev, &baddr6, mask);
	} else if (errval == -1) {
		tuntap_log(TUNTAP_LOG_ERR, "Invalid parameters");
		return -1;
	}

	/* NOTREACHED */
	return -1;
}
