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

#include <stdlib.h>
#include <string.h>

static int
tuntap_sys_set_ifname(struct device *dev, const char *ifname, size_t len) {
#if defined Linux
	struct ifreq ifr;

	(void)strncpy(ifr.ifr_name, dev->if_name, IF_NAMESIZE);
	(void)strncpy(ifr.ifr_newname, ifname, len);

	if (ioctl(dev->ctrl_sock, SIOCSIFNAME, &ifr) == -1) {
		tuntap_log(TUNTAP_LOG_ERR, "Can't set interface name");
		return -1;
	}
	return 0;
#elif defined FreeBSD
	struct ifreq ifr;

	(void)strncpy(ifr.ifr_name, dev->if_name, IF_NAMESIZE);
	(void)strncpy(ifr.ifr_data, ifname, len);

	if (ioctl(dev->ctrl_sock, SIOCSIFNAME, &ifr) == -1) {
		tuntap_log(TUNTAP_LOG_ERR, "Can't set interface name");
		perror(NULL);
		return -1;
	}
	return 0;
#else
	(void)dev;
	(void)ifname;
	(void)len;
	tuntap_log(TUNTAP_LOG_NOTICE,
	    "Your system does not support tuntap_set_ifname()");
	return -1;
#endif
}

int
tuntap_set_ifname(struct device *dev, const char *ifname) {
	size_t len;

	if (ifname == NULL) {
		tuntap_log(TUNTAP_LOG_ERR, "Invalid parameter 'ifname'");
		return -1;
	}

	len = strlen(ifname);
	if (len > IF_NAMESIZE) {
		tuntap_log(TUNTAP_LOG_ERR, "Parameter 'ifname' is too long");
		return -1;
	}

	if (tuntap_sys_set_ifname(dev, ifname, len) == -1) {
		return -1;
	}

	(void)memset(dev->if_name, 0, IF_NAMESIZE);
	(void)strncpy(dev->if_name, ifname, len);
	return 0;
}

