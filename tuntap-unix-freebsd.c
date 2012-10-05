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

#include <arpa/inet.h>
#include <net/if.h>
#include <net/if_tun.h>
#include <net/if_types.h>
#include <netinet/if_ether.h>
#include <netinet/in.h>

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "tuntap.h"

static int
tuntap_sys_create_dev(struct device *dev, int tun) {
	return -1;
}

int
tuntap_sys_start(struct device *dev, int mode, int tun) {
	return -1;
}

void
tuntap_sys_destroy(struct device *dev) {
	return -1;
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
tuntap_sys_set_ipv4(struct device *dev, struct sockaddr_in *s4, uint32_t bits) {
	struct ifaliasreq ifa;
	struct ifreq ifr;
	struct sockaddr_in mask;

	(void)memset(&ifa, '\0', sizeof ifa);
	(void)strlcpy(ifa.ifra_name, dev->if_name, sizeof dev->if_name);

	(void)memset(&ifr, '\0', sizeof ifr);
	(void)strlcpy(ifr.ifr_name, dev->if_name, sizeof dev->if_name);

	/* Delete previously assigned address */
	(void)ioctl(dev->ctrl_sock, SIOCDIFADDR, &ifr);

	/*
	 * Fill-in the destination address and netmask,
         * but don't care of the broadcast address
	 */
	(void)memcpy(&(ifa.ifra_addr), s4, sizeof ifa.ifra_addr);

	(void)memset(&mask, '\0', sizeof mask);
	mask.sin_family = AF_INET;
	mask.sin_addr.s_addr = bits;
	mask.sin_len = sizeof mask;
	(void)memcpy(&ifa.ifra_mask, &mask, sizeof ifa.ifra_mask);

	/* Simpler than calling SIOCSIFADDR and/or SIOCSIFBRDADDR */
	if (ioctl(dev->ctrl_sock, SIOCAIFADDR, &ifa) == -1) {
		tuntap_log(0, "libtuntap (sys): ioctl SIOCAIFADDR");
		return -1;
	}
	return 0;
}

int
tuntap_sys_set_ipv6(struct device *dev, struct sockaddr_in6 *s6, uint32_t bits) {
	(void)dev;
	(void)s6;
	(void)bits;
	tuntap_log(0, "libtuntap (sys): ipv6 not implemented");
	return -1;
}

