/* stripped-up version of linux header for libtuntap */
/*
 *  Universal TUN/TAP device driver.
 *  Copyright (C) 1999-2000 Maxim Krasnyansky <max_mk@yahoo.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 */
#ifndef LIBTUNTAP__IF_TUN_H
#define LIBTUNTAP__IF_TUN_H
/* Read queue size */
#define TUN_READQ_SIZE	500
/* TUN device type flags: deprecated. Use IFF_TUN/IFF_TAP instead. */
#define TUN_TUN_DEV 	IFF_TUN
#define TUN_TAP_DEV	IFF_TAP
#define TUN_TYPE_MASK   0x000f
/* Ioctl defines */
#define TUNSETNOCSUM  _IOW('T', 200, int)
#define TUNSETDEBUG   _IOW('T', 201, int)
#define TUNSETIFF     _IOW('T', 202, int)
#define TUNSETPERSIST _IOW('T', 203, int)
#define TUNSETOWNER   _IOW('T', 204, int)
#define TUNSETLINK    _IOW('T', 205, int)
#define TUNSETGROUP   _IOW('T', 206, int)
#define TUNGETFEATURES _IOR('T', 207, unsigned int)
#define TUNSETOFFLOAD  _IOW('T', 208, unsigned int)
#define TUNSETTXFILTER _IOW('T', 209, unsigned int)
#define TUNGETIFF      _IOR('T', 210, unsigned int)
#define TUNGETSNDBUF   _IOR('T', 211, int)
#define TUNSETSNDBUF   _IOW('T', 212, int)
#define TUNATTACHFILTER _IOW('T', 213, struct sock_fprog)
#define TUNDETACHFILTER _IOW('T', 214, struct sock_fprog)
#define TUNGETVNETHDRSZ _IOR('T', 215, int)
#define TUNSETVNETHDRSZ _IOW('T', 216, int)
#define TUNSETQUEUE  _IOW('T', 217, int)
#define TUNSETIFINDEX	_IOW('T', 218, unsigned int)
#define TUNGETFILTER _IOR('T', 219, struct sock_fprog)
#define TUNSETVNETLE _IOW('T', 220, int)
#define TUNGETVNETLE _IOR('T', 221, int)
/* The TUNSETVNETBE and TUNGETVNETBE ioctls are for cross-endian support on
 * little-endian hosts. Not all kernel configurations support them, but all
 * configurations that support SET also support GET.
 */
#define TUNSETVNETBE _IOW('T', 222, int)
#define TUNGETVNETBE _IOR('T', 223, int)
/* TUNSETIFF ifr flags */
#define IFF_TUN		0x0001
#define IFF_TAP		0x0002
#define IFF_NO_PI	0x1000
/* This flag has no real effect */
#define IFF_ONE_QUEUE	0x2000
#define IFF_VNET_HDR	0x4000
#define IFF_TUN_EXCL	0x8000
#define IFF_MULTI_QUEUE 0x0100
#define IFF_ATTACH_QUEUE 0x0200
#define IFF_DETACH_QUEUE 0x0400
/* read-only flag */
#define IFF_PERSIST	0x0800
#define IFF_NOFILTER	0x1000
/* Socket options */
#define TUN_TX_TIMESTAMP 1
/* Features for GSO (TUNSETOFFLOAD). */
#define TUN_F_CSUM	0x01	/* You can hand me unchecksummed packets. */
#define TUN_F_TSO4	0x02	/* I can handle TSO for IPv4 packets */
#define TUN_F_TSO6	0x04	/* I can handle TSO for IPv6 packets */
#define TUN_F_TSO_ECN	0x08	/* I can handle TSO with ECN bits. */
#define TUN_F_UFO	0x10	/* I can handle UFO packets */
#endif /* __IF_TUN_H */
