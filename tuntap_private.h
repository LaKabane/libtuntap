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

#if defined HAVE_LINUX_IF_H
# include <linux/if.h>
#elif defined HAVE_NET_IF_H
# include <net/if.h>
#endif

#if !defined Windows
# include <netinet/in.h>
#endif

#if defined HAVE_NETINET_IF_ETHER_H
# include <netinet/if_ether.h>
#elif defined HAVE_NETINET_ETHER_H
# include <netinet/ether.h>
#endif

#ifndef LIBTUNTAP_PRIVATE_H_
# define LIBTUNTAP_PRIVATE_H_

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
# elif defined ETHERADDRL
#  define ETHER_ADDR_LEN ETHERADDRL /* SunOS */
# else /* Windows */
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

struct device {
	t_tun           tun_fd;
	int             ctrl_sock;
	int             flags;     /* ifr.ifr_flags on Unix */
	unsigned char   hwaddr[ETHER_ADDR_LEN];
	char            if_name[IF_NAMESIZE];
};

/*
 * Windows helpers, prevent it from warning every time
 */
# if defined Windows
#  define snprintf(x, y, z, ...) _snprintf_s((x), (y), (y), (z), __VA_ARGS__);
#  define strncat(x, y, z) strncat_s((x), _countof(x), (y), (z));
#  define strdup(x) _strdup(x)
# endif

#if __GNUC__ >= 4
  #define TUNTAP_PRI __attribute__ ((visibility ("hidden")))
#elif __SUNPRO_C
  #define TUNTAP_PRI __hidden
#else
  #define TUNTAP_PRI
#endif

/* OS specific functions */
TUNTAP_PRI void	tuntap_log_default(int, const char *);
TUNTAP_PRI int	tuntap_sys_start(struct device *, int, int);

#endif
