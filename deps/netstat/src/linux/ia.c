#include "netstat.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include <net/if.h>
#include <sys/param.h>
#include <sys/prctl.h>
#include <sys/sysinfo.h>
#include <linux/if_link.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#define HAVE_IFADDRS_H 1

#ifdef __UCLIBC__
# if __UCLIBC_MAJOR__ < 0 || __UCLIBC_MINOR__ < 9 || __UCLIBC_SUBLEVEL__ < 32
#  undef HAVE_IFADDRS_H
# endif
#endif

#ifdef HAVE_IFADDRS_H
# if defined(__ANDROID__)
#  include "android-ifaddrs.h"
# else
#  include <ifaddrs.h>
# endif
# include <sys/socket.h>
# include <net/ethernet.h>
# include <linux/if_packet.h>
#endif /* HAVE_IFADDRS_H */

/* chech iface is down */
static int if_down(struct ifaddrs *entry) {
	return ((!((entry->ifa_flags & IFF_UP) && (entry->ifa_flags & IFF_RUNNING))) ||
			(entry->ifa_addr == NULL));
}

int node_netstat_interface_addresses(node_netstat_iface_t** ifaces, node_netstat_iaddress_t** addresses, uint* i_count, uint* a_count) {
#ifndef HAVE_IFADDRS_H
	return -ENOSYS;
#else
	struct ifaddrs *addrs, *entry;
	node_netstat_iaddress_t* address; /* pointer to list head */
	node_netstat_iface_t* iface; /* pointer to list head */
	struct rtnl_link_stats *stats;
	struct sockaddr_ll *sll;

	if (getifaddrs(&addrs) == -1) {
		return -errno;
	}

	*i_count = 0;
	*a_count = 0;
	/* walk through addrs list & count */
	for (entry = addrs; entry != NULL; entry = entry->ifa_next) {
		if (if_down(entry)) {
			continue;
		}

		if (entry->ifa_addr->sa_family == PF_PACKET) {
			(*i_count)++;
		} else {
			(*a_count)++;
		}
	}

	/* allocate mem */
	*addresses = malloc(*a_count * sizeof(**addresses));
	if (!(*addresses)) {
		return -ENOMEM;
	}
	address = *addresses;

	/* allocate mem */
	*ifaces = malloc(*i_count * sizeof(**ifaces));
	if (!(*ifaces)) {
		free(*addresses);
		return -ENOMEM;
	}
	iface = *ifaces;

	/* walk through addrs list */
	for (entry = addrs; entry != NULL; entry = entry->ifa_next) {
		if (if_down(entry)) {
			continue;
		}

		if (entry->ifa_addr->sa_family == PF_PACKET) {
			/* copy name */
			iface->name = strdup(entry->ifa_name);

			/* copy physical address */
			sll = (struct sockaddr_ll*)entry->ifa_addr;
			memcpy(iface->phys_addr, sll->sll_addr, sizeof(iface->phys_addr));

			/* loopback flag */
			iface->is_internal = !!(entry->ifa_flags & IFF_LOOPBACK);

			if (entry->ifa_data != NULL) {
				stats = entry->ifa_data;
				/* copy stats data */
				iface->ibytes = stats->tx_bytes;
				iface->obytes = stats->rx_bytes;
			} else {
				iface->ibytes = 0;
				iface->obytes = 0;
			}

			iface++;
		} else {
			/* copy name */
			address->name = strdup(entry->ifa_name);

			/* copy address */
			if (entry->ifa_addr->sa_family == AF_INET6) {
				 address->address.address6 = *((struct sockaddr_in6*) entry->ifa_addr);
			} else if (entry->ifa_addr->sa_family == AF_INET) {
				 address->address.address4 = *((struct sockaddr_in*) entry->ifa_addr);
			}

			/* copy netmask */
			if (entry->ifa_netmask->sa_family == AF_INET6) {
				address->netmask.netmask6 = *((struct sockaddr_in6*) entry->ifa_netmask);
			} else if (entry->ifa_netmask->sa_family == AF_INET)  {
				address->netmask.netmask4 = *((struct sockaddr_in*) entry->ifa_netmask);
			}

			/* loopback flag */
			address->is_internal = !!(entry->ifa_flags & IFF_LOOPBACK);
			address++;
		}
	}

	freeifaddrs(addrs);
	return 0;
#endif
}

void node_netstat_free_interface_addresses(node_netstat_iface_t* ifaces, node_netstat_iaddress_t* addresses, uint i_count, uint a_count) {
	uint i;

	for (i = 0; i < i_count; i++) {
		free(ifaces[i].name);
	}

	for (i = 0; i < a_count; i++) {
		free(addresses[i].name);
	}

	free(ifaces);
	free(addresses);
}
