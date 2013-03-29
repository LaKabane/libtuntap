/*
 * Copyright (c) 2013 Tristan Le Guern <leguern AT medu DOT se>
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
 * This is a little demonstration of what you can do with libtuntap.
 *
 * demo1 can be used to create tun and tap device, with a custom device name
 * or description depending on the operationg systems.
 *
 * If requested to create a tap device demo1 will also generate a new random
 * MAC address for this interface.
 *
 * Without the option '-p' this is not really usefull ;-)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <tuntap.h>

void usage(void);
void log_cb(int, const char *);

int
main(int argc, char *argv[]) {
	int type = -1;
	int unit = -1;
	int persist = -1;
	int ch;
	const char *errstr;
	const char *name;
	struct device *dev = NULL;

	while ((ch = getopt(argc, argv, "d:hn:p")) != -1)
		switch (ch) {
		case 'd':
			unit = strtonum(optarg, 0, TUNTAP_ID_MAX, &errstr);
			if (errstr) {
				fprintf(stderr, "error: %s", errstr);
				usage();
			}
			break;
		case 'n':
			if (strcmp(optarg, "tun") == 0) {
				type = TUNTAP_MODE_TUNNEL;
			} else if (strcmp(optarg, "tap") == 0) {
				type = TUNTAP_MODE_ETHERNET;
			} else {
				usage();
			}
			break;
		case 'p':
			persist = 1;
			break;
		case 'h':
		default:
			usage();
		}
	argc -= optind;
	argv += optind;
	if (type == -1)
		type = TUNTAP_MODE_TUNNEL;
	if (unit == -1)
		unit = TUNTAP_ID_ANY;
	if (persist == 1)
		type |= TUNTAP_MODE_PERSIST;

	tuntap_log_set_cb(log_cb);

	dev = tuntap_init();
	if (tuntap_start(dev, type, unit) == -1) {
		return 1;
	}

	(void)tuntap_set_ifname(dev, "demo0");
	(void)tuntap_set_descr(dev, "libtuntap demo interface");

	name = tuntap_get_ifname(dev);
	(void)printf("Created device %s\n", name);

	if (type & TUNTAP_MODE_ETHERNET)
		(void)tuntap_set_hwaddr(dev, "random");
	
	if (type & TUNTAP_MODE_PERSIST) {
		tuntap_release(dev);
	} else {
		tuntap_destroy(dev);
	}
	return 0;
}

void
usage(void) {
	(void)fprintf(stderr, "usage: demo1 [-hp] [-n tap|tun] [-d unit]\n");
	exit(64);
}

void
log_cb(int level, const char *errmsg) {
	const char *prefix = NULL;

	switch (level) {
	case TUNTAP_LOG_DEBUG:
		prefix = "debug";
		break;
	case TUNTAP_LOG_INFO:
		prefix = "info";
		break;
	case TUNTAP_LOG_NOTICE:
		prefix = "notice";
		break;
	case TUNTAP_LOG_WARN:
		prefix = "warn";
		break;
	case TUNTAP_LOG_ERR:
		prefix = "err";
		break;
	default:
		/* NOTREACHED */
		break;
	}
	(void)fprintf(stderr, "%s: %s\n", prefix, errmsg);
}

