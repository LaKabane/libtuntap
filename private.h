/*
 * Copyright (c) 2018 Tristan Le Guern <tleguern AT bouledef DOT eu>
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

#if defined Windows
# include <In6addr.h>
# include <stdint.h>
#else /* Unix */
# include <sys/socket.h>
#endif

#if !defined Windows /* Unix :) */
# if defined Linux
#  include <linux/if.h>
# else
#  include <net/if.h>
# endif
# include <netinet/in.h>
# include <netinet/if_ether.h>
#endif

#ifndef PRIVATE_H_
# define PRIVATE_H_

/*
 * Uniformize macros
 * - ETHER_ADDR_LEN: Magic number from IEEE 802.3
 * - IF_NAMESIZE: Length of interface external name
 * - IF_DESCRSIZE: Length of interface description
 * - TUNSDEBUG: ioctl flag to enable the debug mode of a tun device
 * - TUNFD_INVALID_VALUE: Invalid value for tun_fd
 */

# if defined IFNAMSIZ && !defined IF_NAMESIZE
#  define IF_NAMESIZE IFNAMSIZ /* Historical BSD name */
# elif !defined IF_NAMESIZE
#  define IF_NAMESIZE 16
# endif

# if defined ETH_ALEN /* Linux */
#  define ETHER_ADDR_LEN ETH_ALEN
# elif defined Windows
#  define ETHER_ADDR_LEN 6 
# endif

# if defined IFDESCRSIZE
#  define IF_DESCRSIZE IFDESCRSIZE /* OpenBSD */
# else
#  define IF_DESCRSIZE 1024 /* FreeBSD /sys/net/if.c, ifdescr_maxlen */
# endif

# if defined TUNSETDEBUG
#  define TUNSDEBUG TUNSETDEBUG
# endif

# if defined Windows
#  define TUNFD_INVALID_VALUE INVALID_HANDLE_VALUE
# else /* Unix */
#  define TUNFD_INVALID_VALUE -1
# endif

/*
 * Uniformize types
 * - t_tun: tun device file descriptor
 * - t_tun_in_addr: struct in_addr/IN_ADDR
 * - t_tun_in6_addr: struct in6_addr/IN6_ADDR
 */
# if defined Windows
typedef IN_ADDR t_tun_in_addr;
typedef IN6_ADDR t_tun_in6_addr;
# else /* Unix */
typedef struct in_addr t_tun_in_addr;
typedef struct in6_addr t_tun_in6_addr;
# endif

struct device {
	t_tun		tun_fd;
	int		ctrl_sock;
	int		flags;     /* ifr.ifr_flags on Unix */
	unsigned char	hwaddr[ETHER_ADDR_LEN];
	char		if_name[IF_NAMESIZE + 1];
};

/*
 * Windows helpers
 */
# if defined Windows
#  define snprintf(x, y, z, ...) _snprintf_s((x), (y), (y), (z), __VA_ARGS__);
#  define strncat(x, y, z) strncat_s((x), _countof(x), (y), (z));
#  define strdup(x) _strdup(x)
# endif

/* Internal log facilities */
extern t_tuntap_log tuntap_log;
void	 tuntap_log_default(int, const char *);
void	 tuntap_log_hexdump(void *, size_t);
void	 tuntap_log_chksum(void *, int);

/* OS specific functions */
int	 tuntap_sys_start(struct device *, int, int);
void	 tuntap_sys_destroy(struct device *);
int	 tuntap_sys_set_hwaddr(struct device *, struct ether_addr *);
int	 tuntap_sys_set_ipv4(struct device *, t_tun_in_addr *, uint32_t);
int	 tuntap_sys_set_ipv6(struct device *, t_tun_in6_addr *, uint32_t);
int	 tuntap_sys_set_ifname(struct device *, const char *, size_t);
int	 tuntap_sys_set_descr(struct device *, const char *, size_t);
char	*tuntap_sys_get_descr(struct device *);

#endif
