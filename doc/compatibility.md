Operating Systems Compatibility
===============================

libtuntap is portable among various operating systems, mostly free UNIX ones.

The only requirements for libtuntap to work is:

   * A working C compiler;
   * A working virtual network device (like tun and tap);

As every systems as a different device implementation, not all functions
provided by libtuntap will work on every plateform.

API compatibility grid
----------------------

+------------------------+---------+-------+--------+--------+
| API                    | OpenBSD | Linux | Darwin | NetBSD |
+------------------------+---------+-------+--------+--------+
| tuntap_destroy         |   Yes   |  Yes  |   Yes  |   Yes  |
| tuntap_down            |   Yes   |  Yes  |   Yes  |   Yes  |
| tuntap_get_debug       |   Yes   |  No   |   No   |   Yes  |
| tuntap_get_descr       |   Yes   |  No   |   No   |   No   |
| tuntap_get_hwaddr      |   Yes   |  Yes  |   Yes  |   Yes  |
| tuntap_get_ifname      |   Yes   |  Yes  |   Yes  |   Yes  |
| tuntap_get_mtu         |   Yes   |  Yes  |   Yes  |   Yes  |
| tuntap_get_readable    |   Yes   |  Yes  |   Yes  |   Yes  |
| tuntap_init            |   Yes   |  Yes  |   Yes  |   Yes  |
| tuntap_log             |   Yes   |  Yes  |   No   |   Yes  |
| tuntap_read            |   Yes   |  Yes  |   Yes  |   Yes  |
| tuntap_release         |   Yes   |  Yes  |   Yes  |   Yes  |
| tuntap_set_debug       |   Yes   |  Yes  |   No   |   No   |
| tuntap_set_desrc       |   Yes   |  No   |   No   |   No   |
| tuntap_set_hwaddr      |   Yes   |  Yes  |   Yes  |   Yes  |
| tuntap_set_ifname      |   No    |  Yes  |   No   |   No   |
| tuntap_set_ip          |   Yes   |  Yes  |   Yes  |   Yes  |
| tuntap_set_mtu         |   Yes   |  Yes  |   Yes  |   Yes  |
| tuntap_set_nonblocking |   Yes   |  No   |   Yes  |   Yes  |
| tuntap_start           |   Yes   |  Yes  |   Yes  |   Yes  |
| tuntap_up              |   Yes   |  Yes  |   Yes  |   Yes  |
| tuntap_version         |   Yes   |  Yes  |   Yes  |   Yes  |
| tuntap_write           |   Yes   |  Yes  |   Yes  |   Yes  |
+------------------------+---------+-------+--------+--------+

Limitation - MTU
----------------

The different operating systems on which libtuntap is ported impose different
restrictions. One of them is the MTU, that you can modify with tuntap_set_mtu().

=== OpenBSD

On OpenBSD, the given MTU should be between 50 (ETHERMIN) and 16384 (TUNMRU).

Source: /usr/src/sys/net/if_tun.{c,h}

=== Linux

On Linux, the given MTU should be between 68 and 65535.

Limitation - Devices number
---------------------------

Some systems don't spawn new device on the fly, and thus they have to be
manually generated past some point.

=== OpenBSD

OpenBSD limits to 4 devices by default. If you want more you have use MAKEDEV.

    sudo /dev/MAKEDEV tun4

