/******************************************************************************
*                    Copyright 2018 aQuantia Corporation
*                    Confidential and Proprietary
******************************************************************************/

#include <errno.h>
#include <net/ethernet.h>
#include <netlink/route/neighbour.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "json-c/json.h"
#include "Neighbour.h"
#include "AQService.h"

#include <iostream>
#include <sstream>
#include <map>
#include <fstream>
#include <sstream>
#include <vector>

std::map <std::string, std::string>  getDhcpNamesByIp(std::string fileName)
{
	std::ifstream ifs(fileName, std::ifstream::in);
	std::map <std::string, std::string> result;
	std::string line;
	
	if (ifs.is_open())
	{
		try
		{
			while (std::getline(ifs, line))
			{
				std::string token;
				std::stringstream ss(line);
				std::vector<std::string> tokens;
				while (std::getline(ss, token, ' '))
				{
					tokens.push_back(token);
				}
				result.insert(std::pair<std::string, std::string> (tokens.at(2), tokens.at(3)));
			}
		}
		catch (std::exception e)
		{
		}
	}
	return result;
}


struct neighParserData
{
	std::map <std::string, std::string>  dhcpMap;
	json_object * rootObject;
};

struct nl_addr * getInterestingMark(struct nl_object *obj)
{
	struct rtnl_neigh *neigh = (struct rtnl_neigh *)obj;
	
	int state = rtnl_neigh_get_state(neigh);
	int flags = rtnl_neigh_get_flags(neigh);
	int if_index = rtnl_neigh_get_ifindex(neigh);
	int adapter_if_index = pService->m_tc->getNIfIndex();

	if ((if_index != adapter_if_index) || (state == NUD_NONE) || (flags & NTF_ROUTER) || (state == NUD_NOARP) || (state == NUD_FAILED)) 
	{
		return nullptr;
	}
	
	struct nl_addr *ll_addr = rtnl_neigh_get_lladdr(neigh);
	
	if (!ll_addr)
	{
		return nullptr;
	}

	int family = nl_addr_get_family(ll_addr);
	int len = nl_addr_get_len(ll_addr);
	if ((family == AF_LLC) && (len == ETHER_ADDR_LEN))
	{
		return ll_addr;
	}
	else
	{
		return nullptr;
	}
}


void iterateOverArpEntries(void * userData, void(*callback)(struct nl_object *, void *))
{ 
	struct nl_cache *cache;

	struct nl_sock *sock = nl_socket_alloc();
	if (sock != NULL) 
	{
		int ret = nl_connect(sock, NETLINK_ROUTE);
		if (ret == 0) 
		{
			ret = rtnl_neigh_alloc_cache(sock, &cache);
			if (ret == 0)
			{
				nl_cache_foreach(cache, callback, userData);
				nl_cache_free(cache);
			}

			nl_close(sock);
		}
		nl_socket_free(sock);
	}
}


static void neigh_parser(struct nl_object *obj, void *user)
{
	struct rtnl_neigh * neigh = reinterpret_cast<struct rtnl_neigh*>(obj);
	struct neighParserData * data = static_cast<struct neighParserData*> (user);
	json_object *dataObj = data->rootObject;
	
	struct nl_addr *ll_addr  = getInterestingMark(obj);

	if (ll_addr) 
	{

		struct nl_addr *dst_addr = rtnl_neigh_get_dst(neigh);

		int family = nl_addr_get_family(dst_addr);

		if (family == AF_INET) 
		{
			json_object *neighObj = json_object_new_object();

			const char *hwAddr = (const char *)nl_addr_get_binary_addr(ll_addr);
			char hwAddrSz[18];
			snprintf(hwAddrSz, sizeof(hwAddrSz), "%02hhX-%02hhX-%02hhX-%02hhX-%02hhX-%02hhX", hwAddr[0] & 0xFF, hwAddr[1] & 0xFF, hwAddr[2] & 0xFF, hwAddr[3] & 0xFF, hwAddr[4] & 0xFF, hwAddr[5] & 0xFF);

			json_object *addr = json_object_new_string(hwAddrSz);
			json_object_object_add(neighObj, "hwAddr", addr);

			const char *ipAddr = (const char *)nl_addr_get_binary_addr(dst_addr);
			char ipAddrSz[18];
			snprintf(ipAddrSz, sizeof(ipAddrSz), "%hhu.%hhu.%hhu.%hhu", ipAddr[0] & 0xFF, ipAddr[1] & 0xFF, ipAddr[2] & 0xFF, ipAddr[3] & 0xFF);

			json_object *addrObj = json_object_new_string(ipAddrSz);
			json_object_object_add(neighObj, "ipAddr", addrObj);
			
			in_addr_t ip_addr = inet_addr(ipAddrSz);

			bool isAqcc = false;
			auto session = pService->m_Rest.m_arrSessions.find(ip_addr);

			if (session != pService->m_Rest.m_arrSessions.end())
			{
				isAqcc = session->second.isAthorized();
			}

			json_object *aqccObj = json_object_new_boolean(isAqcc);
			
			json_object_object_add(neighObj, "aqcc", aqccObj);

			struct hostent *host =  gethostbyaddr(ipAddr, 4, AF_INET);
			json_object *hostnameObj = NULL;
			if (host)

			{
				if (strcmp(host->h_name, ipAddrSz))
				{
					hostnameObj = json_object_new_string(host->h_name);
				}
				else
				{
					for (int i = 0; host->h_aliases[i]; i++)
					{
						if (strcmp(host->h_aliases[i], ipAddrSz))
						{
							hostnameObj = json_object_new_string(host->h_aliases[i]);
							break ;
						}
					}
				}
			}
			if (!hostnameObj)
			{
				decltype(data->dhcpMap)::iterator it;
				if ((it = data->dhcpMap.find(ipAddrSz)) != data->dhcpMap.end())
				{
					hostnameObj = json_object_new_string(it->second.c_str());
				}
				else
				{
					hostnameObj = json_object_new_string("");
				}
			}
			json_object_object_add(neighObj, "host", hostnameObj);
			json_object_array_add(dataObj, neighObj);
		}
	}
}


std::string neigHandleRequest(const std::string method, const std::string &request)
{
	std::string response;
	json_object *jsonRoot = json_object_new_object();
	json_object *successObj = NULL;
	
	neighParserData data;
	data.dhcpMap = getDhcpNamesByIp("/tmp/dhcp.leases");

	if (method.compare("GET") == 0) 
	{

		successObj = json_object_new_boolean(true);
		data.rootObject = json_object_new_array();
		iterateOverArpEntries(static_cast<void*>(&data), neigh_parser);
	}

	if (successObj)
		json_object_object_add(jsonRoot, "success", successObj);

	if (data.rootObject) 
		json_object_object_add(jsonRoot, "data", data.rootObject);	

	const char *jsonStr = json_object_to_json_string(jsonRoot);

	response = std::string(jsonStr);

	json_object_put(jsonRoot);

	return response ;
}


static void neigh_parser_fast(struct nl_object *obj, void *user)
{
	struct nl_addr *ll_addr;
	struct rtnl_neigh *neigh = (struct rtnl_neigh *)obj;
	if (ll_addr = getInterestingMark(obj)) 
	{
		
		std::map < unsigned int,
		unsigned long long > * data = static_cast < std::map < unsigned int,
		unsigned long long > * > (user);

		struct nl_addr *dst_addr = rtnl_neigh_get_dst(neigh);

		int family = nl_addr_get_family(dst_addr);
		int  len = nl_addr_get_len(dst_addr);	

		if (family == AF_INET) 
		{

			json_object *neighObj = json_object_new_object();

			unsigned long long *hwAddr = (unsigned long long *)nl_addr_get_binary_addr(ll_addr);
			unsigned int *ipAddr = (unsigned int *)nl_addr_get_binary_addr(dst_addr);
			data->insert(std::pair <uint32_t, uint64_t> (*ipAddr, *hwAddr & ((1ULL << 48) - 1)));
		}
	}
}


std::map<unsigned int, unsigned long long> getArpTable()
{
	struct nl_cache *cache;
	std::map < unsigned int,unsigned long long> result;  
	
	
	iterateOverArpEntries(&result, neigh_parser_fast);

	return result ;
}
