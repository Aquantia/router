/******************************************************************************
*                    Copyright 2018 aQuantia Corporation
*                    Confidential and Proprietary
******************************************************************************/

#ifndef _SESSION_H

#define _SESSION_H

#include <time.h>
#include <string>
#include <map>
#include <memory>
#include <netinet/in.h>
#include "TrafficClassifier.h"
#include "RuleFlow.h"

class Session
{
public:
	static const int SESSION_EXPIRATION_TIME = 120;

public:
	Session(uint32_t ipAddr);
	~Session();
	void close();
	void poll();
	void setToken(const std::string &token);
	bool validateToken(const std::string &token);
	bool isExpired();
	bool isAthorized();

	std::map<RULE_FLOW_KEY, RULE_FLOW> m_arrFlows;

private:
	uint64_t m_hwAddr;
	std::string m_szToken;
	time_t m_tExpireTime;	
};

#endif