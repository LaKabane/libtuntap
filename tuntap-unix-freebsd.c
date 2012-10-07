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

#include <arpa/inet.h>
#include <net/if.h>
#include <net/if_tun.h>
#include <net/if_types.h>
#include <netinet/if_ether.h>
#include <netinet/in.h>

#include <errno.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "tuntap.h"

int
tuntap_sys_start(struct device *dev, int mode, int tun) {
	int fd;
	int persist;
	char *ifname;
	char name[MAXPATHLEN];
	struct ifaddrs *ifa;
	struct ifreq ifr;

	/* Get the persistence bit */
	if (mode & TUNTAP_MODE_PERSIST) {
		mode &= ~TUNTAP_MODE_PERSIST;
		persist = 1;
	} else {
		persist = 0;
	}

        /* Set the mode: tun or tap */
	if (mode == TUNTAP_MODE_ETHERNET) {
		ifname = "tap";
	} else if (mode == TUNTAP_MODE_TUNNEL) {
		ifname = "tun";
	} else {
		tuntap_log(0, "libtuntap (sys): Invalid mode");
		return -1;
	}

	/* Try to use the given driver or loop throught the avaible ones */
	fd = -1;
	if (tun < TUNTAP_ID_MAX) {
		(void)snprintf(name, sizeof name, "/dev/%s%i", ifname, tun);
		fd = open(name, O_RDWR);
	} else if (tun == TUNTAP_ID_ANY) {
		for (tun = 0; tun < TUNTAP_ID_MAX; ++tun) {
			(void)memset(name, '\0', sizeof name);
			(void)snprintf(name, sizeof name, "/dev/%s%i",
			    ifname, tun);
			if ((fd = open(name, O_RDWR)) > 0)
				break;
		}
	} else {
		tuntap_log(0, "libtuntap (sys): Invalid device unit");
		return -1;
	}
	switch (fd) {
	case -1:
		tuntap_log(0, "libtuntap (sys): Permission denied");
		return -1;
	case 256:
		tuntap_log(0, "libtuntap (sys): Can't find a tun entry");
		return -1;
	default:
		/* NOTREACHED */
		break;
	}

	/* Set the interface name */
	(void)memset(&ifr, '\0', sizeof ifr);
	(void)snprintf(ifr.ifr_name, sizeof ifr.ifr_name, "%s%i", ifname, tun);
	/* And save it */
	(void)strlcpy(dev->if_name, ifr.ifr_name, sizeof ifr.ifr_name);

	/* Get the interface default values */
	if (ioctl(dev->ctrl_sock, SIOCGIFFLAGS, &ifr) == -1) {
		tuntap_log(0, "libtuntap (sys): ioctl SIOCGIFFLAGS");
		return -1;
	}

	/* Save flags for tuntap_{up, down} */
	dev->flags = ifr.ifr_flags;

	/* Save pre-existing MAC address */
	if (mode == TUNTAP_MODE_ETHERNET && getifaddrs(&ifa) == 0) {
		struct ifaddrs *pifa;

		for (pifa = ifa; pifa != NULL; pifa = pifa->ifa_next) {
			if (strcmp(pifa->ifa_name, dev->if_name) == 0) {
				struct ether_addr eth_addr;

				/*
				 * The MAC address is from 10 to 15.
				 *
				 * And yes, I know, the buffer is supposed
				 * to have a size of 14 bytes.
				 */
				(void)memcpy(dev->hwaddr,
				  pifa->ifa_addr->sa_data + 10,
				  ETHER_ADDR_LEN);

				(void)memset(&eth_addr.octet, 0,
				  ETHER_ADDR_LEN);
				(void)memcpy(&eth_addr.octet,
				  pifa->ifa_addr->sa_data + 10,
				  ETHER_ADDR_LEN);

#if 0
				fprintf(stderr,
				  "MAC: %s\n", ether_ntoa(&eth_addr));
#endif
				break;
			}
		}
		freeifaddrs(ifa);
	}

	return fd;
}

void
tuntap_sys_destroy(struct device *dev) {
	return;
}

int
tuntap_sys_set_hwaddr(struct device *dev, struct ether_addr *eth_addr) {
	struct ifreq ifr;

	(void)memset(&ifr, '\0', sizeof ifr);
	(void)strlcpy(ifr.ifr_name, dev->if_name, sizeof dev->if_name);
	ifr.ifr_addr.sa_len = ETHER_ADDR_LEN;
	ifr.ifr_addr.sa_family = AF_LINK;
	(void)memcpy(ifr.ifr_addr.sa_data, eth_addr, ETHER_ADDR_LEN);
	if (ioctl(dev->ctrl_sock, SIOCSIFLLADDR, &ifr) < 0) {
	        tuntap_log(0, "libtuntap (sys): ioctl SIOCSIFLLADDR");
		return -1;
	}
	return 0;
}

int
tuntap_sys_set_ipv4(struct device *dev, struct sockaddr_in *s, uint32_t bits) {
	struct ifreq ifr;
	struct sockaddr_in mask;
	struct sockaddr_in addr;

	(void)memset(&ifr, '\0', sizeof ifr);
	(void)strlcpy(ifr.ifr_name, dev->if_name, sizeof dev->if_name);

	/* Delete previously assigned address */
	(void)ioctl(dev->ctrl_sock, SIOCDIFADDR, &ifr);

	/* Set the address */
	(void)memset(&addr, '\0', sizeof addr);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = s->sin_addr.s_addr;
	addr.sin_len = sizeof addr;
	(void)memcpy(&ifr.ifr_addr, &addr, sizeof addr);

	if (ioctl(dev->ctrl_sock, SIOCSIFADDR, &ifr) == -1) {
		tuntap_log(0, "libtuntap (sys): ioctl SIOCSIFADDR");
		perror("ADDR");
		return -1;
	}

	/* Then set the netmask */
	(void)memset(&mask, '\0', sizeof mask);
	mask.sin_family = AF_INET;
	mask.sin_addr.s_addr = bits;
	mask.sin_len = sizeof mask;
	(void)memcpy(&ifr.ifr_addr, &mask, sizeof ifr.ifr_addr);

	if (ioctl(dev->ctrl_sock, SIOCSIFNETMASK, &ifr) == -1) {
		tuntap_log(0, "libtuntap (sys): ioctl SIOCSIFNETMASK");
		perror("NETMASK");
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

