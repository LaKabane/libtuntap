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
# define LIBTUNTAP_H_

# define TUNTAP_TUNID_MAX 256
# define TUNTAP_TUNID_ANY 257

# define TUNTAP_TUNMODE_ETHERNET 0x0001
# define TUNTAP_TUNMODE_TUNNEL   0x0002
# define TUNTAP_TUNMODE_PERSIST  0x0004

# define TUNTAP_GET_FD(x) (x)->tun_fd

# ifdef __cplusplus
extern "C" {
# endif

struct device {
	int		tun_fd;
	int		ctrl_sock;
	int		flags;     /* ifr.ifr_flags on Unix */
	unsigned char	hwaddr[6];
	char		if_name[IFNAMSIZ];
};

/* User definable log callback */
typedef void (*t_tuntap_log)(int, const char *);
t_tuntap_log tuntap_log;

/* Portable "public" functions */
struct device	*tuntap_init(void);
void		 tuntap_destroy(struct device *);
void		 tuntap_release(struct device *);
int		 tuntap_start(struct device *, int, int);
char		*tuntap_get_ifname(struct device *);
char		*tuntap_get_hwaddr(struct device *);
int		 tuntap_set_hwaddr(struct device *, const char *);
int		 tuntap_up(struct device *);
int		 tuntap_down(struct device *);
int		 tuntap_get_mtu(struct device *);
int		 tuntap_set_mtu(struct device *, int);
int		 tuntap_set_ip(struct device *, const char *, const char *);
int		 tuntap_read(struct device *, void *, size_t);
int		 tuntap_write(struct device *, void *, size_t);

/* Logging functions */
void		 tuntap_log_default(int, const char *);
void		 tuntap_log_hexdump(void *, size_t);
void		 tuntap_log_chksum(void *, int);

/* OS specific functions */
int		 tuntap_sys_start(struct device *, int, int);
void		 tuntap_sys_destroy(struct device *);
int		 tuntap_sys_set_hwaddr(struct device *, struct ether_addr *);
int		 tuntap_sys_set_ip(struct device *, unsigned int, unsigned int);

# ifdef __cplusplus
}
# endif

#endif
