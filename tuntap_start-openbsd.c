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

#include "tuntap.h"
#include "tuntap_private.h"

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/param.h> /* For MAXPATHLEN */
#include <sys/socket.h>

#include <net/if.h>
#include <net/if_tun.h>
#include <net/if_types.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <netinet6/in6_var.h>
#include <arpa/inet.h>

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int
tuntap_sys_create_dev(struct device *dev, int tun) {
	struct ifreq ifr;

	/* At this point 'tun' can't be TUNTAP_ID_ANY */
	(void)memset(&ifr, '\0', sizeof ifr);
	(void)snprintf(ifr.ifr_name, IF_NAMESIZE, "tun%i", tun);

	if (ioctl(dev->ctrl_sock, SIOCIFCREATE, &ifr) == -1) {
		tuntap_log(TUNTAP_LOG_ERR, "Can't set persistent");
		return -1;
	}
	return 0;
}

int
tuntap_sys_start(struct device *dev, int mode, int tun) {
	int fd;
	char name[MAXPATHLEN];
	struct ifreq ifr;

	/* Get the persistence bit */
	if (mode & TUNTAP_MODE_PERSIST) {
		mode &= ~TUNTAP_MODE_PERSIST;
		/* And force the creation of the driver, if needed */
		if (tuntap_sys_create_dev(dev, tun) == -1)
			return -1;
	}

	/* Try to use the given driver or loop throught the avaible ones */
	fd = -1;
	if (tun < TUNTAP_ID_MAX) {
		(void)snprintf(name, sizeof name, "/dev/tun%i", tun);
		fd = open(name, O_RDWR);
	} else if (tun == TUNTAP_ID_ANY) {
		for (tun = 0; tun < TUNTAP_ID_MAX; ++tun) {
			(void)memset(name, '\0', sizeof name);
			(void)snprintf(name, sizeof name, "/dev/tun%i", tun);
			if ((fd = open(name, O_RDWR)) > 0)
				break;
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
	(void)snprintf(ifr.ifr_name, sizeof ifr.ifr_name, "tun%i", tun);
	/* And save it */
	(void)strlcpy(dev->if_name, ifr.ifr_name, sizeof dev->if_name);

	/* Get the interface default values */
	if (ioctl(dev->ctrl_sock, SIOCGIFFLAGS, &ifr) == -1) {
		tuntap_log(TUNTAP_LOG_ERR, "Can't get interface values");
		return -1;
	}

        /* Set the mode: tun or tap */
	if (mode == TUNTAP_MODE_ETHERNET) {
		ifr.ifr_flags |= IFF_LINK0;
	}
	else if (mode == TUNTAP_MODE_TUNNEL) {
		ifr.ifr_flags &= ~IFF_LINK0;
	}
	else {
		tuntap_log(TUNTAP_LOG_ERR, "Invalid parameter 'mode'");
		return -1;
	}

	/* Set back our modifications */
	if (ioctl(dev->ctrl_sock, SIOCSIFFLAGS, &ifr) == -1) {
		tuntap_log(TUNTAP_LOG_ERR, "Can't set interface values");
		return -1;
	}

	/* Save flags for tuntap_{up, down} */
	dev->flags = ifr.ifr_flags;

	/*
	 * Don't save the pre-existing MAC address,
	 * tuntap_get_hwaddr() do the job on OpenBSD.
	 */
	return fd;
}

