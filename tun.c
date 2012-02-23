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
#include <netinet/ether.h>


#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "tun.h"

#include <err.h>

struct device *
tnt_tt_init(void) {
	struct device *dev = NULL;

	if ((dev = malloc(sizeof(*dev))) == NULL)
		return NULL;

	(void)memset(&(dev->ifr), '\0', sizeof dev->ifr);
	(void)memset(dev->hwaddr, '\0', sizeof dev->hwaddr);
	dev->tun_fd = -1;
	dev->ctrl_sock = -1;
	dev->started = 0;
	return dev;
}

void
tnt_tt_destroy(struct device *dev) {
	(void)memset(&(dev->ifr), '\0', sizeof(dev->ifr));
	/* XXX: SIOCIFDESTROY ? */
	free(dev);
}

int
tnt_tt_start(struct device *dev, int mode, int tun) {
	int sock;
	int fd;

	fd = sock = -1;
	
	if (dev->started == 1) {
		return -1;
	}

	/* OpenBSD needs the control socket to be ready now */
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == -1) {
		goto clean;
	}
	dev->ctrl_sock = sock;

	fd = tnt_tt_sys_start(dev, mode, tun);
	if (fd == -1) {
		goto clean;
	}

	dev->tun_fd = fd;
	dev->started = 1;
	/* XXX: Status */
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
tnt_tt_stop(struct device *dev) {
	if (close(dev->tun_fd) == -1)
		warn("libtt: close dev->tun_fd");
	if (close(dev->ctrl_sock) == -1)
		warn("libtt: close dev->ctrl_sock");
	dev->tun_fd = -1;
	dev->ctrl_sock = -1;
	dev->started = 0;
}

char *
tnt_tt_get_ifname(struct device *dev) {
	/* XXX: Not portable outside of UNIX systems */
	return dev->ifr.ifr_name;
}

char *
tnt_tt_get_hwaddr(struct device *dev) {
	return dev->hwaddr;
}

int
tnt_tt_set_hwaddr(struct device *dev, const char *hwaddr) {
	struct ether_addr *eth_addr;

	eth_addr = ether_aton(hwaddr);
	if (eth_addr == NULL) {
		return -1;
	}

	if (tnt_tt_sys_set_hwaddr(dev, eth_addr) == -1)
		return -1;
	return 0;
}

int
tnt_tt_up(struct device *dev) {
	dev->ifr.ifr_flags |= IFF_UP;
	if (ioctl(dev->ctrl_sock, SIOCSIFFLAGS, &(dev->ifr)) == -1) {
		return -1;
	}
	return 0;
}

int
tnt_tt_down(struct device *dev) {
	dev->ifr.ifr_flags &= ~IFF_UP;
	if (ioctl(dev->ctrl_sock, SIOCSIFFLAGS, &(dev->ifr)) == -1) {
		return -1;
	}
	return 0;
}

int
tnt_tt_get_mtu(struct device *dev) {
	return dev->ifr.ifr_mtu;
}

int
tnt_tt_set_mtu(struct device *dev, int mtu) {
	return tnt_tt_sys_set_mtu(dev, mtu);
}

int
tnt_tt_set_ip(struct device *dev, const char *saddr, const char *smask) {
	struct sockaddr_in sa;
	unsigned int addr;
	unsigned int mask;

	/* Destination address */
	if (inet_pton(AF_INET, saddr, &(sa.sin_addr)) != 1) {
		warn("tnt_tt_set_ip (IPv4)");
		return -1;
	}
	addr = sa.sin_addr.s_addr;

	/* Netmask */
	if (inet_pton(AF_INET, smask, &(sa.sin_addr)) != 1) {
		warn("tnt_tt_set_ip (IPv4)");
		return -1;
	}
	mask = sa.sin_addr.s_addr;

	return tnt_tt_sys_set_ip(dev, addr, mask);
}

