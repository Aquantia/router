/******************************************************************************
*                    Copyright 2018 aQuantia Corporation
*                    Confidential and Proprietary
******************************************************************************/

#include <sstream>
#include "TrafficClassifier.h"
#include "Log.h"
#include "AQService.h"

bool TrafficClassifier::fastRemoveFlag = false;

TrafficClassifier::TrafficClassifier(std::shared_ptr<TrafficController> tc, struct rtnl_cls *pFilter) : m_TrafficController(tc), m_pFilter(pFilter)
{
	ULOG_NOTE("Add classifier, filter pointer %u\n", m_pFilter);
}

TrafficClassifier::~TrafficClassifier()
{
	ULOG_NOTE("Remove classifier\n");
	freeFilter();
	remove();
	rtnl_cls_put(m_pFilter);
	m_pFilter = nullptr;
}

void TrafficClassifier::remove()
{
	if (m_pFilter) {
		int ret = rtnl_cls_delete(m_TrafficController->m_pSock, m_pFilter, 0);

		if (ret  && !TrafficClassifier::fastRemoveFlag)
		{
			uint32_t filterIndex = rtnl_tc_get_handle(TC_CAST(m_pFilter));
			std::stringstream stream;
			stream << "tc filter del dev "<< pService->m_Settings.ethNameFilter << " parent 1: protocol ip prio 10 handle 800::" << std::hex << filterIndex << " u32";
			
			system(stream.str().c_str());
			ULOG_NOTE("remove: system execute %s\n", stream.str().c_str());
		}
	}
}

void TrafficClassifier::freeFilter()
{
	if (m_pFilter) 
	{
		int filterIndex = rtnl_tc_get_handle(TC_CAST(m_pFilter));
		m_TrafficController->setFreeFilterIndex(filterIndex);
	}
	else
	{
		ULOG_NOTE("TrafficClassifier::freeFilter invalid filter pointer\n");
	}
}

struct rtnl_cls* TrafficClassifier::getFilter()
{
	return m_pFilter;
};