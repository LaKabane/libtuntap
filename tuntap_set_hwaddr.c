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
#include "tuntap_private.h"

#include <sys/types.h>
#include <sys/ioctl.h>

#include <netinet/in.h>
#if defined Linux
# include <netinet/ether.h>
#else
# include <netinet/if_ether.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <time.h>

static int
tuntap_sys_set_hwaddr(struct device *dev, struct ether_addr *eth_addr) {
#if defined OpenBSD || defined FreeBSD || defined DragonFly || defined Darwin
	struct ifreq ifr;

	(void)memset(&ifr, '\0', sizeof ifr);
	(void)strlcpy(ifr.ifr_name, dev->if_name, sizeof ifr.ifr_name);
	ifr.ifr_addr.sa_len = ETHER_ADDR_LEN;
	ifr.ifr_addr.sa_family = AF_LINK;
	(void)memcpy(ifr.ifr_addr.sa_data, eth_addr, ETHER_ADDR_LEN);

	if (ioctl(dev->ctrl_sock, SIOCSIFLLADDR, (caddr_t)&ifr) < 0) {
	        tuntap_log(TUNTAP_LOG_ERR, "Can't set link-layer address");
		return -1;
	}
	return 0;
#elif defined NetBSD
	struct ifaliasreq ifra;

	(void)memset(&ifra, 0, sizeof ifra);
	(void)memcpy(ifra.ifra_name, dev->if_name, sizeof dev->if_name);
	ifra.ifra_addr.sa_len = ETHER_ADDR_LEN;
	ifra.ifra_addr.sa_family = AF_LINK;
	(void)memcpy(ifra.ifra_addr.sa_data, eth_addr, ETHER_ADDR_LEN);

	if (ioctl(dev->ctrl_sock, SIOCSIFPHYADDR, &ifra) == -1) {
	        tuntap_log(TUNTAP_LOG_ERR, "Can't set link-layer address");
		return -1;
	}
	return 0;
#elif defined Linux
	struct ifreq ifr;

	(void)memset(&ifr, '\0', sizeof ifr);
	(void)memcpy(ifr.ifr_name, dev->if_name, sizeof dev->if_name);

	ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
	(void)memcpy(ifr.ifr_hwaddr.sa_data, eth_addr->ether_addr_octet, 6);

	/* Linux has a special flag for setting the MAC address */
	if (ioctl(dev->ctrl_sock, SIOCSIFHWADDR, &ifr) == -1) {
	        tuntap_log(TUNTAP_LOG_ERR, "Can't set link-layer address");
		return -1;
	}
	return 0;
#else
	(void)dev;
	(void)eth_addr;
	tuntap_log(TUNTAP_LOG_NOTICE,
	    "Your system does not support tuntap_set_hwaddr()");
	return -1;
#endif
}

int
tuntap_set_hwaddr(struct device *dev, const char *hwaddr) {
	struct ether_addr *eth_addr, eth_rand;

	if (strcmp(hwaddr, "random") == 0) {
		unsigned int i;
		unsigned char *ptr;

		ptr = (unsigned char *)&eth_rand;
		srandom((unsigned int)time(NULL));
		for (i = 0; i < sizeof eth_rand; ++i) {
			*ptr = (unsigned char)random();
			ptr++;
		}
		ptr = (unsigned char *)&eth_rand;
		ptr[0] <<= 1;
		ptr[0] |= 1;
		ptr[0] <<= 1;
		eth_addr = &eth_rand;
	} else {
		eth_addr = ether_aton(hwaddr);
		if (eth_addr == NULL) {
			return -1;
		}
	}
	(void)memcpy(dev->hwaddr, eth_addr, ETHER_ADDR_LEN);

	return tuntap_sys_set_hwaddr(dev, eth_addr);
}

