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

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/param.h> /* For MAXPATHLEN */
#include <sys/socket.h>

#include <net/if.h>
#include <net/if_tun.h>
#include <net/if_types.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <netinet6/in6_var.h>
#include <arpa/inet.h>

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "tuntap.h"

static int
tuntap_sys_create_dev(struct device *dev, int tun) {
	struct ifreq ifr;

	/* At this point 'tun' can't be TUNTAP_ID_ANY */
	(void)memset(&ifr, '\0', sizeof ifr);
	(void)snprintf(ifr.ifr_name, IFNAMSIZ, "tun%i", tun);

	if (ioctl(dev->ctrl_sock, SIOCIFCREATE, &ifr) == -1) {
		tuntap_log(0, "libtuntap (sys): ioctl SIOCIFCREATE");
		return -1;
	}
	return 0;
}

int
tuntap_sys_start(struct device *dev, int mode, int tun) {
	struct ifreq ifr;
	char name[MAXPATHLEN];
	int fd;

	fd = -1;

	/* Force creation of the driver if needed or let it resilient */
	if (mode & TUNTAP_MODE_PERSIST) {
		mode &= ~TUNTAP_MODE_PERSIST;
		if (tuntap_sys_create_dev(dev, tun) == -1)
			return -1;
	}

	/* Try to use the given tun driver or loop throught the avaible ones */
	if (tun < TUNTAP_ID_MAX) {
		(void)snprintf(name, sizeof name, "/dev/tun%i", tun);
		fd = open(name, O_RDWR);
	} else if (tun == TUNTAP_ID_ANY) {
		for (tun = 0; tun < TUNTAP_ID_MAX; ++tun) {
			(void)memset(name, '\0', sizeof name);
			(void)snprintf(name, sizeof name, "/dev/tun%i", tun);
			if ((fd = open(name, O_RDWR)) > 0)
				break;
		}
	} else {
		return -1;
	}

	if (fd < 0 || fd == 256) {
		tuntap_log(0, "libtuntap (sys): Can't find a tun entry");
		return -1;
	}

	/* Set the interface name */
	(void)memset(&ifr, '\0', sizeof ifr);
	(void)snprintf(ifr.ifr_name, sizeof ifr.ifr_name, "tun%i", tun);
	/* And save it */
	(void)strlcpy(dev->if_name, ifr.ifr_name, sizeof ifr.ifr_name);

	/* Get the interface default values */
	if (ioctl(dev->ctrl_sock, SIOCGIFFLAGS, &ifr) == -1) {
		tuntap_log(0, "libtuntap (sys): ioctl SIOCGIFFLAGS");
		return -1;
	}

        /* Set the mode: tun or tap */
	if (mode == TUNTAP_MODE_ETHERNET) {
		ifr.ifr_flags |= IFF_LINK0;
	}
	else if (mode == TUNTAP_MODE_TUNNEL) {
		ifr.ifr_flags &= ~IFF_LINK0;
	}
	else {
		return -1;
	}

	/* Set back our modifications */
	if (ioctl(dev->ctrl_sock, SIOCSIFFLAGS, &ifr) == -1) {
		tuntap_log(0, "libtuntap (sys): ioctl SIOCSIFFLAGS");
		return -1;
	}

	/* Save flags for tuntap_{up, down} */
	dev->flags = ifr.ifr_flags;

	/* Save pre-existing MAC address */
	if (mode == TUNTAP_MODE_ETHERNET) {
		struct ether_addr addr;

		if (ioctl(fd, SIOCGIFADDR, &addr) == -1) {
			tuntap_log(0, "libtuntap (sys): ioctl SIOCGIFADDR");
			return -1;
		}
		(void)memcpy(dev->hwaddr, &addr, 6);
	}
	return fd;
}

void
tuntap_sys_destroy(struct device *dev) {
	struct ifreq ifr;

	(void)memset(&ifr, '\0', sizeof ifr);
	(void)strlcpy(ifr.ifr_name, dev->if_name, sizeof dev->if_name);

	if (ioctl(dev->ctrl_sock, SIOCIFDESTROY, &ifr) == -1)
		tuntap_log(0, "libtuntap (sys): ioctl SIOCIFDESTROY");
}

int
tuntap_sys_set_hwaddr(struct device *dev, struct ether_addr *eth_addr) {
	struct ifreq ifr;

	(void)memset(&ifr, '\0', sizeof ifr);
	(void)strlcpy(ifr.ifr_name, dev->if_name, sizeof dev->if_name);
	ifr.ifr_addr.sa_len = ETHER_ADDR_LEN;
	ifr.ifr_addr.sa_family = AF_LINK;
	(void)memcpy(ifr.ifr_addr.sa_data, eth_addr, ETHER_ADDR_LEN);

	if (ioctl(dev->ctrl_sock, SIOCSIFLLADDR, (caddr_t)&ifr) < 0) {
	        tuntap_log(0, "libtuntap (sys): ioctl SIOCSIFLLADDR");
		return -1;
	}
	return 0;
}

int
tuntap_sys_set_ipv4(struct device *dev, struct sockaddr_in *s4, uint32_t bits) {
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
	addr.sin_addr.s_addr = s4->sin_addr.s_addr;
	addr.sin_len = sizeof addr;
	(void)memcpy(&ifa.ifra_addr, &addr, sizeof addr);

	(void)memset(&mask, '\0', sizeof mask);
	mask.sin_family = AF_INET;
	mask.sin_addr.s_addr = bits;
	mask.sin_len = sizeof mask;
	(void)memcpy(&ifa.ifra_mask, &mask, sizeof mask);

	/* Simpler than calling SIOCSIFADDR and/or SIOCSIFBRDADDR */
	if (ioctl(dev->ctrl_sock, SIOCAIFADDR, &ifa) == -1) {
		tuntap_log(0, "libtuntap (sys): ioctl SIOCAIFADDR");
		return -1;
	}
	return 0;
}

int
tuntap_sys_set_ipv6(struct device *dev, struct sockaddr_in6 *s, uint32_t bits) {
	(void)dev;
	(void)s;
	(void)bits;
	tuntap_log(0, "libtuntap (sys): ipv6 not implemented");
	return -1;
}

