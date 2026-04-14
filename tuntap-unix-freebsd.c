/*
 * Copyright (c) 2012 Tristan Le Guern <tleguern@bouledef.eu>
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

#include <sys/ioctl.h>
#include <sys/param.h> /* For MAXPATHLEN */
#include <sys/socket.h>
#include <sys/types.h>

#include <arpa/inet.h>
#include <net/if.h>
#if defined FreeBSD
#include <net/if_tun.h>
#include <net/if_tap.h>
#elif defined DragonFly
#include <net/tun/if_tun.h>
#include <net/tap/if_tap.h>
#endif
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

#include "private.h"
#include "tuntap.h"

static int
tuntap_sys_create_dev(struct device *dev, const char *iftype, int unit)
{
	struct ifreq    ifr;
	char	    	ifname[MAXPATHLEN];

	(void)memset(ifname, 0, sizeof ifname);
	(void)snprintf(ifname, sizeof ifname, "%s%i", iftype, unit);
	(void)memset(&ifr, 0, sizeof ifr);
	(void)strlcpy(ifr.ifr_name, ifname, sizeof ifr.ifr_name);
	if (ioctl(dev->ctrl_sock, SIOCIFCREATE2 ,&ifr) < 0) {
		switch (errno) {
		case EEXIST:
			return 0;
		default:
			return -1;
		}
	}
	return 1;
}

int
tuntap_sys_start(struct device *dev, int mode, int unit)
{
	int		 fd;
	int		 persist;
	char 		 ifname[MAXPATHLEN];
	struct ifreq	 ifr;
	char		*iftype;

	/* Get the persistence bit */
	if (mode & TUNTAP_MODE_PERSIST) {
		mode &= ~TUNTAP_MODE_PERSIST;
		persist = 1;
	} else {
		persist = 0;
	}

	/* Set the mode: tun or tap */
	if (mode == TUNTAP_MODE_ETHERNET) {
		iftype = "tap";
	} else if (mode == TUNTAP_MODE_TUNNEL) {
		iftype = "tun";
	} else {
		tuntap_log(TUNTAP_LOG_ERR, "Invalid parameter 'mode'");
		return -1;
	}

	/* Create the interface if required */
	if (unit < TUNTAP_ID_MAX) {
		if (persist == 1) {
			int result = tuntap_sys_create_dev(dev, iftype, unit);
			switch (result) {
			case -1:
				tuntap_log(TUNTAP_LOG_ERR, "Can't create interface");
				return -1;
			case 0:
				tuntap_log(TUNTAP_LOG_ERR, "Interface already exists");
				return -1;
			default:
				break;
			}
		}
	} else if (unit == TUNTAP_ID_ANY) {
		for (unit = 0; unit < TUNTAP_ID_MAX; unit++) {
			int result = tuntap_sys_create_dev(dev, iftype, unit);
			switch (result) {
			case -1:
				tuntap_log(TUNTAP_LOG_ERR, "Can't create interface");
				return -1;
			case 0:
				continue;
			default:
				goto ifcreated;
			}
		}
	} else {
		tuntap_log(TUNTAP_LOG_ERR, "Invalid parameter 'tun'");
		return -1;
	}

ifcreated:
	/*
	 * Interfaces with custom names are symbolic-link to the "real" tun or
	 * tap interfaces. Opening /dev/tun0 instead of /dev/foobar0 works.
	 */
	(void)memset(ifname, 0, sizeof ifname);
	(void)snprintf(ifname, sizeof ifname, "/dev/%s%i", iftype, unit);
	if ((fd = open(ifname, O_RDWR)) == -1) {
		tuntap_log(TUNTAP_LOG_ERR, "Permission denied");
		return -1;
	}

	/* Get the real interface name */
	(void)memset(&ifr, 0, sizeof ifr);
	if (ioctl(fd, mode == TUNTAP_MODE_ETHERNET ? TAPGIFNAME : TUNGIFNAME, &ifr) == -1) {
		tuntap_log(TUNTAP_LOG_ERR, "Can't get the interface name");
		return -1;
	}
	(void)strlcpy(dev->if_name, ifr.ifr_name, sizeof dev->if_name);

	/* Save the interface default flags for tuntap_{up, down} */
	if (ioctl(dev->ctrl_sock, SIOCGIFFLAGS, &ifr) == -1) {
		tuntap_log(TUNTAP_LOG_ERR, "Can't get interface values");
		return -1;
	}
	dev->flags = ifr.ifr_flags;

	/* Save pre-existing MAC address */
	if (mode == TUNTAP_MODE_ETHERNET) {
		struct ether_addr addr;

		if (ioctl(fd, SIOCGIFADDR, &addr) == -1) {
			tuntap_log(TUNTAP_LOG_WARN, "Can't get link-layer address");
			return fd;
		}
		(void)memcpy(dev->hwaddr, &addr, ETHER_ADDR_LEN);
	}
	return fd;
}

void
tuntap_sys_destroy(struct device *dev)
{
	struct ifreq ifr;

	/* Close tun_fd early otherwise SIOCIFDESTROY will block */
	(void)close(dev->tun_fd);
	dev->tun_fd = -1;
	(void)memset(&ifr, '\0', sizeof ifr);
	(void)strlcpy(ifr.ifr_name, dev->if_name, sizeof ifr.ifr_name);
	if (ioctl(dev->ctrl_sock, SIOCIFDESTROY, &ifr) < 0) {
		tuntap_log(TUNTAP_LOG_WARN, "Can't destroy the interface");
	}
}

int
tuntap_sys_set_hwaddr(struct device *dev, struct ether_addr *eth_addr)
{
	struct ifreq ifr;

	(void)memset(&ifr, '\0', sizeof ifr);
	(void)strlcpy(ifr.ifr_name, dev->if_name, sizeof ifr.ifr_name);
	ifr.ifr_addr.sa_len = ETHER_ADDR_LEN;
	ifr.ifr_addr.sa_family = AF_LINK;
	(void)memcpy(ifr.ifr_addr.sa_data, eth_addr, ETHER_ADDR_LEN);
	if (ioctl(dev->ctrl_sock, SIOCSIFLLADDR, &ifr) < 0) {
		tuntap_log(TUNTAP_LOG_ERR, "Can't set link-layer address");
		return -1;
	}
	return 0;
}

int
tuntap_sys_set_ipv4(struct device *dev, t_tun_in_addr *s4, uint32_t bits)
{
	struct ifreq ifr;
	struct sockaddr_in mask;
	struct sockaddr_in addr;

	(void)memset(&ifr, '\0', sizeof ifr);
	(void)strlcpy(ifr.ifr_name, dev->if_name, sizeof ifr.ifr_name);

	/* Delete previously assigned address */
	(void)ioctl(dev->ctrl_sock, SIOCDIFADDR, &ifr);

	/* Set the address */
	(void)memset(&addr, '\0', sizeof addr);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = s4->s_addr;
	addr.sin_len = sizeof addr;
	(void)memcpy(&ifr.ifr_addr, &addr, sizeof addr);

	if (ioctl(dev->ctrl_sock, SIOCSIFADDR, &ifr) == -1) {
		tuntap_log(TUNTAP_LOG_ERR, "Can't set IP address");
		return -1;
	}

	/* Then set the netmask */
	(void)memset(&mask, '\0', sizeof mask);
	mask.sin_family = AF_INET;
	mask.sin_addr.s_addr = bits;
	mask.sin_len = sizeof mask;
	(void)memcpy(&ifr.ifr_addr, &mask, sizeof ifr.ifr_addr);

	if (ioctl(dev->ctrl_sock, SIOCSIFNETMASK, &ifr) == -1) {
		tuntap_log(TUNTAP_LOG_ERR, "Can't set netmask");
		return -1;
	}
	return 0;
}

int
tuntap_sys_set_ipv6(struct device *dev, t_tun_in6_addr *s6, uint32_t bits)
{
	(void)dev;
	(void)s6;
	(void)bits;
	tuntap_log(TUNTAP_LOG_NOTICE, "IPv6 is not implemented on your system");
	return -1;
}

int
tuntap_sys_set_ifname(struct device *dev, const char *ifname, size_t len)
{
	struct ifreq ifr;

	(void)memset(&ifr, '\0', sizeof ifr);
	(void)strlcpy(ifr.ifr_name, dev->if_name, sizeof(ifr.ifr_name));
	ifr.ifr_data = (caddr_t)ifname;
	if (ioctl(dev->ctrl_sock, SIOCSIFNAME, (caddr_t)&ifr) == -1) {
		tuntap_log(TUNTAP_LOG_ERR, "Can't set interface name");
		return -1;
	}
	(void)strlcpy(dev->if_name, ifr.ifr_name, sizeof(dev->if_name));
	return 0;
}
int
tuntap_sys_set_descr(struct device *dev, const char *descr, size_t len)
{
#if defined FreeBSD
	struct ifreq ifr;
	struct ifreq_buffer ifrbuf;

	(void)memset(&ifr, '\0', sizeof ifr);
	(void)strlcpy(ifr.ifr_name, dev->if_name, sizeof ifr.ifr_name);

	ifrbuf.buffer = (void *)descr;
	ifrbuf.length = len;
	ifr.ifr_buffer = ifrbuf;

	if (ioctl(dev->ctrl_sock, SIOCSIFDESCR, &ifr) == -1) {
		tuntap_log(TUNTAP_LOG_ERR, "Can't set the interface description");
		return -1;
	}
	return 0;
#elif defined DragonFly
	tuntap_log(TUNTAP_LOG_NOTICE, "Your system does not support tuntap_set_descr()");
	return -1;
#endif
}

char *
tuntap_sys_get_descr(struct device *dev)
{
	(void)dev;
	tuntap_log(TUNTAP_LOG_NOTICE, "Your system does not support tuntap_get_descr()");
	return NULL;
}
