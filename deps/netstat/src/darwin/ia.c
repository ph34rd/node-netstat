#include "netstat.h"

#include <assert.h>
#include <string.h>
#include <errno.h>

#include <ifaddrs.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <sys/user.h>
#include <sys/types.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

/* chech iface is down */
static int if_down(struct ifaddrs *entry) {
	return ((!((entry->ifa_flags & IFF_UP) && (entry->ifa_flags & IFF_RUNNING))) ||
			(entry->ifa_addr == NULL));
}

int node_netstat_interface_addresses(node_netstat_iface_t** ifaces, node_netstat_iaddress_t** addresses, uint* i_count, uint* a_count) {
	struct ifaddrs *addrs, *entry;
	node_netstat_iaddress_t* address; /* pointer to list head */
	node_netstat_iface_t* iface; /* pointer to list head */
	struct sockaddr_dl *sdl;
	struct if_data *stats;

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

		if (entry->ifa_addr->sa_family == AF_LINK) {
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

		if (entry->ifa_addr->sa_family == AF_LINK) {
			/* copy name */
			iface->name = strdup(entry->ifa_name);

			/* copy physical address */
			sdl = (struct sockaddr_dl*)entry->ifa_addr;
			memcpy(iface->phys_addr, LLADDR(sdl), sizeof(iface->phys_addr));

			/* loopback flag */
			iface->is_internal = !!(entry->ifa_flags & IFF_LOOPBACK);

			if (entry->ifa_data != NULL) {
				stats = entry->ifa_data;
				/* copy stats data */
				iface->ibytes = stats->ifi_ibytes;
				iface->obytes = stats->ifi_obytes;
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
