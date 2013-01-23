/*
 * Copyright (c) 2012,2013 Tristan Le Guern <leguern AT medu DOT se>
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

#include <sys/ioctl.h>

#if defined Linux
# include <linux/if_tun.h>
#elif defined DragonFly
# include <net/tun/if_tun.h>
#elif !defined Darwin
# include <net/if_tun.h>
#endif

#include <string.h>

static void
tuntap_sys_destroy(struct device *dev) {
#if defined SIOCIFDESTROY /* For now: OpenBSD, FreeBSD, NetBSD, DragonFlyBSD */
	struct ifreq ifr;

	(void)memset(&ifr, '\0', sizeof ifr);
	(void)strlcpy(ifr.ifr_name, dev->if_name, sizeof ifr.ifr_name);

	if (ioctl(dev->ctrl_sock, SIOCIFDESTROY, &ifr) == -1) {
		tuntap_log(TUNTAP_LOG_WARN, "Can't destroy the interface");
	}
#elif defined TUNSETPERSIST /* For now: Linux */
	if (ioctl(dev->tun_fd, TUNSETPERSIST, 0) == -1) {
		tuntap_log(TUNTAP_LOG_WARN, "Can't destroy the interface");
	}
#else /* Windows */
	tuntap_log(TUNTAP_LOG_NOTICE,
	    "Your system does not support tuntap_destroy()");
#endif
}

void
tuntap_destroy(struct device *dev) {
	tuntap_sys_destroy(dev);
	tuntap_release(dev);
}

