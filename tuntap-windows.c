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
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "tuntap.h"

void
tuntap_sys_destroy(struct device *dev) {
	(void)dev;
	return;
}

int
tuntap_start(struct device *dev, int mode, int tun) {
	return -1;
}

void
tuntap_release(struct device *dev) {
	(void)closesocket(dev->tun_fd);
	(void)closesocket(dev->ctrl_sock);
	free(dev);
}

char *
tuntap_get_hwaddr(struct device *dev) {
	return NULL;
}

int
tuntap_set_hwaddr(struct device *dev, const char *hwaddr) {
	return -1;
}

int
tuntap_up(struct device *dev) {
	return -1;
}

int
tuntap_down(struct device *dev) {
	return -1;
}

int
tuntap_get_mtu(struct device *dev) {
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

