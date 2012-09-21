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

/*
 * DeviceIoControl
 */

#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <Winbase.h>

#include "tuntap.h"

void
tuntap_sys_destroy(struct device *dev) {
	(void)dev;
	return;
}

int
tuntap_start(struct device *dev, int mode, int tun) {
	HANDLE tun_fd;

	/* Don't re-initialise a previously started device */
	if (dev->tun_fd != INVALID_HANDLE_VALUE) {
		return -1;
	}

	/* TODO: Get the tap device name */
	tun_fd = CreateFile("blah", GENERIC_WRITE | GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_SYSTEM, 0);
	if (tun_fd == INVALID_HANDLE_VALUE) {
		return -1;
	}

	dev->tun_fd - tun_fd;
	return 0;
}

void
tuntap_release(struct device *dev) {
	(void)closesocket(dev->tun_fd); /* XXX: Really? */
	free(dev);
}

char *
tuntap_get_hwaddr(struct device *dev) {
	/* TAP_IOCTL_GET_MAC */
	return NULL;
}

int
tuntap_set_hwaddr(struct device *dev, const char *hwaddr) {
	return -1;
}

int
tuntap_up(struct device *dev) {
	/* TAP_IOCTL_SET_MEDIA_STATUS */
	return -1;
}

int
tuntap_down(struct device *dev) {
	/* TAP_IOCTL_SET_MEDIA_STATUS */
	return -1;
}

int
tuntap_get_mtu(struct device *dev) {
	/* TAP_IOCTL_GET_MTU */
	return -1;
}

int
tuntap_set_mtu(struct device *dev, int mtu) {
	return -1;
}

int
tuntap_set_ip(struct device *dev, const char *saddr, int bits) {
	return -1;
}

int
tuntap_read(struct device *dev, void *buf, size_t size) {
	return -1;
}

int
tuntap_write(struct device *dev, void *buf, size_t size) {
	return -1;
}

int
tuntap_get_readable(struct device *dev) {
	return -1;
}

int
tuntap_set_nonblocking(struct device *dev, int set) {
	return -1;
}

int
tuntap_set_debug(struct device *dev, int set) {
	return -1;
}

