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

	fd = -1;
	if ((fd = open("/dev/net/tun", O_RDWR)) == -1) {
		warn("libtt (sys): open /dev/net/tun");
		return -1;
	}

	(void)memset(&(dev->ifr), '\0', sizeof dev->ifr);

        /* Set the mode: tun or tap */
	if (mode == TNT_TUNMODE_ETHERNET) {
		dev->ifr.ifr_flags = IFF_TAP;
		ifname = "tap%i";
	} else if (mode == TNT_TUNMODE_TUNNEL) {
		dev->ifr.ifr_flags = IFF_TUN;
		ifname = "tun%i";
	} else {
		return -1;
	}
	dev->ifr.ifr_flags |= IFF_NO_PI;

	/* Configure the interface */
	if (ioctl(fd, TUNSETIFF, &(dev->ifr)) == -1) {
		warn("libtt (sys): ioctl TUNSETIFF");
		return -1;
	}

	/* Set the interface name, if any */
	/* XXX: Is this really necessary ? */
	if (tun != TNT_TUNID_ANY) {
		if (fd > TNT_TUNID_MAX) {
			return -1;
		}
		snprintf(dev->ifr.ifr_name, sizeof(dev->ifr.ifr_name),
		    ifname, tun);
	}

	/* Get the internal parameters of ifr */
	if (ioctl(dev->ctrl_sock, SIOCGIFFLAGS, &(dev->ifr)) == -1) {
		warn("ioctl SIOCGIFFLAGS");
	    	return -1;
	}

	return fd;
}

int
tnt_tt_sys_set_hwaddr(struct device *dev, const char *hwaddr) {
	warnx("libtt (sys): tnt_tt_sys_set_hwaddr: not implemented");
	return -1;
}

int
tnt_tt_sys_set_ip(struct device *dev, int iaddr, int imask) {
	struct sockaddr_in addr;
	struct sockaddr_in mask;

	/* Linux doesn't have SIOCDIFADDR, so let just do two calls */
	memset(&addr, '\0', sizeof addr);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = iaddr;
	memcpy(&dev->ifr.ifr_addr, &addr, sizeof dev->ifr.ifr_addr);
	if (ioctl(dev->ctrl_sock, SIOCSIFADDR, &dev->ifr) == -1) {
		warn("libtt (sys): ioctl SIOCSIFADDR");
		return -1;
	}
	
	memset(&addr, '\0', sizeof addr);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = imask;
	memcpy(&dev->ifr.ifr_netmask, &addr, sizeof dev->ifr.ifr_netmask);
	if (ioctl(dev->ctrl_sock, SIOCSIFNETMASK, &dev->ifr) == -1) {
		warn("libtt (sys): ioctl SIOCSIFNETMASK");
		return -1;
	}

	return 0;
}

