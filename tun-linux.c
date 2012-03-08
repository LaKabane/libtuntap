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
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <netinet/if_ether.h>
#include <net/if_arp.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "tun.h"

#include <err.h>

int
tnt_tt_sys_start(struct device *dev, int mode, int tun) {
	int fd;
	char *ifname;
	struct ifreq ifr;

	fd = -1;
	if ((fd = open("/dev/net/tun", O_RDWR)) == -1) {
		warn("libtt (sys): open /dev/net/tun");
		return -1;
	}

	(void)memset(&ifr, '\0', sizeof ifr);

        /* Set the mode: tun or tap */
	if (mode == TNT_TUNMODE_ETHERNET) {
		ifr.ifr_flags = IFF_TAP;
		ifname = "tap%i";
	} else if (mode == TNT_TUNMODE_TUNNEL) {
		ifr.ifr_flags = IFF_TUN;
		ifname = "tun%i";
	} else {
		return -1;
	}
	ifr.ifr_flags |= IFF_NO_PI;

	/* Configure the interface */
	if (ioctl(fd, TUNSETIFF, &ifr) == -1) {
		warn("libtt (sys): ioctl TUNSETIFF");
		return -1;
	}

	/* Set the interface name, if any */
	if (tun != TNT_TUNID_ANY) {
		if (fd > TNT_TUNID_MAX) {
			return -1;
		}
		(void)snprintf(ifr.ifr_name, sizeof ifr.ifr_name,
		    ifname, tun);
	}

	/* Get the internal parameters of ifr */
	if (ioctl(dev->ctrl_sock, SIOCGIFFLAGS, &ifr) == -1) {
		warn("ioctl SIOCGIFFLAGS");
	    	return -1;
	}

	/* Save flags for tnt_tt_{up, down} */
	dev->flags = ifr.ifr_flags;

	/* Save the interface name */
	(void)strlcpy(dev->if_name, ifr.ifr_name,
	    sizeof ifr.ifr_name);
	return fd;
}

void
tnt_tt_sys_destroy(struct device *dev) {
	/* Linux automatically remove unused interface */
	(void)dev;
}

int
tnt_tt_sys_set_hwaddr(struct device *dev, struct ether_addr *eth_addr) {
	struct ifreq ifr;

	(void)memset(&ifr, '\0', sizeof ifr);
	(void)strlcpy(ifr.ifr_name, dev->if_name, sizeof dev->if_name);

	ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
	(void)memcpy(ifr.ifr_hwaddr.sa_data, eth_addr->ether_addr_octet, 6);

	/* Linux has a special flag for setting the MAC address */
	if (ioctl(dev->ctrl_sock, SIOCSIFHWADDR, &ifr) == -1) {
		warn("libtt (sys): ioctl SIOCSIFHWADDR");
		return -1;
	}
	return 0;
}

int
tnt_tt_sys_set_ip(struct device *dev, unsigned int iaddr, unsigned int imask) {
	struct sockaddr_in addr;
	struct ifreq ifr;

	(void)memset(&ifr, '\0', sizeof ifr);
	(void)strlcpy(ifr.ifr_name, dev->if_name, sizeof dev->if_name);

	/* Linux doesn't have SIOCDIFADDR, so let just do two calls */
	(void)memset(&addr, '\0', sizeof addr);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = iaddr;
	(void)memcpy(&ifr.ifr_addr, &addr, sizeof ifr.ifr_addr);
	if (ioctl(dev->ctrl_sock, SIOCSIFADDR, &ifr) == -1) {
		warn("libtt (sys): ioctl SIOCSIFADDR");
		return -1;
	}
	
	/* Reinit the struct ifr */
	(void)memset(&ifr.ifr_addr, '\0', sizeof ifr.ifr_addr);

	/* Netmask */
	(void)memset(&addr, '\0', sizeof addr);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = imask;
	(void)memcpy(&ifr.ifr_netmask, &addr, sizeof ifr.ifr_netmask);
	if (ioctl(dev->ctrl_sock, SIOCSIFNETMASK, &ifr) == -1) {
		warn("libtt (sys): ioctl SIOCSIFNETMASK");
		return -1;
	}

	return 0;
}

