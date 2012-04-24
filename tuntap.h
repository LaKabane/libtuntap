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
#include <sys/socket.h>

#if defined Linux
# include <linux/if.h>
#else
# include <net/if.h>
#endif
#include <netinet/in.h>
#include <netinet/if_ether.h>

#ifndef LIBTUNTAP_H_
#define LIBTUNTAP_H_

# define TNT_TUNID_MAX 256
# define TNT_TUNID_ANY 257

# define TNT_TUNMODE_ETHERNET 1
# define TNT_TUNMODE_TUNNEL   2

#define TNT_TT_GET_FD(x) (x)->tun_fd

#ifdef __cplusplus
extern "C" {
#endif

/* XXX: flags should probably be a short int */
struct device {
	int		tun_fd;
	int		ctrl_sock;
	int		flags;     /* ifr.ifr_flags on Unix */
	unsigned char	hwaddr[6];
	char		if_name[IFNAMSIZ];
};

struct device	*tnt_tt_init(void);
void		 tnt_tt_destroy(struct device *);
void		 tnt_tt_release(struct device *);
int		 tnt_tt_start(struct device *, int, int);
char		*tnt_tt_get_ifname(struct device *);
char		*tnt_tt_get_hwaddr(struct device *);
int		 tnt_tt_set_hwaddr(struct device *, const char *);
int		 tnt_tt_up(struct device *);
int		 tnt_tt_down(struct device *);
int		 tnt_tt_get_mtu(struct device *);
int		 tnt_tt_set_mtu(struct device *, int);
int		 tnt_tt_set_ip(struct device *, const char *, const char *);

/* OS specific */
int		tnt_tt_sys_start(struct device *, int, int);
void		tnt_tt_sys_destroy(struct device *);
int		tnt_tt_sys_set_hwaddr(struct device *, struct ether_addr *);
int		tnt_tt_sys_set_ip(struct device *, unsigned int, unsigned int);

#ifdef __cplusplus
}
#endif

#endif
