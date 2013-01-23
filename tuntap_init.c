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

#include <stdlib.h>
#include <string.h>

struct device *
tuntap_init(void) {
	struct device *dev = NULL;

	if ((dev = (struct device *)malloc(sizeof(*dev))) == NULL)
		return NULL;

	(void)memset(dev->if_name, '\0', sizeof dev->if_name);
	(void)memset(dev->hwaddr, '\0', sizeof dev->hwaddr);
	dev->tun_fd = TUNFD_INVALID_VALUE;
	dev->ctrl_sock = -1;
	dev->flags = 0;

	tuntap_log = tuntap_log_default;
	return dev;
}

