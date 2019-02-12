/******************************************************************************
*                    Copyright 2018 aQuantia Corporation
*                    Confidential and Proprietary
******************************************************************************/

#ifndef _NEIGHBOUR_H

#define _NEIGHBOUR_H

#include <string>
#include <map>

std::string neigHandleRequest(const std::string method, const std::string &request);
std::map <std::string, std::string>  getDhcpNamesByIp(std::string fileName);
std::map<unsigned int, unsigned long long> getArpTable();

#endif