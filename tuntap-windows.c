/*
 * Copyright (c) 2012 Tristan Le Guern <tleguern@bouledef.eu>
 * Copyright (c) 2026 Roland Schwarz <roland.schwarz@blackspace.at>
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

/* The windows port makes use of the tuntap driver from the tap-windows6
 * subproject of the openvpn project. It is possible to use the driver that can
 * be installed as part of the openvpn installation. Once the driver is on the
 * system, use "add legacy adapters" from the device manager to add additional
 * adapters.
 */

#include <assert.h>
#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strsafe.h>
#include <time.h>

#include "private.h"
#include "tuntap.h"

#include <wbemidl.h>

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

#define WORKING_BUFFER_SIZE 15000
#define MAX_TRIES 3

/* From OpenVPN tap driver, common.h */
#define TAP_CONTROL_CODE(request, method) CTL_CODE(FILE_DEVICE_UNKNOWN, request, method, FILE_ANY_ACCESS)
#define TAP_IOCTL_GET_MAC TAP_CONTROL_CODE(1, METHOD_BUFFERED)
#define TAP_IOCTL_GET_VERSION TAP_CONTROL_CODE(2, METHOD_BUFFERED)
#define TAP_IOCTL_GET_MTU TAP_CONTROL_CODE(3, METHOD_BUFFERED)
#define TAP_IOCTL_GET_INFO TAP_CONTROL_CODE(4, METHOD_BUFFERED)
#define TAP_IOCTL_CONFIG_POINT_TO_POINT TAP_CONTROL_CODE(5, METHOD_BUFFERED)
#define TAP_IOCTL_SET_MEDIA_STATUS TAP_CONTROL_CODE(6, METHOD_BUFFERED)
#define TAP_IOCTL_CONFIG_DHCP_MASQ TAP_CONTROL_CODE(7, METHOD_BUFFERED)
#define TAP_IOCTL_GET_LOG_LINE TAP_CONTROL_CODE(8, METHOD_BUFFERED)
#define TAP_IOCTL_CONFIG_DHCP_SET_OPT TAP_CONTROL_CODE(9, METHOD_BUFFERED)
#define TAP_IOCTL_CONFIG_TUN TAP_CONTROL_CODE(10, METHOD_BUFFERED)

/* Windows registry crap */
#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383
#define NETWORK_ADAPTERS                                                                                                                                       \
	"SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-"                                                                                 \
	"08002BE10318}"
#define NETWORK_CONNECTIONS                                                                                                                                    \
	"SYSTEM\\CurrentControlSet\\Control\\Network\\{4D36E972-E325-11CE-BFC1-"                                                                               \
	"08002BE10318}"

/* The component Id from the tap-windows6 project, used to identify the driver
   in the registry. */
#define TAP_WIN_COMPONENT_ID "tap0901"

#define TUNTAP_MODE_MASK (TUNTAP_MODE_ETHERNET | TUNTAP_MODE_TUNNEL)

/* From OpenVPN tap driver, proto.h */
typedef unsigned long IPADDR;

#define NET_CFG_INSTANCE_ID_LEN 38

typedef struct {
	int mode;
	IF_INDEX index;
	char macbuf[3 * ETHER_ADDR_LEN];
	char net_cfg_instance_id[NET_CFG_INSTANCE_ID_LEN + 1];
	char description[256];
	union {
		ULONG Flags;
		struct {
			ULONG DdnsEnabled : 1;
			ULONG RegisterAdapterSuffix : 1;
			ULONG Dhcpv4Enabled : 1;
			ULONG ReceiveOnly : 1;
			ULONG NoMulticast : 1;
			ULONG Ipv6OtherStatefulConfig : 1;
			ULONG NetbiosOverTcpipEnabled : 1;
			ULONG Ipv4Enabled : 1;
			ULONG Ipv6Enabled : 1;
			ULONG Ipv6ManagedAddressConfigurationSupported : 1;
		};
	};

} system_device;

/* convenience fmt forwarding to logger */
static void
tuntap_logf(int level, const char *fmt, ...)
{
	char buf[256];
	va_list args = NULL;
	va_start(args, fmt);
	vsnprintf(buf, sizeof buf, fmt, args);
	va_end(args);
	tuntap_log(level, buf);
}

static void
tuntap_log_winerr(int level, DWORD err)
{ /* TODO: Use FormatMessageW and convert tu UTF8 strings */
	LPTSTR lpBuffer = NULL;
	FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, err, 0, (LPTSTR)&lpBuffer, 0, NULL);
	tuntap_log(level, lpBuffer);
	LocalFree(lpBuffer);
}

/* Helper for reading registry wide chars */
static int
WideToMulti(PWCHAR src, char *dstBuf, int dstBufSize, UINT codePage)
{
	if (!src || !dstBuf) {
		return 0;
	}
	// Get required size (including null)
	int required = WideCharToMultiByte(codePage, 0, src, -1, NULL, 0, NULL, NULL);
	if (required == 0) {
		return 0;
	}
	if (required > dstBufSize) {
		return 0; // caller may realloc
	}
	return WideCharToMultiByte(codePage, 0, src, -1, dstBuf, dstBufSize, NULL, NULL);
}

/* The list of valid adapters (tap-windows6) must be read from registry */
typedef char (*valid_adapters_t)[39];

static size_t
get_valid_adapters(valid_adapters_t *pva)
{
	*pva = NULL;
	size_t cva = 0;

	HKEY adapters, adapter;
	DWORD i, ret, len;
	DWORD sub_keys = 0;

	ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT(NETWORK_ADAPTERS), 0, KEY_READ, &adapters);
	if (ret != ERROR_SUCCESS) {
		tuntap_log_winerr(TUNTAP_LOG_ERR, ret);
		return 0;
	}

	ret = RegQueryInfoKey(adapters, NULL, NULL, NULL, &sub_keys, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
	if (ret != ERROR_SUCCESS) {
		tuntap_log_winerr(TUNTAP_LOG_ERR, ret);
		RegCloseKey(adapters);
		return 0;
	}

	if (sub_keys <= 0) {
		tuntap_log(TUNTAP_LOG_DEBUG, "Wrong registry key");
		RegCloseKey(adapters);
		return 0;
	}

	*pva = calloc(sub_keys, sizeof *(valid_adapters_t)0);

	for (i = 0; i < sub_keys; ++i) {
		char new_key[MAX_KEY_LENGTH];
		char data[256];
		TCHAR key[MAX_KEY_LENGTH];
		DWORD keylen = MAX_KEY_LENGTH;

		/* Get the adapter key name */
		ret = RegEnumKeyEx(adapters, i, key, &keylen, NULL, NULL, NULL, NULL);
		if (ret != ERROR_SUCCESS) {
			continue;
		}

		/* Append it to NETWORK_ADAPTERS and open it */
		snprintf(new_key, sizeof new_key, "%s\\%s", NETWORK_ADAPTERS, key);
		ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT(new_key), 0, KEY_READ, &adapter);
		if (ret != ERROR_SUCCESS) {
			continue;
		}

		/* Check its values */
		len = sizeof data;
		ret = RegQueryValueEx(adapter, "ComponentId", NULL, NULL, (LPBYTE)data, &len);
		if (ret != ERROR_SUCCESS) {
			/* This value doesn't exist in this adaptater tree */
			goto clean;
		}

		if (strcasecmp(data, TAP_WIN_COMPONENT_ID) == 0 || strcasecmp(data, "root\\" TAP_WIN_COMPONENT_ID) == 0) {
			len = sizeof data;
			ret = RegQueryValueEx(adapter, "NetCfgInstanceId", NULL, NULL, (LPBYTE)data, &len);
			if (ret != ERROR_SUCCESS) {
				tuntap_log_winerr(TUNTAP_LOG_ERR, ret);
				goto clean;
			}

			strncpy((*pva)[cva++], data, sizeof *(valid_adapters_t)0);
		}
	clean:
		RegCloseKey(adapter);
	}
	RegCloseKey(adapters);
	if (0 == cva) {
		free(*pva);
		*pva = NULL;
	}

	return cva;
}

static int
tuntap_start_named(struct device *dev, int mode, const char *name)
{
	HANDLE tun_fd = TUNFD_INVALID_VALUE;

	ULONG outBufLen = 0;
	ULONG Iterations = 0;
	PIP_ADAPTER_ADDRESSES pAddresses, pCurr;
	PIP_ADAPTER_UNICAST_ADDRESS pUnicast;
	ULONG flags = GAA_FLAG_INCLUDE_PREFIX;
	ULONG family = AF_UNSPEC;

	TCHAR szFName[256];
	IPADDR ipAddr[3];
	DWORD dwLen, dwRetVal;

	size_t n;
	char buf[MAX_PATH];

	/* Don't re-initialise a previously started device */
	if (dev->tun_fd != TUNFD_INVALID_VALUE) {
		tuntap_log(TUNTAP_LOG_WARN, "Dont restart an open device");
		return -1;
	}

	if (NULL != dev->sys) {
		tuntap_log(TUNTAP_LOG_DEBUG, "System device not empty");
	}

	system_device sysdev = {0};

	if (TUNTAP_MODE_ETHERNET != (mode & TUNTAP_MODE_MASK) && TUNTAP_MODE_TUNNEL != (mode & TUNTAP_MODE_MASK)) {
		tuntap_log(TUNTAP_LOG_ERR, "Invalid parameter 'mode'");
		return -1;
	}

	/* Open by 'name' or any adapter if name is empty by walking the Adapters*/

	/* Acquire buffer space */
	outBufLen = WORKING_BUFFER_SIZE;
	do {
		pAddresses = (IP_ADAPTER_ADDRESSES *)MALLOC(outBufLen);
		if (NULL == pAddresses) {
			tuntap_log(TUNTAP_LOG_ERR, "Memory allocation failed");
			return -1;
		}
		dwRetVal = GetAdaptersAddresses(family, flags, NULL, pAddresses, &outBufLen);

		if (ERROR_BUFFER_OVERFLOW == dwRetVal) {
			FREE(pAddresses);
			pAddresses = NULL;
		} else {
			break;
		}

		++Iterations;

	} while ((dwRetVal == ERROR_BUFFER_OVERFLOW) && (Iterations < MAX_TRIES));

	size_t vac = 0; /* count of valid adapters*/
	valid_adapters_t va = NULL;
	/* load from registry */
	if (0 == (vac = get_valid_adapters(&va))) {
		tuntap_log(TUNTAP_LOG_ERR, "No usable adapters installed");
		return -1;
	}

	if (NO_ERROR == dwRetVal) {
		/* Loop all adapters, open first matching the 'name' */

		for (pCurr = pAddresses; NULL != pCurr; pCurr = pCurr->Next) {
			if (53 != pCurr->IfType) {
				continue;
			}
			/* Lookup the list of valid adapters */
			for (n = 0; n < vac; ++n) {
				if (0 == strcmp(va[n], pCurr->AdapterName)) {
					break;
				}
			}
			/* Found a valid candidate */
			WideToMulti(pCurr->FriendlyName, szFName, sizeof szFName, CP_UTF8);
			if (n < vac && (0 == strcmp(name, szFName) || '\0' == name[0])) {
				tuntap_logf(TUNTAP_LOG_DEBUG, "Adapter tap-windows6: %s", szFName);
				snprintf(buf, sizeof buf, "\\\\.\\Global\\%s.tap", pCurr->AdapterName);
				tun_fd =
				    CreateFile(buf, GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_SYSTEM | FILE_FLAG_OVERLAPPED, NULL);
				if (tun_fd != TUNFD_INVALID_VALUE) {
					strncpy(dev->if_name, szFName, IF_NAMESIZE + 1); // sizeof(dev->if_name));
					dev->if_name[IF_NAMESIZE] = '\0';
					strncpy(sysdev.net_cfg_instance_id, pCurr->AdapterName, NET_CFG_INSTANCE_ID_LEN);
					sysdev.net_cfg_instance_id[NET_CFG_INSTANCE_ID_LEN] = '\0';
					WideToMulti(pCurr->Description, sysdev.description, sizeof sysdev.description, CP_UTF8);
					tuntap_logf(TUNTAP_LOG_DEBUG, "Interface: %lu", pCurr->IfIndex);
					sysdev.index = pCurr->IfIndex;
					sysdev.Flags = pCurr->Flags;
					if (TUNTAP_MODE_TUNNEL & mode) {
						ipAddr[0] = 0;
						/* Get this adapters address IPv4 address */
						for (pUnicast = pCurr->FirstUnicastAddress; pUnicast != NULL; pUnicast = pUnicast->Next) {
							if (AF_INET == pUnicast->Address.lpSockaddr->sa_family) {
								ipAddr[0] = ((PSOCKADDR_IN)(pUnicast->Address.lpSockaddr))->sin_addr.S_un.S_addr;
								break;
							}
						}
						if (!pCurr->Ipv4Enabled) {
							tuntap_log(TUNTAP_LOG_DEBUG, "Adapter in IPv6 only mode");
						} else {
							inet_ntop(AF_INET, &ipAddr[0], buf, sizeof buf);
							tuntap_logf(TUNTAP_LOG_DEBUG, "Adapter IPv4: %s", buf);
						}
						ipAddr[1] = ipAddr[2] = 0;
						if (DeviceIoControl(tun_fd, TAP_IOCTL_CONFIG_TUN, &ipAddr, sizeof(ipAddr), &ipAddr, sizeof(ipAddr), &dwLen,
						                    NULL) == 0) {
							DWORD error = GetLastError();
							CloseServiceHandle(tun_fd);
							tun_fd = TUNFD_INVALID_VALUE;
							tuntap_log_winerr(TUNTAP_LOG_DEBUG, error);
						} else {
							tuntap_log(TUNTAP_LOG_DEBUG, "Opened in TUN mode");
							break;
						}
					} else {
						tuntap_log(TUNTAP_LOG_DEBUG, "Opened in TAP mode");
						break;
					}
				} else {
					DWORD error = GetLastError();
					tuntap_log_winerr(TUNTAP_LOG_DEBUG, error);
				}
			}
		}
	} else {
		tuntap_log_winerr(TUNTAP_LOG_ERR, dwRetVal);
	}

	free(va);

	dev->tun_fd = tun_fd;
	if (TUNFD_INVALID_VALUE != tun_fd) {
		dev->sys = calloc(1, sizeof(system_device));
		*(system_device *)dev->sys = sysdev;
		return 0;
	} else {
		return -1;
	}
}

/* Open, using canonical tunX tapX names */
int
tuntap_start(struct device *dev, int mode, int tun)
{
	char name[16] = ""; /* empty means any name */

	if (TUNTAP_ID_ANY != tun) {
		if (TUNTAP_MODE_ETHERNET == (mode & TUNTAP_MODE_MASK)) {
			snprintf(name, sizeof name, "tap%i", tun);
		} else if (TUNTAP_MODE_TUNNEL == (mode & TUNTAP_MODE_MASK)) {
			snprintf(name, sizeof name, "tun%i", tun);
			;
		} else {
			tuntap_log(TUNTAP_LOG_ERR, "Invalid parameter 'mode'");
			return -1;
		}
	}

	return tuntap_start_named(dev, mode, name);
}

void
tuntap_release(struct device *dev)
{
	if (0 == CloseHandle(dev->tun_fd)) {
		tuntap_log_winerr(TUNTAP_LOG_ERR, GetLastError());
	}
	assert(NULL != dev->sys);
	free(dev->sys);
	dev->sys = NULL;
	free(dev);
}

void
tuntap_sys_destroy(struct device *dev)
{
	tuntap_log(TUNTAP_LOG_WARN, "On windows devices always persist");
	(void)dev;
	return;
}

char *
tuntap_get_hwaddr(struct device *dev)
{
	system_device *psys = dev->sys;
	DWORD len;

	if (DeviceIoControl(dev->tun_fd, TAP_IOCTL_GET_MAC, &dev->hwaddr, sizeof(dev->hwaddr), &dev->hwaddr, sizeof(dev->hwaddr), &len, NULL) == 0) {
		tuntap_log_winerr(TUNTAP_LOG_ERR, GetLastError());
		return NULL;
	} else {
		(void)_snprintf_s(psys->macbuf, sizeof psys->macbuf, sizeof psys->macbuf, "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", dev->hwaddr[0], dev->hwaddr[1],
		                  dev->hwaddr[2], dev->hwaddr[3], dev->hwaddr[4], dev->hwaddr[5]);
		tuntap_logf(TUNTAP_LOG_DEBUG, "MAC address: %s", psys->macbuf);
	}

	return psys->macbuf;
}

int
tuntap_set_hwaddr(struct device *dev, const char *hwaddr)
{
	(void)hwaddr;
	struct ether_addr dummy;
	return tuntap_sys_set_hwaddr(dev, &dummy);
}

int
tuntap_sys_set_hwaddr(struct device *dev, struct ether_addr *eth_addr)
{
	(void)dev;
	(void)eth_addr;
	tuntap_log(TUNTAP_LOG_NOTICE, "Your system does not support tuntap_set_hwaddr()");
	return -1;
}

static int
tuntap_sys_set_updown(struct device *dev, ULONG flag)
{
	DWORD len;

	if (DeviceIoControl(dev->tun_fd, TAP_IOCTL_SET_MEDIA_STATUS, &flag, sizeof(flag), &flag, sizeof(flag), &len, NULL) == 0) {
		tuntap_log_winerr(TUNTAP_LOG_ERR, GetLastError());
		return -1;
	} else {
		if (flag) {
			tuntap_logf(TUNTAP_LOG_DEBUG, "Status %s: up", dev->if_name);
		} else {
			tuntap_logf(TUNTAP_LOG_DEBUG, "Status %s: down", dev->if_name);
		}
		return 0;
	}
}

int
tuntap_up(struct device *dev)
{
	ULONG flag;

	flag = 1;
	return tuntap_sys_set_updown(dev, flag);
}

int
tuntap_down(struct device *dev)
{
	ULONG flag;

	flag = 0;
	return tuntap_sys_set_updown(dev, flag);
}

int
tuntap_get_mtu(struct device *dev)
{
	ULONG mtu;
	DWORD len;

	if (DeviceIoControl(dev->tun_fd, TAP_IOCTL_GET_MTU, &mtu, sizeof(mtu), &mtu, sizeof(mtu), &len, NULL) == 0) {
		tuntap_log_winerr(TUNTAP_LOG_ERR, GetLastError());
		return -1;
	}
	return mtu;
}

int
tuntap_set_mtu(struct device *dev, int mtu)
{
#if 0	/* The following did not work reliably for me */
	system_device *psys = dev->sys;
	DWORD err = 0;
	MIB_IPINTERFACE_ROW ipiface;
	InitializeIpInterfaceEntry(&ipiface);
	ipiface.Family = AF_INET6;
	ipiface.InterfaceIndex = psys->index;
	err = GetIpInterfaceEntry(&ipiface);
	if ( NO_ERROR == err) {
		ipiface.SitePrefixLength = 0;
	}
	ipiface.NlMtu = mtu;
	err = SetIpInterfaceEntry(&ipiface);
	if (NO_ERROR != err) {
		tuntap_log_winerr(TUNTAP_LOG_WARN, err);
		return -1;
	} else {
		tuntap_logf(TUNTAP_LOG_DEBUG, "%s MTU set to %d", dev->if_name, mtu);
	}
#endif
	(void)dev;
	(void)mtu;
	tuntap_log(TUNTAP_LOG_NOTICE, "Your system does not support tuntap_set_mtu()");
	return -1;
}

int
tuntap_sys_set_ipv4(struct device *dev, t_tun_in_addr *s, uint32_t mask)
{
	system_device *sysdev = dev->sys;
	DWORD ret, dwLen;

	TCHAR IPv4Address[16];
	inet_ntop(AF_INET, s, IPv4Address, sizeof IPv4Address);
	/* calc prefix length */
	UINT8 PrefixLength = 0;
	uint32_t test = ntohl(mask);
	while (test & UINT32_C(0x80000000)) {
		++PrefixLength;
		test <<= 1;
	}

	if (sysdev->mode & TUNTAP_MODE_ETHERNET) {
		tuntap_log(TUNTAP_LOG_ERR, "libtuntap does not allow to set IPv4 address in tap mode");
		return -1;
	} else if (sysdev->Dhcpv4Enabled) {

		tuntap_logf(TUNTAP_LOG_DEBUG, "Using fake DHCP to set address: %s/%d", IPv4Address, PrefixLength);

		IPADDR psock[4];
		DWORD len;

		/* We will answer DHCP requests with a reply to set IP/subnet to these values */
		psock[0] = s->S_un.S_addr;
		psock[1] = mask;
		/* DHCP server address */
		psock[2] = 0; /* We don't want it to answer anyone else */
		/* DHCP lease time */
		psock[3] = 3600; /* seconds */

		if (DeviceIoControl(dev->tun_fd, TAP_IOCTL_CONFIG_DHCP_MASQ, &psock, sizeof(psock), &psock, sizeof(psock), &len, NULL) == 0) {
			int error = GetLastError();
			tuntap_log_winerr(TUNTAP_LOG_ERR, error);
			return -1;
		}

		/* Reinitialize the adapters IP address */
		IPADDR ipAddr[3];
		ipAddr[0] = s->S_un.S_addr;
		ipAddr[1] = ipAddr[2] = 0;
		if (0 == DeviceIoControl(dev->tun_fd, TAP_IOCTL_CONFIG_TUN, &ipAddr, sizeof ipAddr, &ipAddr, sizeof ipAddr, &dwLen, NULL)) {
			DWORD error = GetLastError();
			tuntap_log_winerr(TUNTAP_LOG_ERR, error);
			return -1;
		}

	} else {
		/* The adapter is not set for autoconfiguration, but in admin mode the following may work */
		PMIB_UNICASTIPADDRESS_TABLE pipTable = NULL;
		ret = GetUnicastIpAddressTable(AF_INET, &pipTable);
		if (NO_ERROR != ret) {
			tuntap_log_winerr(TUNTAP_LOG_ERR, ret);
			return -1;
		}

		MIB_UNICASTIPADDRESS_ROW ipUni;
		InitializeUnicastIpAddressEntry(&ipUni);
		ipUni.InterfaceIndex = sysdev->index;
		ipUni.Address.Ipv4.sin_family = AF_INET;

		/* Find out if we need admin privilege to change IP address */
		size_t num_addresses = 0;
		int address_not_set = 1;
		for (DWORD n = 0; n < pipTable->NumEntries; ++n) {
			if (pipTable->Table[n].InterfaceIndex == sysdev->index) {
				++num_addresses;
				if (pipTable->Table[n].Address.Ipv4.sin_addr.S_un.S_addr == s->S_un.S_addr) {
					address_not_set = 0;
				}
			}
		}

		/* Addess not already, we need to run as admin */
		if (1 != num_addresses || address_not_set) {
			tuntap_logf(TUNTAP_LOG_DEBUG, "Setting address to %s/%d", IPv4Address, PrefixLength);
			/* Remove all current IPv4 addresses from adapter */
			for (DWORD n = 0; n < pipTable->NumEntries; ++n) {
				if (pipTable->Table[n].InterfaceIndex == sysdev->index) {
					inet_ntop(AF_INET, &pipTable->Table[n].Address.Ipv4.sin_addr.S_un.S_addr, IPv4Address, sizeof IPv4Address);
					tuntap_logf(TUNTAP_LOG_DEBUG, "Remove: %s", IPv4Address);
					ipUni.Address.Ipv4.sin_addr.S_un.S_addr = pipTable->Table[n].Address.Ipv4.sin_addr.S_un.S_addr;
					ret = DeleteUnicastIpAddressEntry(&ipUni);
					if (ERROR_SUCCESS != ret) {
						tuntap_log_winerr(TUNTAP_LOG_ERR, ret);
						return -1;
					}
				}
			}

			FreeMibTable(pipTable);

			/* Add the new address */
			ipUni.Address.Ipv4.sin_addr.S_un.S_addr = s->S_un.S_addr;
			ipUni.OnLinkPrefixLength = PrefixLength;
			ipUni.PrefixOrigin = IpPrefixOriginOther;
			ipUni.SuffixOrigin = IpSuffixOriginOther;
			ipUni.DadState = NldsPreferred;
			ret = CreateUnicastIpAddressEntry(&ipUni);
			if (ERROR_OBJECT_ALREADY_EXISTS != ret && ERROR_SUCCESS != ret) {
				tuntap_log_winerr(TUNTAP_LOG_ERR, ret);
				return -1;
			}

		} else {
			tuntap_logf(TUNTAP_LOG_DEBUG, "Address already at %s/%d", IPv4Address, PrefixLength);
		}

		/* Reinitialize the adapters IP address */
		IPADDR ipAddr[3];
		ipAddr[0] = s->S_un.S_addr;
		ipAddr[1] = ipAddr[2] = 0;
		if (0 == DeviceIoControl(dev->tun_fd, TAP_IOCTL_CONFIG_TUN, &ipAddr, sizeof ipAddr, &ipAddr, sizeof ipAddr, &dwLen, NULL)) {
			DWORD error = GetLastError();
			tuntap_log_winerr(TUNTAP_LOG_ERR, error);
			return -1;
		}

		tuntap_log(TUNTAP_LOG_DEBUG, "Opened TUN mode");
	}

	return 0;
}

int
tuntap_sys_set_ipv6(struct device *dev, t_tun_in6_addr *s, uint32_t mask)
{ /* TODO: implement */
	(void)dev;
	(void)s;
	(void)mask;
	tuntap_log(TUNTAP_LOG_NOTICE, "Your system does not support tuntap_sys_set_ipv6()");
	return -1;
}

int
tuntap_read(struct device *dev, void *buf, size_t size)
{
	OVERLAPPED overlapped;
	memset(&overlapped, 0, sizeof(OVERLAPPED));

	BOOL ok = ReadFile(dev->tun_fd, buf, (DWORD)size, NULL, &overlapped);

	int errcode = GetLastError();
	if (!ok && errcode != ERROR_IO_PENDING) {
		tuntap_log_winerr(TUNTAP_LOG_ERR, errcode);
		return -1;
	}

	DWORD len;
	if (!GetOverlappedResult(dev->tun_fd, &overlapped, &len, TRUE)) {
		tuntap_log_winerr(TUNTAP_LOG_ERR, errcode);
		return -1;
	}

	return (int)len;
}

static int
wait_completion_or_timeout(HANDLE fd, OVERLAPPED *overlapped, int timeout_ms)
{
	// An operation is pending (either a read or a write),
	// so wait until either it completes or the timeout
	// triggers. If timeout_ms < 0, then don't set a timeout.

	DWORD timeout;
	if (timeout_ms < 0) {
		timeout = INFINITE;
	} else {
		timeout = timeout_ms;
	}

	DWORD len;
	BOOL ok = GetOverlappedResultEx(fd, overlapped, &len, timeout, FALSE);
	if (!ok) {
		int errcode = GetLastError();
		if (errcode == ERROR_IO_INCOMPLETE || errcode == WAIT_TIMEOUT) {
			CancelIo(fd);
		} else {
			tuntap_log_winerr(TUNTAP_LOG_ERR, errcode);
		}
		return -1;
	}

	return (int)len;
}

int
tuntap_read_tm(struct device *dev, void *buf, size_t size, int timeout_ms)
{
	BOOL ok;
	DWORD len;
	OVERLAPPED overlapped;
	memset(&overlapped, 0, sizeof(overlapped));

	ok = ReadFile(dev->tun_fd, buf, (DWORD)size, &len, &overlapped);
	if (ok) {
		return (int)len; // Operation resolved immediately
	}

	int errcode = GetLastError();
	if (errcode != ERROR_IO_PENDING) {
		// Couldn't start the read operation
		tuntap_log_winerr(TUNTAP_LOG_ERR, errcode);
		return -1;
	}

	return wait_completion_or_timeout(dev->tun_fd, &overlapped, timeout_ms);
}

int
tuntap_write(struct device *dev, void *buf, size_t size)
{
	OVERLAPPED overlapped;
	memset(&overlapped, 0, sizeof(OVERLAPPED));

	BOOL ok = WriteFile(dev->tun_fd, buf, (DWORD)size, NULL, &overlapped);

	int errcode = GetLastError();
	if (!ok && errcode != ERROR_IO_PENDING) {
		tuntap_log_winerr(TUNTAP_LOG_ERR, errcode);
		return -1;
	}

	DWORD len;
	if (!GetOverlappedResult(dev->tun_fd, &overlapped, &len, TRUE)) {
		tuntap_log_winerr(TUNTAP_LOG_ERR, errcode);
		return -1;
	}

	return (int)len;
}

int
tuntap_write_tm(struct device *dev, void *buf, size_t size, int timeout_ms)
{
	int errcode;
	BOOL ok;
	DWORD len;
	OVERLAPPED overlapped;
	memset(&overlapped, 0, sizeof(overlapped));

	ok = WriteFile(dev->tun_fd, buf, (DWORD)size, &len, &overlapped);
	if (ok) {
		return (int)len; // Write operation completed immediately
	}

	errcode = GetLastError();
	if (errcode != ERROR_IO_PENDING) {
		// Couldn't start write operation at all
		tuntap_log_winerr(TUNTAP_LOG_ERR, errcode);
		return -1;
	}

	return wait_completion_or_timeout(dev->tun_fd, &overlapped, timeout_ms);
}

int
tuntap_get_readable(struct device *dev)
{
	(void)dev;
	tuntap_log(TUNTAP_LOG_NOTICE, "Your system does not support tuntap_get_readable()");
	return -1;
}

int
tuntap_set_nonblocking(struct device *dev, int set)
{
	(void)dev;
	(void)set;
	tuntap_log(TUNTAP_LOG_NOTICE, "Your system does not support tuntap_set_nonblocking()");
	return -1;
}

int
tuntap_set_debug(struct device *dev, int set)
{
	(void)dev;
	(void)set;
	tuntap_log(TUNTAP_LOG_NOTICE, "Your system does not support tuntap_set_debug()");
	return -1;
}

int
tuntap_set_descr(struct device *dev, const char *descr)
{
	(void)dev;
	(void)descr;
	tuntap_log(TUNTAP_LOG_NOTICE, "Your system does not support tuntap_set_descr()");
	return -1;
}

int
tuntap_set_ifname(struct device *dev, const char *name)
{
	/* NOTE: Unfortunatley there is no easy interface to set the RFC2863 ifAlias  */
	/* address, by C code, so the user is expected to do it by external means,    */
	/* e.g. by using Rename-NetAdapter -Name "tunX" -NewName "tunY" of powershell */
	/* or by using netadapter panel by renaming the adapter.                      */
	(void)dev;
	(void)name;
	if (0 == strncmp(dev->if_name, name, sizeof dev->if_name)) {
		tuntap_log(TUNTAP_LOG_DEBUG, "if_name already set");
		return 0;
	} else {
		tuntap_log(TUNTAP_LOG_NOTICE, "Your system does not support tuntap_set_ifname()");
		return -1;
	}
	return -1;
}

char *
tuntap_get_descr(struct device *dev)
{
	system_device *sysdev = dev->sys;
	return sysdev->description;
}
