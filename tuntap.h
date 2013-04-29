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

#include <sys/types.h>
#if defined Windows
# include <In6addr.h>
#else /* Unix */
# include <sys/socket.h>
#endif

#ifndef LIBTUNTAP_H_
# define LIBTUNTAP_H_

/*
 * Uniformize types
 * - t_tun: tun device file descriptor
 * - t_tun_in_addr: struct in_addr/IN_ADDR
 * - t_tun_in6_addr: struct in6_addr/IN6_ADDR
 */
# if defined Windows
typedef HANDLE t_tun;
typedef IN_ADDR t_tun_in_addr;
typedef IN6_ADDR t_tun_in6_addr;
# else /* Unix */
typedef int t_tun;
typedef struct in_addr t_tun_in_addr;
typedef struct in6_addr t_tun_in6_addr;
# endif

# define TUNTAP_ID_MAX 256
# define TUNTAP_ID_ANY 257

# define TUNTAP_MODE_ETHERNET 0x0001
# define TUNTAP_MODE_TUNNEL   0x0002
# define TUNTAP_MODE_PERSIST  0x0004

/* Needed for people who redefine the log callback */
# define TUNTAP_LOG_NONE      0x0000
# define TUNTAP_LOG_DEBUG     0x0001
# define TUNTAP_LOG_INFO      0x0002
# define TUNTAP_LOG_NOTICE    0x0004
# define TUNTAP_LOG_WARN      0x0008
# define TUNTAP_LOG_ERR       0x0016

/* Versioning: 0xMMmm, with 'M' for major and 'm' for minor */
# define TUNTAP_VERSION_MAJOR 0
# define TUNTAP_VERSION_MINOR 4
# define TUNTAP_VERSION ((TUNTAP_VERSION_MAJOR<<8)|TUNTAP_VERSION_MINOR)

/* XXX: Why not as a function? */
# define TUNTAP_GET_FD(x) (x)->tun_fd

/* Handle Windows symbols export */
# if defined Windows 
#  if defined(tuntap_EXPORTS) /* CMake generated goo */
#   define  TUNTAP_PUB __declspec(dllexport)
#  else
#   define  TUNTAP_PUB __declspec(dllimport)
#  endif
# else /* Unix */
#  define TUNTAP_PUB
# endif

# ifdef __cplusplus
extern "C" {
# endif

/* Forward declare struct device */
struct device;

/* User definable log callback */
typedef void (*t_tuntap_log)(int, const char *);
TUNTAP_PUB t_tuntap_log tuntap_log;

/* Portable "public" functions */
TUNTAP_PUB struct device*tuntap_init(void);
TUNTAP_PUB int		 tuntap_version(void);
TUNTAP_PUB void		 tuntap_destroy(struct device *);
TUNTAP_PUB void		 tuntap_release(struct device *);
TUNTAP_PUB int		 tuntap_start(struct device *, int, int);
TUNTAP_PUB int		 tuntap_up(struct device *);
TUNTAP_PUB int		 tuntap_down(struct device *);
TUNTAP_PUB int		 tuntap_read(struct device *, void *, size_t);
TUNTAP_PUB int		 tuntap_write(struct device *, void *, size_t);

TUNTAP_PUB char		*tuntap_get_ifname(struct device *);
TUNTAP_PUB int		 tuntap_set_ifname(struct device *, const char *);
TUNTAP_PUB char		*tuntap_get_hwaddr(struct device *);
TUNTAP_PUB int		 tuntap_set_hwaddr(struct device *, const char *);
TUNTAP_PUB char		*tuntap_get_descr(struct device *);
TUNTAP_PUB int		 tuntap_set_descr(struct device *, const char *);
TUNTAP_PUB int		 tuntap_get_mtu(struct device *);
TUNTAP_PUB int		 tuntap_set_mtu(struct device *, int);
TUNTAP_PUB int		 tuntap_get_debug(struct device *dev);
TUNTAP_PUB int		 tuntap_set_debug(struct device *dev, int);
TUNTAP_PUB int		 tuntap_set_ip(struct device *, const char *, int);
TUNTAP_PUB int		 tuntap_del_ip(struct device *, const char *, int);
TUNTAP_PUB int		 tuntap_get_readable(struct device *);
TUNTAP_PUB int		 tuntap_set_nonblocking(struct device *dev, int);

/* Logging functions */
TUNTAP_PUB void		 tuntap_log_set_cb(t_tuntap_log cb);
TUNTAP_PUB void		 tuntap_log_hexdump(void *, size_t);
TUNTAP_PUB void		 tuntap_log_chksum(void *, int);

# ifdef __cplusplus
}
# endif

#endif
