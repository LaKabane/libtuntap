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
#if defined Unix
# include <sys/socket.h>
#else /* Windows */
#include <In6addr.h>
#endif

#if defined Unix
# if defined Linux
#  include <linux/if.h>
# else
#  include <net/if.h>
# endif
# include <netinet/in.h>
# include <netinet/if_ether.h>
#endif

#include <stdint.h>

#ifndef LIBTUNTAP_H_
# define LIBTUNTAP_H_

/*
 * Uniformize macros
 * - ETHER_ADDR_LEN: Magic number from IEEE 802.3
 * - IF_NAMESIZE: Length of interface external name
 * - IF_DESCRSIZE: Length of interface description
 * - TUNSDEBUG: ioctl flag to enable the debug mode of a tun device
 * - TUNFD_INVALID_VALUE: Invalid value for tun_fd
 */
# if defined ETH_ALEN /* Linux */
#  define ETHER_ADDR_LEN ETH_ALEN
# elif defined Windows
#  define ETHER_ADDR_LEN 6 
# endif

# if defined IFNAMSIZ && !defined IF_NAMESIZE
#  define IF_NAMESIZE IFNAMSIZ /* Historical BSD name */
# elif !defined IF_NAMESIZE
#  define IF_NAMESIZE 16
# endif

# define IF_DESCRSIZE 50 /* XXX: Tests needed on NetBSD and OpenBSD */

# if defined TUNSETDEBUG
#  define TUNSDEBUG TUNSETDEBUG
# endif

# if defined Unix
#  define TUNFD_INVALID_VALUE -1
# else /* Window */
#  define TUNFD_INVALID_VALUE INVALID_HANDLE_VALUE
# endif

/*
 * Uniformize types
 * - t_tun: tun device file descriptor
 * - t_tun_in_addr: struct in_addr/IN_ADDR
 * - t_tun_in6_addr: struct in6_addr/IN6_ADDR
 */
# if defined Unix
typedef int t_tun;
typedef struct in_addr t_tun_in_addr;
typedef struct in6_addr t_tun_in6_addr;
# else /* Windows */
typedef HANDLE t_tun;
typedef IN_ADDR t_tun_in_addr;
typedef IN6_ADDR t_tun_in6_addr;
# endif

# define TUNTAP_ID_MAX 256
# define TUNTAP_ID_ANY 257

# define TUNTAP_MODE_ETHERNET 0x0001
# define TUNTAP_MODE_TUNNEL   0x0002
# define TUNTAP_MODE_PERSIST  0x0004

# define TUNTAP_LOG_NONE      0x0000
# define TUNTAP_LOG_DEBUG     0x0001
# define TUNTAP_LOG_INFO      0x0002
# define TUNTAP_LOG_NOTICE    0x0004
# define TUNTAP_LOG_WARN      0x0008
# define TUNTAP_LOG_ERR       0x0016

# define TUNTAP_GET_FD(x) (x)->tun_fd

#if defined (_WIN32) 
  #if defined(tuntap_EXPORTS) /* CMake generated goo */
    #define  TUNTAP_EXPORT __declspec(dllexport)
  #else
    #define  TUNTAP_EXPORT __declspec(dllimport)
  #endif
#else /* Unix */
 #define TUNTAP_EXPORT
#endif

# ifdef __cplusplus
extern "C" {
# endif

struct device {
	t_tun		tun_fd;
	int		ctrl_sock;
	int		flags;     /* ifr.ifr_flags on Unix */
	unsigned char	hwaddr[ETHER_ADDR_LEN];
	char		if_name[IF_NAMESIZE];
};

/* User definable log callback */
typedef void (*t_tuntap_log)(int, const char *);
t_tuntap_log tuntap_log;

/* Portable "public" functions */
TUNTAP_EXPORT struct device	*tuntap_init(void);
TUNTAP_EXPORT void		 tuntap_destroy(struct device *);
TUNTAP_EXPORT void		 tuntap_release(struct device *);
TUNTAP_EXPORT int		 tuntap_start(struct device *, int, int);
TUNTAP_EXPORT char		*tuntap_get_ifname(struct device *);
TUNTAP_EXPORT int		 tuntap_set_ifname(struct device *, const char *);
TUNTAP_EXPORT char		*tuntap_get_hwaddr(struct device *);
TUNTAP_EXPORT int		 tuntap_set_hwaddr(struct device *, const char *);
TUNTAP_EXPORT int		 tuntap_set_descr(struct device *, const char *);
TUNTAP_EXPORT int		 tuntap_up(struct device *);
TUNTAP_EXPORT int		 tuntap_down(struct device *);
TUNTAP_EXPORT int		 tuntap_get_mtu(struct device *);
TUNTAP_EXPORT int		 tuntap_set_mtu(struct device *, int);
TUNTAP_EXPORT int		 tuntap_set_ip(struct device *, const char *, int);
TUNTAP_EXPORT int		 tuntap_read(struct device *, void *, size_t);
TUNTAP_EXPORT int		 tuntap_write(struct device *, void *, size_t);
TUNTAP_EXPORT int		 tuntap_get_readable(struct device *);
TUNTAP_EXPORT int		 tuntap_set_nonblocking(struct device *dev, int);
TUNTAP_EXPORT int		 tuntap_set_debug(struct device *dev, int);

/* Logging functions */
TUNTAP_EXPORT void		 tuntap_log_set_cb(t_tuntap_log cb);
void		 tuntap_log_default(int, const char *);
void		 tuntap_log_hexdump(void *, size_t);
void		 tuntap_log_chksum(void *, int);

/* OS specific functions */
int		 tuntap_sys_start(struct device *, int, int);
void		 tuntap_sys_destroy(struct device *);
int		 tuntap_sys_set_hwaddr(struct device *, struct ether_addr *);
int		 tuntap_sys_set_ipv4(struct device *, t_tun_in_addr *, uint32_t);
int		 tuntap_sys_set_ipv6(struct device *, t_tun_in6_addr *, uint32_t);
int		 tuntap_sys_set_ifname(struct device *, const char *, size_t);
int		 tuntap_sys_set_descr(struct device *, const char *, size_t);

# ifdef __cplusplus
}
# endif

#endif
