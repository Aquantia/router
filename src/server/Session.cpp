/******************************************************************************
*                    Copyright 2018 aQuantia Corporation
*                    Confidential and Proprietary
******************************************************************************/

#include "Session.h"
#include "Log.h"
#include "Neighbour.h"
#include "RuleMac.h"


Session::Session(uint32_t ip_addr) : m_szToken("")
{
	ULOG_NOTE("Session create\n");
	std::map<unsigned int, unsigned long long> arpTable  = getArpTable();
	
	auto it = arpTable.find(ip_addr);
	if (it == arpTable.end())
	{
		ULOG_NOTE("Cant find MAC for ip %u\n", ip_addr);
	}
	else
	{
		m_hwAddr = it->second;
		ruleMacDisableNeighbour(m_hwAddr);
	}
	poll();
}

Session::~Session()
{
	close();
	ULOG_NOTE("Session destroy\n");
}

void Session::close()
{
	ruleMacEnableNeighbour(m_hwAddr);
	m_arrFlows.clear();
	ULOG_NOTE("Session close\n");
}

void Session::setToken(const std::string &token)
{
	m_arrFlows.clear();
	poll();
	m_szToken = token;
	ULOG_NOTE("Token set\n");
}

bool Session::validateToken(const std::string &token)
{
	return !m_szToken.empty() && m_szToken.compare(token) == 0;
}

bool Session::isAthorized()
{
	return !m_szToken.empty();
}

bool Session::isExpired()
{
	return m_tExpireTime <= time(NULL);
}

void Session::poll()
{
	m_tExpireTime = time(NULL) + SESSION_EXPIRATION_TIME;	
}
