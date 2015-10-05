#include <node.h>
#include <nan.h>
#include <cstdlib>
#include "get.h"
#include "netstat.h"

using v8::Array;
using v8::Number;
using v8::Local;
using v8::Object;
using v8::String;

#define MAC_SIZE 18
#define MAC_TPL "%02x:%02x:%02x:%02x:%02x:%02x"

NAN_METHOD(get) {
	NanScope();

	node_netstat_iaddress_t* addresses;
	node_netstat_iface_t* ifaces;
	uint i_count, a_count, i, j;
	char ip[INET6_ADDRSTRLEN];
	char netmask[INET6_ADDRSTRLEN];
	char mac[MAC_SIZE];

	Local<Object> aobj, ifobj;
	Local<String> family;
	Local<Array> ret, ifarr;

	// create return value array
	ret = NanNew<Array>();

	int err = node_netstat_interface_addresses(&ifaces, &addresses, &i_count, &a_count);

	if (err) {
		NanReturnValue(ret);
	}

	// iterate ifaces
	for (i = 0; i < i_count; i++) {
		ifobj = NanNew<Object>();

		// prepare mac
		snprintf(mac,
			 MAC_SIZE,
			 MAC_TPL,
			 static_cast<unsigned char>(ifaces[i].phys_addr[0]),
			 static_cast<unsigned char>(ifaces[i].phys_addr[1]),
			 static_cast<unsigned char>(ifaces[i].phys_addr[2]),
			 static_cast<unsigned char>(ifaces[i].phys_addr[3]),
			 static_cast<unsigned char>(ifaces[i].phys_addr[4]),
			 static_cast<unsigned char>(ifaces[i].phys_addr[5]));

		ifobj->Set(NanNew<String>("name"), NanNew<String>(ifaces[i].name));
		ifobj->Set(NanNew<String>("mac"), NanNew<String>(mac));
		ifobj->Set(NanNew<String>("ibytes"), NanNew<Number>(ifaces[i].ibytes));
		ifobj->Set(NanNew<String>("obytes"), NanNew<Number>(ifaces[i].obytes));
		ifobj->Set(NanNew<String>("internal"), ifaces[i].is_internal ? NanTrue() : NanFalse());

		ifarr = NanNew<Array>();

		// iterate addresses
		for (j = 0; j < a_count; j++) {
			if (strcmp(ifaces[i].name, addresses[j].name) == 0) {
				aobj = NanNew<Object>();

				if (addresses[j].address.address4.sin_family == AF_INET) {
					uv_ip4_name(&addresses[j].address.address4, ip, sizeof(ip));
					uv_ip4_name(&addresses[j].netmask.netmask4, netmask, sizeof(netmask));
					family = NanNew<String>("ipv4");
				} else if (addresses[j].address.address4.sin_family == AF_INET6) {
					uv_ip6_name(&addresses[j].address.address6, ip, sizeof(ip));
					uv_ip6_name(&addresses[j].netmask.netmask6, netmask, sizeof(netmask));
					family = NanNew<String>("ipv6");
				} else {
					strncpy(ip, "<unknown sa family>", INET6_ADDRSTRLEN);
					family = NanNew<String>("unknown");
				}

				aobj->Set(NanNew<String>("address"), NanNew<String>(ip));
				aobj->Set(NanNew<String>("netmask"), NanNew<String>(netmask));
				aobj->Set(NanNew<String>("family"), family);
				ifarr->Set(ifarr->Length(), aobj);
			}
		}

		ifobj->Set(NanNew<String>("addresses"), ifarr);
		ret->Set(ret->Length(), ifobj);
	}

	node_netstat_free_interface_addresses(ifaces, addresses, i_count, a_count);

	NanReturnValue(ret);
}
