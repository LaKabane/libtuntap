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

#include <stdio.h>
#if defined Windows
# include <windows.h>
# define strcasecmp(x, y) _stricmp((x), (y))
#else
# include <strings.h>
#endif

#include "tuntap.h"

int
main(void) {
	int ret;
	char *hwaddr;
	struct device *dev;

	ret = 0;
	dev = tuntap_init();
	if (tuntap_start(dev, TUNTAP_MODE_ETHERNET, TUNTAP_ID_ANY)
	    == -1) {
		ret = 1;
		goto clean;
	}

	if (tuntap_set_hwaddr(dev, "54:1a:13:ef:b6:b5") == -1) {
		ret = 1;
		goto clean;
	}

	hwaddr = tuntap_get_hwaddr(dev);
	if (strcasecmp(hwaddr, "54:1a:13:ef:b6:b5") != 0)
		ret = 1;

clean:
	tuntap_destroy(dev);
	return ret;
}

