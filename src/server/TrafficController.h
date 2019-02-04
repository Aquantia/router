/******************************************************************************
*                    Copyright 2018 aQuantia Corporation
*                    Confidential and Proprietary
******************************************************************************/

#ifndef _TRAFFIC_CONTROLLER_H

#define _TRAFFIC_CONTROLLER_H

#include <string>
#include <memory>
#include <netlink/route/classifier.h>
#include <stack>
#include "TrafficClassifier.h"

class TrafficClassifier;

class TrafficController
{
	friend TrafficClassifier;

public:

	enum {
		eTrafficClassCriticalPriority = 1,
		eTrafficClassHighPriority     = 2,
		eTrafficClassNormalPriority   = 3,
		eTrafficClassLowPriority      = 4
	};

	static constexpr double BANDWIDTH_MULTIPLIER = 0.9f;

	class IdStack: public std::stack<int>
	{
	
	public:
		enum {
			eMaxFilterIndex = 0x7ff
		};

		IdStack()
		{
			for(int i = eMaxFilterIndex; i >= 1; i--)
			{
				push(i);
			}
		}

		int topPop()
		{
			if(empty())
				return -1;

			int id = top();
			pop();
			return id;
		}
	};

public:
	TrafficController();
	~TrafficController();

	int init(const std::string &ethName, unsigned int rateLimit);

	std::shared_ptr<TrafficClassifier> addFlowClassifier(unsigned int src_addr, unsigned short src_port, 
			unsigned int dst_addr, unsigned short dst_port, 
			unsigned char proto, int classId);
	bool modifyFlowClassifier(std::shared_ptr<TrafficClassifier> classifier, int newClassId);

	std::shared_ptr<TrafficClassifier> addMacClassifier(char hwAddr[6], int classId);
	std::shared_ptr<TrafficClassifier> addGateWayClassifier(std::shared_ptr<TrafficController> tc, unsigned int selfAddress);
	bool modifyMacClassifier(std::shared_ptr<TrafficClassifier> classifier, int newClassId);
	bool setSpeedLimit(unsigned int downloadLimit);

	int getNIfIndex ();

	int  getFreeFilterIndex();
	void setFreeFilterIndex(int freeFilterIndex);

private:
	void clear();

	void setTrafficClass(struct rtnl_cls *, int trafficClass);
	struct nl_sock *m_pSock;
	struct rtnl_link * m_pLink;
	struct nl_cache *m_pCache;
	struct rtnl_qdisc *m_pRoot;
	struct rtnl_class *m_pGlobalShaper;
	struct rtnl_class *m_pNonRealtimeShaper;
	struct rtnl_class *m_pRealtimeShaper;
	struct rtnl_class *m_pHighShaper;
	struct rtnl_class *m_pNormalShaper;
	int m_bInitDone;
	int m_nIfIndex;

	static IdStack m_idStack;
};

#endif
