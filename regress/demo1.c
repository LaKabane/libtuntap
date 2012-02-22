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
 * This is a really basic demonstration of libtuntap's usage.
 * It will create a tap driver and pause for 5 seconds.
 * You can specify the number of the tap device and the IP address of
 * the interface.
 */

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include "tun.h"

void
usage(void) {
	fprintf(stderr, "usage: -a addr/netmask -i interface -h");
	exit(1);
}

int
main(int argc, char *argv[]) {
	struct device *dev;
  	int ch;
	char *addr;
	char *netmask;
	char *ptr;
	int interface;
	struct timeval tv;

	interface = TNT_TUNID_ANY;
	addr = "1.2.3.4";
	netmask = "255.255.255.0";
	while ((ch = getopt(argc, argv, "a:i:h:")) != -1) {
		switch (ch) {
		case 'a':
			ptr = strchr(optarg, '/');
			if (ptr == NULL)
			    usage();
			*ptr = '\0';
			ptr++;
			addr = strdup(optarg);
			netmask = strdup(ptr);
			break;
		case 'i':
			interface = atoi(optarg);
			break;
		case 'h':
		default:
			usage();
		}
	}

	dev = tnt_tt_init();
	if (tnt_tt_start(dev, TNT_TUNMODE_ETHERNET, interface) == -1)
	    fprintf(stderr, "Can't start the device\n");
	printf("Interface: %s\n", tnt_tt_get_ifname(dev));

	if (tnt_tt_set_ip(dev, addr, netmask) == -1)
	    fprintf(stderr, "Can't set ip\n");

	if (tnt_tt_up(dev) == -1)
	    fprintf(stderr, "Can't bring interface up\n");

	tv.tv_sec = 5;
	tv.tv_usec = 0;
	select(1, NULL, NULL, NULL, &tv);

	if (tnt_tt_down(dev) == -1)
	    fprintf(stderr, "Can't bring interface down\n");
	tnt_tt_stop(dev);
	tnt_tt_destroy(dev);
	return 0;
}

