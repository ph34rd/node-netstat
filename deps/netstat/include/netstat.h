#ifndef NODE_NETSTAT_H
#define NODE_NETSTAT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <arpa/inet.h>

struct node_netstat_iface_s {
	char* name;
	char phys_addr[6];
	int is_internal;
	size_t ibytes;
	size_t obytes;
};
typedef struct node_netstat_iface_s node_netstat_iface_t;

struct node_netstat_iaddress_s {
	char* name;
	union {
		struct sockaddr_in address4;
		struct sockaddr_in6 address6;
	} address;
	union {
		struct sockaddr_in netmask4;
		struct sockaddr_in6 netmask6;
	} netmask;
	int family;
	int is_internal;
};
typedef struct node_netstat_iaddress_s node_netstat_iaddress_t;

int node_netstat_interface_addresses(node_netstat_iface_t** ifaces, node_netstat_iaddress_t** addresses, uint* i_count, uint* a_count);
void node_netstat_free_interface_addresses(node_netstat_iface_t* ifaces, node_netstat_iaddress_t* addresses, uint i_count, uint a_count);

#ifdef __cplusplus
}
#endif

#endif /* NODE_NETSTAT_H */
