/******************************************************************************
*                    Copyright 2018 aQuantia Corporation
*                    Confidential and Proprietary
******************************************************************************/

#ifndef _TRAFFIC_CLASSIFIER_H

#define _TRAFFIC_CLASSIFIER_H

#include <netlink/route/classifier.h>
#include "TrafficController.h"

class TrafficController;

class TrafficClassifier
{
public:
	TrafficClassifier(std::shared_ptr<TrafficController> tc, struct rtnl_cls *pFilter);
	~TrafficClassifier();
	struct rtnl_cls* getFilter();
	void remove();
	void freeFilter();
	static bool fastRemoveFlag;
private:
	struct rtnl_cls *m_pFilter;
	std::shared_ptr<TrafficController> m_TrafficController;

};

#endif