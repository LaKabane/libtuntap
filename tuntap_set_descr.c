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

#if defined HAVE_NET_IF_TUN_H
# include <net/if_tun.h>
#endif

#include <stdlib.h>
#include <string.h>

static int
tuntap_sys_set_descr(struct device *dev, const char *descr, size_t len) {
#if defined OpenBSD
	struct ifreq ifr;
	(void)len;

	(void)memset(&ifr, '\0', sizeof ifr);
	(void)strlcpy(ifr.ifr_name, dev->if_name, sizeof ifr.ifr_name);

	ifr.ifr_data = (void *)descr;
	if (ioctl(dev->ctrl_sock, SIOCSIFDESCR, &ifr) == -1) {
		tuntap_log(TUNTAP_LOG_ERR,
		    "Can't set the interface description");
		return -1;
	}
	return 0;
#elif defined FreeBSD
	struct ifreq ifr;
	struct ifreq_buffer ifrbuf;

	(void)memset(&ifr, '\0', sizeof ifr);
	(void)strlcpy(ifr.ifr_name, dev->if_name, sizeof ifr.ifr_name);

	ifrbuf.buffer = (void *)descr;
	ifrbuf.length = len;
	ifr.ifr_buffer = ifrbuf;
	if (ioctl(dev->ctrl_sock, SIOCSIFDESCR, &ifr) == -1) {
		tuntap_log(TUNTAP_LOG_ERR,
		    "Can't set the interface description");
		return -1;
	}
	return 0;
#else /* For now: DragonFlyBSD, Linux and Darwin */
	(void)dev;
	(void)descr;
	(void)len;
	tuntap_log(TUNTAP_LOG_NOTICE,
	    "Your system does not support tuntap_set_descr()");
	return -1;
#endif
}

int
tuntap_set_descr(struct device *dev, const char *descr) {
	size_t len;

	if (descr == NULL) {
		tuntap_log(TUNTAP_LOG_ERR, "Invalid parameter 'descr'");
		return -1;
	}

	len = strlen(descr);
	if (len > IF_DESCRSIZE) {
		/* The value will be troncated */
		tuntap_log(TUNTAP_LOG_WARN, "Parameter 'descr' is too long");
	}

	return tuntap_sys_set_descr(dev, descr, len);
}

