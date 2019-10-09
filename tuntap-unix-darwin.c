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

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <sys/errno.h>

#include <arpa/inet.h>
#include <net/if.h>
#include <net/if_types.h>
#include <netinet/if_ether.h>
#include <netinet/in.h>

#include <fcntl.h>
#include <ifaddrs.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "tuntap.h"
#include "private.h"


#include <sys/kern_control.h>
#include <sys/sys_domain.h>
#include <sys/kern_event.h>

#define APPLE_UTUN "com.apple.net.utun_control"
#define UTUN_OPT_IFNAME 2

static int
tuntap_utun_open(char * ifname, uint32_t namesz)
{
	int fd;
	struct sockaddr_ctl addr;
	struct ctl_info info;

	fd = socket(PF_SYSTEM, SOCK_DGRAM, SYSPROTO_CONTROL);
	if(fd == -1)
		return -1;

	memset(&info, 0, sizeof(info));
	strncpy(info.ctl_name, APPLE_UTUN, strlen(APPLE_UTUN));

	if(ioctl(fd, CTLIOCGINFO, &info) < 0) {
		tuntap_log(TUNTAP_LOG_ERR, "call to ioctl() failed");
		tuntap_log(TUNTAP_LOG_ERR, strerror(errno));
		close(fd);
		return -1;
	}

	addr.sc_id = info.ctl_id;

	addr.sc_len = sizeof(addr);
	addr.sc_family = AF_SYSTEM;
	addr.ss_sysaddr = AF_SYS_CONTROL;
	addr.sc_unit = 0;

	if(connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		close(fd);
		return -1;
	}

	if(getsockopt(fd, SYSPROTO_CONTROL, UTUN_OPT_IFNAME, ifname, &namesz) < 0) {
		close(fd);
		return -1;
	}
	return fd;
}



int
tuntap_sys_start(struct device *dev, int mode, int tun) {
	struct ifreq ifr;
	struct ifaddrs *ifa;
	char name[MAXPATHLEN];
	int fd;
	char *type;
	char *ifname;

	fd = -1;

	/* Force creation of the driver if needed or let it resilient */
	if (mode & TUNTAP_MODE_PERSIST) {
		tuntap_log(TUNTAP_LOG_NOTICE, "Your system does not support persistent device");
		return -1;
	}

	/* Set the mode: tun or tap */
	if (mode == TUNTAP_MODE_ETHERNET) {
		type = "tap";
		ifr.ifr_flags |= IFF_LINK0;
	}
	else if (mode == TUNTAP_MODE_TUNNEL) {
		type = "tun";
		ifr.ifr_flags &= ~IFF_LINK0;
	}
	else {
		tuntap_log(TUNTAP_LOG_ERR, "Invalid parameter 'mode'");
		return -1;
	}

	ifname = NULL;

	/* Try to use the given driver or loop throught the avaible ones */
	if (tun < TUNTAP_ID_MAX) {
		(void)snprintf(name, sizeof name, "/dev/%s%i", type, tun);
		fd = open(name, O_RDWR);
	} else if (tun == TUNTAP_ID_ANY) {
		/* Use utun if we can */
		if( mode == TUNTAP_MODE_TUNNEL) {
			fd = tuntap_utun_open(name, IFNAMSIZ);
		}
		if( fd != -1 )
			ifname = name;
		else {
			for (tun = 0; tun < TUNTAP_ID_MAX; ++tun) {
				(void)memset(name, '\0', sizeof name);
				(void)snprintf(name, sizeof name, "/dev/%s%i",
					type, tun);
				if ((fd = open(name, O_RDWR)) > 0)
					break;
			}
		}
	} else {
		tuntap_log(TUNTAP_LOG_ERR, "Invalid parameter 'tun'");
		return -1;
	}
	switch (fd) {
	case -1:
		tuntap_log(TUNTAP_LOG_ERR, "Permission denied");
		return -1;
	case 256:
		tuntap_log(TUNTAP_LOG_ERR, "Can't find a tun entry");
		return -1;
	default:
		/* NOTREACHED */
		break;
	}

	/* Set the interface name */
	(void)memset(&ifr, '\0', sizeof ifr);
	if(ifname)
		(void)strlcpy(ifr.ifr_name, ifname, strnlen(ifname, sizeof ifr.ifr_name));
	else
		(void)snprintf(ifr.ifr_name, sizeof ifr.ifr_name, "%s%i", type, tun);
	/* And save it */
	(void)strlcpy(dev->if_name, ifr.ifr_name, sizeof dev->if_name);

	/* Get the interface default values */
	if (ioctl(dev->ctrl_sock, SIOCGIFFLAGS, &ifr) == -1) {
		tuntap_log(TUNTAP_LOG_ERR, "Can't get interface values");
		return -1;
	}

	/* Set our modifications */
	if (ioctl(dev->ctrl_sock, SIOCSIFFLAGS, &ifr) == -1) {
		tuntap_log(TUNTAP_LOG_ERR, "Can't set interface values");
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

				(void)memset(&eth_addr.ether_addr_octet, 0,
				  ETHER_ADDR_LEN);
				(void)memcpy(&eth_addr.ether_addr_octet,
				  pifa->ifa_addr->sa_data + 10,
				  ETHER_ADDR_LEN);
				break;
			}
		}
		if (pifa == NULL)
			tuntap_log(TUNTAP_LOG_WARN,
			    "Can't get link-layer address");
		freeifaddrs(ifa);
	}
	return fd;
}

void
tuntap_sys_destroy(struct device *dev) {
    (void)dev;
}

int
tuntap_sys_set_hwaddr(struct device *dev, struct ether_addr *eth_addr) {
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
tuntap_sys_set_ipv4(struct device *dev, t_tun_in_addr *s4, uint32_t bits) {
	struct ifaliasreq ifa;
	struct ifreq ifr;
	struct sockaddr_in addr;
	struct sockaddr_in mask;

	(void)memset(&ifa, '\0', sizeof ifa);
	(void)strlcpy(ifa.ifra_name, dev->if_name, sizeof ifa.ifra_name);

	(void)memset(&ifr, '\0', sizeof ifr);
	(void)strlcpy(ifr.ifr_name, dev->if_name, sizeof ifr.ifr_name);

	/* Delete previously assigned address */
	(void)ioctl(dev->ctrl_sock, SIOCDIFADDR, &ifr);

	/*
	 * Fill-in the destination address and netmask,
         * but don't care of the broadcast address
	 */
	(void)memset(&addr, '\0', sizeof addr);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = s4->s_addr;
	addr.sin_len = sizeof addr;
	(void)memcpy(&ifa.ifra_addr, &addr, sizeof addr);

	(void)memset(&mask, '\0', sizeof mask);
	mask.sin_family = AF_INET;
	mask.sin_addr.s_addr = bits;
	mask.sin_len = sizeof mask;
	(void)memcpy(&ifa.ifra_mask, &mask, sizeof ifa.ifra_mask);

	/* Simpler than calling SIOCSIFADDR and/or SIOCSIFBRDADDR */
	if (ioctl(dev->ctrl_sock, SIOCSIFADDR, &ifa) == -1) {
		tuntap_log(TUNTAP_LOG_ERR, "Can't set IP/netmask");
		return -1;
	}
	return 0;
}

int
tuntap_sys_set_descr(struct device *dev, const char *descr, size_t len) {
	tuntap_log(TUNTAP_LOG_NOTICE,
	    "Your system does not support tuntap_set_descr()");
	return -1;
}

char *
tuntap_sys_get_descr(struct device *dev) {
	(void)dev;
	tuntap_log(TUNTAP_LOG_NOTICE,
	    "Your system does not support tuntap_get_descr()");
	return NULL;
}
