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

# if defined ETH_ALEN /* Linux */
#  define ETHER_ADDR_LEN ETH_ALEN
# elif defined Windows
#  define ETHER_ADDR_LEN 6 
# endif

# define IF_DESCRSIZE 50 /* XXX: Tests needed on NetBSD and OpenBSD */

# if defined TUNSETDEBUG
#  define TUNSDEBUG TUNSETDEBUG
# endif

# if defined Windows
#  define TUNFD_INVALID_VALUE INVALID_HANDLE_VALUE
# else /* Unix */
#  define TUNFD_INVALID_VALUE -1
# endif

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
