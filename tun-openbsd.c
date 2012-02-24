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
#include <sys/param.h> /* For MAXPATHLEN */

#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <net/if_types.h>
#include <net/if_tun.h>
#include <netinet/if_ether.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "tun.h"

#include <err.h>
#include <errno.h>

int
tnt_tt_sys_start(struct device *dev, int mode, int tun) {
	char name[MAXPATHLEN];
	int fd;

	fd = -1;
	/*
	 * Try to use the given tun driver, or loop throught the avaible ones
	 */
	if (tun < TNT_TUNID_MAX) {
		(void)snprintf(name, sizeof name, "/dev/tun%i", tun);
		fd = open(name, O_RDWR);
	} else if (tun == TNT_TUNID_ANY) {
		for (tun = 0; tun < TNT_TUNID_MAX; ++tun) {
			(void)memset(name, '\0', sizeof name);
			(void)snprintf(name, sizeof name, "/dev/tun%i", tun);
			if ((fd = open(name, O_RDWR)) > 0)
				break;
		}
	} else {
		return -1;
	}

	if (fd < 0 || fd == 256) {
		warnx("libtt: Can't find a tun entry");
		return -1;
	}

	/* Set the interface name */
	snprintf(dev->ifr.ifr_name, sizeof(dev->ifr.ifr_name),
	    "tun%i", tun);

	/* Get the interface default values */
	if (ioctl(dev->ctrl_sock, SIOCGIFFLAGS, &(dev->ifr)) == -1) {
		warn("libtt: ioctl SIOCGIFFLAGS");
		return -1;
	}

        /* Set the mode: tun or tap */
	if (mode == TNT_TUNMODE_ETHERNET) {
		dev->ifr.ifr_flags |= IFF_LINK0;
	}
	else if (mode == TNT_TUNMODE_TUNNEL) {
		dev->ifr.ifr_flags &= ~IFF_LINK0;
	}
	else {
		return -1;
	}

	/* Set back our modifications */
	if (ioctl(dev->ctrl_sock, SIOCSIFFLAGS, &(dev->ifr)) == -1) {
		warn("libtt: ioctl SIOCSIFFLAGS");
		return -1;
	}

	return fd;
}

int
tnt_tt_sys_set_hwaddr(struct device *dev, const char *hwaddr) {
	struct ether_addr *eth_addr;

	eth_addr = ether_aton(hwaddr);
	if (eth_addr == NULL) {
		return -1;
	}

	if (ioctl(dev->ctrl_sock, SIOCSIFADDR, eth_addr) == -1) {
		perror(NULL);
		return -1;
	}
	return 0;
}

int
tnt_tt_sys_set_ip(struct device *dev, int iaddr, int imask) {
	struct ifaliasreq ifa;
	struct sockaddr_in addr;
	struct sockaddr_in mask;

	memset(&ifa, '\0', sizeof ifa);
	strcpy(ifa.ifra_name, dev->ifr.ifr_name);

	/* Delete previously assigned address */
	if (ioctl(dev->ctrl_sock, SIOCDIFADDR, &(dev->ifr)) == -1) {
		warnx("libtt: ioctl SIOCDIFADDR");
		return -1;
	}

	/*
	 * Fill-in the destination address and netmask,
         * but don't care of the broadcast address
	 */
	memset(&addr, '\0', sizeof addr);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = iaddr;
	addr.sin_len = sizeof addr;
	memcpy(&(ifa.ifra_addr), &addr, sizeof ifa.ifra_addr);

	memset(&mask, '\0', sizeof mask);
	mask.sin_family = AF_INET;
	mask.sin_addr.s_addr = imask;
	mask.sin_len = sizeof mask;
	memcpy(&ifa.ifra_mask, &mask, sizeof ifa.ifra_mask);

	/* Simpler than calling SIOCSIFADDR and/or SIOCSIFBRDADDR */
	if (ioctl(dev->ctrl_sock, SIOCAIFADDR, &ifa) == -1) {
		warn("libtt (sys): ioctl SIOCAIFADDR");
		return -1;
	}
	return 0;
}

