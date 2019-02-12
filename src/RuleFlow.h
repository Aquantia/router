/******************************************************************************
*                    Copyright 2018 aQuantia Corporation
*                    Confidential and Proprietary
******************************************************************************/

#ifndef _RULE_FLOW

#define _RULE_FLOW

#include <string>
#include "Session.h"

class Session;

typedef struct tagRULE_FLOW_KEY {
	struct in_addr src_addr;
	unsigned short src_port;
	struct in_addr dst_addr;
	unsigned short dst_port;
	unsigned char  proto;

	bool operator< (tagRULE_FLOW_KEY const &s) const
    {
        if (src_addr.s_addr < s.src_addr.s_addr)
            return true;
        else if (src_addr.s_addr > s.src_addr.s_addr)
            return false;

        if (dst_addr.s_addr < s.dst_addr.s_addr)
            return true;
        else if (dst_addr.s_addr > s.dst_addr.s_addr)
            return false;

        if (src_port < s.src_port)
            return true;
        else if (src_port > s.src_port)
            return false;

        if (dst_port < s.dst_port)
            return true;
        else if (dst_port > s.dst_port)
            return false;

        return false;
    }

} RULE_FLOW_KEY;

typedef struct {
	RULE_FLOW_KEY key;
	int priority;	
	std::shared_ptr<TrafficClassifier> classifier;
} RULE_FLOW;

std::string ruleFlowHandleRequest(const std::string& method, const std::string &request, Session &session);

#endif
