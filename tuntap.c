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
#if defined Linux
# include <netinet/ether.h>
#else
# include <net/if.h>
# include <netinet/in.h>
# include <netinet/if_ether.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "tuntap.h"

struct device *
tuntap_init(void) {
	struct device *dev = NULL;

	if ((dev = malloc(sizeof(*dev))) == NULL)
		return NULL;

	(void)memset(dev->if_name, '\0', sizeof dev->if_name);
	(void)memset(dev->hwaddr, '\0', sizeof dev->hwaddr);
	dev->tun_fd = -1;
	dev->ctrl_sock = -1;
	dev->flags = 0;
	return dev;
}

void
tuntap_destroy(struct device *dev) {
	tuntap_sys_destroy(dev);
	tuntap_release(dev);
}

int
tuntap_start(struct device *dev, int mode, int tun) {
	int sock;
	int fd;

	fd = sock = -1;
	
	/* Don't re-initialise a previously started device */
	if (dev->tun_fd != -1) {
		return -1;
	}

	/* OpenBSD needs the control socket to be ready now */
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == -1) {
		goto clean;
	}
	dev->ctrl_sock = sock;

	fd = tuntap_sys_start(dev, mode, tun);
	if (fd == -1) {
		goto clean;
	}

	dev->tun_fd = fd;
	return 0;

clean:
	if (fd != -1) {
		(void)close(fd);
	}
	if (sock != -1) {
		(void)close(sock);
	}
	return -1;
}

void
tuntap_release(struct device *dev) {
	(void)close(dev->tun_fd);
	(void)close(dev->ctrl_sock);
	free(dev);
}


char *
tuntap_get_ifname(struct device *dev) {
	return dev->if_name;
}

char *
tuntap_get_hwaddr(struct device *dev) {
	struct ether_addr eth_attr;

	(void)memcpy(&eth_attr, dev->hwaddr, sizeof dev->hwaddr);
	return ether_ntoa(&eth_attr);
}

int
tuntap_set_hwaddr(struct device *dev, const char *hwaddr) {
	struct ether_addr *eth_addr, eth_rand;

	if (strcmp(hwaddr, "random") == 0) {
		unsigned int i;

		i = 0;
		srandom((unsigned int)time(NULL));
		for (; i < sizeof eth_rand.ether_addr_octet; ++i)
			eth_rand.ether_addr_octet[i] = (unsigned char)random();
		eth_rand.ether_addr_octet[0] &= 0xfc;
		eth_addr = &eth_rand;
	} else {
		eth_addr = ether_aton(hwaddr);
		if (eth_addr == NULL) {
			return -1;
		}
	}
	(void)memcpy(dev->hwaddr, eth_addr, 6);

	if (tuntap_sys_set_hwaddr(dev, eth_addr) == -1)
		return -1;
	return 0;
}

int
tuntap_up(struct device *dev) {
	struct ifreq ifr;

	(void)memset(&ifr, '\0', sizeof ifr);
	(void)memcpy(ifr.ifr_name, dev->if_name, sizeof dev->if_name);
	ifr.ifr_flags = (short int)dev->flags;
	ifr.ifr_flags |= IFF_UP;

	if (ioctl(dev->ctrl_sock, SIOCSIFFLAGS, &ifr) == -1) {
		return -1;
	}

	dev->flags = ifr.ifr_flags;
	return 0;
}

int
tuntap_down(struct device *dev) {
	struct ifreq ifr;

	(void)memset(&ifr, '\0', sizeof ifr);
	(void)memcpy(ifr.ifr_name, dev->if_name, sizeof dev->if_name);
	ifr.ifr_flags = (short)dev->flags;
	ifr.ifr_flags &= ~IFF_UP;

	if (ioctl(dev->ctrl_sock, SIOCSIFFLAGS, &ifr) == -1) {
		return -1;
	}

	dev->flags = ifr.ifr_flags;
	return 0;
}

int
tuntap_get_mtu(struct device *dev) {
	struct ifreq ifr;

	/* Only accept started device */
	if (dev->tun_fd == -1)
		return 0;

	(void)memset(&ifr, '\0', sizeof ifr);
	(void)memcpy(ifr.ifr_name, dev->if_name, sizeof dev->if_name);

	if (ioctl(dev->ctrl_sock, SIOCGIFMTU, &ifr) == -1) {
		return -1;
	}
	return ifr.ifr_mtu;
}

int
tuntap_set_mtu(struct device *dev, int mtu) {
	struct ifreq ifr;

	/* Only accept started device */
	if (dev->tun_fd == -1)
		return 0;

	(void)memset(&ifr, '\0', sizeof ifr);
	(void)memcpy(ifr.ifr_name, dev->if_name, sizeof dev->if_name);
	ifr.ifr_mtu = mtu;

	if (ioctl(dev->ctrl_sock, SIOCSIFMTU, &ifr) == -1) {
		return -1;
	}
	return 0;
}

int
tuntap_set_ip(struct device *dev, const char *saddr, const char *smask) {
	struct sockaddr_in sa;
	unsigned int addr;
	unsigned int mask;

	if (saddr == NULL || smask == NULL) {
		(void)fprintf(stderr, "libtuntap: tuntap_set_ip"
		    " invalid argument\n");
		return -1;
	}

	/* Destination address */
	if (inet_pton(AF_INET, saddr, &(sa.sin_addr)) != 1) {
		(void)fprintf(stderr, "libtuntap: tuntap_set_ip (IPv4)"
		    " bad address\n");
		return -1;
	}
	addr = sa.sin_addr.s_addr;

	/* Netmask */
	if (inet_pton(AF_INET, smask, &(sa.sin_addr)) != 1) {
		(void)fprintf(stderr, "libtuntap: tuntap_set_ip (IPv4)"
		    " bad netmask\n");
		return -1;
	}
	mask = sa.sin_addr.s_addr;

	return tuntap_sys_set_ip(dev, addr, mask);
}

int
tuntap_read(struct device *dev, void *buf, size_t size) {
	int n;

	/* Only accept started device */
	if (dev->tun_fd == -1)
		return 0;

	n = read(dev->tun_fd, buf, size);
	if (n == -1) {
		(void)fprintf(stderr,
		    "libtuntap: enable to read from device\n");
		return -1;
	}
	return n;
}

int
tuntap_write(struct device *dev, void *buf, size_t size) {
	int n;

	/* Only accept started device */
	if (dev->tun_fd == -1)
		return 0;

	n = write(dev->tun_fd, buf, size);
	if (n == -1) {
		(void)fprintf(stderr,
		    "libtuntap: enable to write to device\n");
		return -1;
	}
	return n;
}

