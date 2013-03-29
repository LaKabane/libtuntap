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
#include <sys/socket.h>

#include <unistd.h>

int
tuntap_start(struct device *dev, int mode, int tun) {
	int sock;
	int fd;

	fd = sock = -1;
	
	/* Don't re-initialise a previously started device */
	if (dev->tun_fd != -1) {
		tuntap_log(TUNTAP_LOG_ERR, "Device is already started");
		return -1;
	}

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == -1) {
		goto clean;
	}
	dev->ctrl_sock = sock;

	if (mode & TUNTAP_MODE_PERSIST && tun == TUNTAP_ID_ANY) {
		tuntap_log(TUNTAP_LOG_ERR,
		    "Can't request persistent device on TUNTAP_ID_ANY");
		goto clean; 
	}

	fd = tuntap_sys_start(dev, mode, tun);
	if (fd == -1) {
		goto clean;
	}

	dev->tun_fd = fd;
	if (tuntap_get_debug(dev) == 1)
		tuntap_set_debug(dev, 0);
	return 0;

clean:
	if (fd != -1) {
		(void)close(fd);
	}
	if (sock != -1) {
		(void)close(sock);
	}
	return -1;
}

