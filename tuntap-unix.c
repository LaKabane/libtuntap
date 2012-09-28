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
# if !defined Darwin
#  include <net/if_tun.h>
# endif
# include <netinet/in.h>
# include <netinet/if_ether.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "tuntap.h"

#if defined Linux
# define TUNSDEBUG TUNSETDEBUG
#endif

int
tuntap_start(struct device *dev, int mode, int tun) {
	int sock;
	int fd;

	fd = sock = -1;
	
	/* Don't re-initialise a previously started device */
	if (dev->tun_fd != -1) {
		return -1;
	}

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == -1) {
		goto clean;
	}
	dev->ctrl_sock = sock;

	if (mode & TUNTAP_MODE_PERSIST && tun == TUNTAP_ID_ANY) {
		goto clean; /* XXX: Explain why */
	}

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
		unsigned char *ptr;

		i = 0;
		ptr = (unsigned char *)&eth_rand;
		srandom((unsigned int)time(NULL));
		for (; i < sizeof eth_rand; ++i) {
			*ptr = (unsigned char)random();
			ptr++;
		}
		ptr = (unsigned char *)&eth_rand;
		*ptr &= 0xfc;
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
tuntap_set_ip(struct device *dev, const char *saddr, int bits) {
	struct sockaddr_in sin;
	struct sockaddr_in6 sin6;
	uint32_t mask;
	int errval;

	if (saddr == NULL) {
		tuntap_log(0, "libtuntap: tuntap_set_ip invalid address");
		return -1;
	}

	if (bits < 0 || bits > 128) {
		tuntap_log(0, "libtuntap: tuntap_set_ip invalid netmask");
		return -1;
	}

	/* Netmask */
	mask = ~0;
	mask = ~(mask >> bits);
	mask = htonl(mask);

	/*
	 * Destination address parsing: we try IPv4 first and fall back to
	 * IPv6 if inet_pton return 0
	 */
	(void)memset(&sin, '\0', sizeof sin);
	(void)memset(&sin6, '\0', sizeof sin6);

	errval = inet_pton(AF_INET, saddr, &(sin.sin_addr));
	if (errval == 1) {
		sin.sin_family = AF_INET;
		return tuntap_sys_set_ipv4(dev, &sin, mask);
	} else if (errval == 0) {
		if (inet_pton(AF_INET6, saddr, &(sin6.sin6_addr)) == -1) {
			tuntap_log(0, "libtuntap: tuntap_set_ip bad address");
			return -1;
		}
		sin.sin_family = AF_INET6;
		return tuntap_sys_set_ipv6(dev, &sin6, mask);
	} else if (errval == -1) {
		tuntap_log(0, "libtuntap: tuntap_set_ip bad address");
		return -1;
	}

	/* NOTREACHED */
	return -1;
}

int
tuntap_read(struct device *dev, void *buf, size_t size) {
	int n;

	/* Only accept started device */
	if (dev->tun_fd == -1)
		return 0;

	n = read(dev->tun_fd, buf, size);
	if (n == -1) {
		tuntap_log(0, "libtuntap: enable to read from device");
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
		tuntap_log(0, "libtuntap: enable to write to device");
		return -1;
	}
	return n;
}

int
tuntap_get_readable(struct device *dev) {
	int n;

	n = 0;
	if (ioctl(dev->tun_fd, FIONREAD, &n) == -1) {
		tuntap_log(0, "libtuntap (sys): "
		    "your system does not support FIONREAD, fallback to MTU");
		return tuntap_get_mtu(dev);
	}
	return n;
}

int
tuntap_set_nonblocking(struct device *dev, int set) {
	if (ioctl(dev->tun_fd, FIONBIO, &set) == -1) {
		tuntap_log(0, "libtuntap (sys): failed to (un)set nonblocking");
		return -1;
	}
	return 0;
}

int
tuntap_set_debug(struct device *dev, int set) {
#if !defined Darwin
	if (ioctl(dev->tun_fd, TUNSDEBUG, &set) == -1) {
		tuntap_log(0, "libtuntap (sys): failed to (un)set debug");
		return -1;
	}
	return 0;
#else
	return -1;
#endif
}

