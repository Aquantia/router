/******************************************************************************
*                    Copyright 2018 aQuantia Corporation
*                    Confidential and Proprietary
******************************************************************************/

#include <netlink/utils.h>

#define class cls
#include <netlink/route/qdisc/hfsc.h>
#undef class

#include <netlink/route/classifier.h>
#include <netlink/route/cls/u32.h>

#include "TrafficController.h"
#include "Log.h"
#include "AQService.h"

#define CRITICAL_PRIORITY_SHAPER_HANDLE	20
#define NORMAL_PRIORITY_SHAPER_HANDLE	100
#define HIGH_PRIORITY_SHAPER_HANDLE	    200

TrafficController::IdStack TrafficController::m_idStack;

TrafficController::TrafficController() : m_pSock(NULL), m_pLink(NULL), m_pCache(NULL), m_pRoot(NULL), m_pGlobalShaper(NULL), 
				m_pNonRealtimeShaper(NULL), m_pRealtimeShaper(NULL), m_pHighShaper(NULL), m_pNormalShaper(NULL), 
				m_bInitDone(0), m_nIfIndex(0)
{

}

TrafficController::~TrafficController()
{
	clear();
}

int TrafficController::init(const std::string &ethName, unsigned int rateLimit)
{
	ULOG_NOTE("Traffic controller init\n");

	if (m_bInitDone)
		return 0;

	for (;;) {
		int ret = -1;
		m_pSock = nl_socket_alloc();
		if (m_pSock == NULL) {
			ULOG_WARN("TC unable to create socket\n");
			break;	
		}
		
		ret = nl_connect(m_pSock, NETLINK_ROUTE);
		if (ret != 0) {
			ULOG_WARN("TC unable to connect socket\n");
			break;
		}	

		ret = rtnl_link_alloc_cache(m_pSock, AF_UNSPEC, &m_pCache);
		if (ret != 0) {
			ULOG_WARN("TC unable to allocate cache\n");
			break;
		}	

		m_pLink = rtnl_link_get_by_name(m_pCache, ethName.c_str());
		if (m_pLink == NULL) {
			ULOG_WARN("TC unable to get link\n");
			break;	
		}

		m_nIfIndex = rtnl_link_get_ifindex(m_pLink);

		// Root discipline
		m_pRoot = rtnl_qdisc_alloc();
		if (m_pRoot == NULL) {
			ULOG_WARN("TC unable to allocate qdisc\n");
			break;	
		}

		rtnl_tc_set_ifindex(TC_CAST(m_pRoot), m_nIfIndex);
		rtnl_tc_set_parent(TC_CAST(m_pRoot), TC_H_ROOT);
		rtnl_tc_set_handle(TC_CAST(m_pRoot), TC_HANDLE(1, 0));
  		ret = rtnl_tc_set_kind(TC_CAST(m_pRoot), "hfsc"); 
		if (ret != 0)
			break;
  
		rtnl_qdisc_hfsc_set_defcls(m_pRoot, NORMAL_PRIORITY_SHAPER_HANDLE);

		// Global traffic shaper
		m_pGlobalShaper = rtnl_class_alloc();
		if (m_pGlobalShaper == NULL) {
			ULOG_WARN("TC unable to allocate global shaper\n");
			break;	
		}

		rtnl_tc_set_ifindex(TC_CAST(m_pGlobalShaper), m_nIfIndex);
		rtnl_tc_set_parent(TC_CAST(m_pGlobalShaper), rtnl_tc_get_handle(TC_CAST(m_pRoot)));
		rtnl_tc_set_handle(TC_CAST(m_pGlobalShaper), TC_HANDLE(1, 1));
  		ret = rtnl_tc_set_kind(TC_CAST(m_pGlobalShaper), "hfsc"); 
		if (ret != 0)
			break;

		// Non-realtime shaper
		m_pNonRealtimeShaper = rtnl_class_alloc();
		if (m_pNonRealtimeShaper == NULL) {
			ULOG_WARN("TC unable to allocate realtime shaper\n");
			break;	
		}

		rtnl_tc_set_ifindex(TC_CAST(m_pNonRealtimeShaper), m_nIfIndex);
		rtnl_tc_set_parent(TC_CAST(m_pNonRealtimeShaper), rtnl_tc_get_handle(TC_CAST(m_pGlobalShaper)));
		rtnl_tc_set_handle(TC_CAST(m_pNonRealtimeShaper), TC_HANDLE(1, 10));
  		ret = rtnl_tc_set_kind(TC_CAST(m_pNonRealtimeShaper), "hfsc"); 
		if (ret != 0)
			break;

		// Critical shaper
		m_pRealtimeShaper = rtnl_class_alloc();
		if (m_pRealtimeShaper == NULL) {
			ULOG_WARN("TC unable to allocate critical shaper\n");
			break;	
		}

		rtnl_tc_set_ifindex(TC_CAST(m_pRealtimeShaper), m_nIfIndex);
		rtnl_tc_set_parent(TC_CAST(m_pRealtimeShaper), rtnl_tc_get_handle(TC_CAST(m_pGlobalShaper)));
		rtnl_tc_set_handle(TC_CAST(m_pRealtimeShaper), TC_HANDLE(1, CRITICAL_PRIORITY_SHAPER_HANDLE));
  		ret = rtnl_tc_set_kind(TC_CAST(m_pRealtimeShaper), "hfsc"); 
		if (ret != 0)
			break;

		// High-priority traffic shaper
		m_pHighShaper = rtnl_class_alloc();
		if (m_pHighShaper == NULL) {
			ULOG_WARN("TC unable to allocate high shaper\n");
			break;	
		}

		rtnl_tc_set_ifindex(TC_CAST(m_pHighShaper), m_nIfIndex);
		rtnl_tc_set_parent(TC_CAST(m_pHighShaper), rtnl_tc_get_handle(TC_CAST(m_pNonRealtimeShaper)));
		rtnl_tc_set_handle(TC_CAST(m_pHighShaper), TC_HANDLE(1, HIGH_PRIORITY_SHAPER_HANDLE));
  		ret = rtnl_tc_set_kind(TC_CAST(m_pHighShaper), "hfsc"); 
		if (ret != 0)
			break;

		// Normal-priority traffic shaper
		m_pNormalShaper = rtnl_class_alloc();
		if (m_pNormalShaper == NULL) {
			ULOG_WARN("TC unable to allocate normal shaper\n");
			break;	
		}

		rtnl_tc_set_ifindex(TC_CAST(m_pNormalShaper), m_nIfIndex);
		rtnl_tc_set_parent(TC_CAST(m_pNormalShaper), rtnl_tc_get_handle(TC_CAST(m_pNonRealtimeShaper)));
		rtnl_tc_set_handle(TC_CAST(m_pNormalShaper), TC_HANDLE(1, NORMAL_PRIORITY_SHAPER_HANDLE));
	  	ret = rtnl_tc_set_kind(TC_CAST(m_pNormalShaper), "hfsc"); 
		if (ret != 0)
			break;

		m_bInitDone = 1;

		if(!setSpeedLimit(rateLimit))
		{
			clear();
			return -1;
		}

		ULOG_NOTE("TC init done\n");
		return 0;
	}

	clear();

	return -1;
}

bool TrafficController::setSpeedLimit(unsigned int rateLimit)
{
	if (!m_bInitDone)
	{
		return false;
	}

	unsigned int _rateLimitBytesPerSec = rateLimit / 8 * BANDWIDTH_MULTIPLIER;

	struct tc_service_curve tsc;

	tsc.m1 = 0;
	tsc.d  = 0;
	tsc.m2 = _rateLimitBytesPerSec;
	rtnl_class_hfsc_set_fsc(m_pGlobalShaper, &tsc);
	rtnl_class_hfsc_set_usc(m_pGlobalShaper, &tsc);

	tsc.m1 = 0;
	tsc.d  = 0;
	tsc.m2 = _rateLimitBytesPerSec / 2;
	rtnl_class_hfsc_set_fsc(m_pNonRealtimeShaper, &tsc);

	tsc.m1 = 0;
	tsc.d  = 0;
	tsc.m2 = (_rateLimitBytesPerSec * 9) / 10;
	rtnl_class_hfsc_set_usc(m_pNonRealtimeShaper, &tsc);

	tsc.m1 = 0;
	tsc.d  = 0;
	tsc.m2 = _rateLimitBytesPerSec * 5;
	rtnl_class_hfsc_set_fsc(m_pRealtimeShaper, &tsc);

	tsc.m1 = 0;
	tsc.d  = 0;
	tsc.m2 = (_rateLimitBytesPerSec / 50) * 49;
	rtnl_class_hfsc_set_fsc(m_pHighShaper, &tsc);

	tsc.m1 = 0;
	tsc.d  = 0;
	tsc.m2 = _rateLimitBytesPerSec / 50;
	rtnl_class_hfsc_set_fsc(m_pNormalShaper, &tsc);
	
	int ret = rtnl_qdisc_add(m_pSock, m_pRoot, NLM_F_CREATE);
	if (ret != 0) {
		ULOG_WARN("TC unable to add root\n");
		return false;
	}

	ret = rtnl_class_add(m_pSock, m_pGlobalShaper, NLM_F_CREATE);
	if (ret != 0) {
		ULOG_WARN("TC unable to add global shaper\n");
		return false;
	}

	ret = rtnl_class_add(m_pSock, m_pNonRealtimeShaper, NLM_F_CREATE);
	if (ret != 0) {
		ULOG_WARN("TC unable to add non-realtime shaper\n");
		return false;
	}

	ret = rtnl_class_add(m_pSock, m_pRealtimeShaper, NLM_F_CREATE);
	if (ret != 0) {
		ULOG_WARN("TC unable to add realtime shaper\n");
		return false; 	
	}

	ret = rtnl_class_add(m_pSock, m_pHighShaper, NLM_F_CREATE);
	if (ret != 0) {
		ULOG_WARN("TC unable to add high shaper\n");
		return false;
	}

  	ret = rtnl_class_add(m_pSock, m_pNormalShaper, NLM_F_CREATE);
	if (ret != 0) {
		ULOG_WARN("TC unable to add normal shaper\n");
		return false;
	}
	return true;
}

void TrafficController::clear()
{
	if (m_pGlobalShaper) {
		rtnl_class_delete(m_pSock, m_pGlobalShaper);
		rtnl_class_put(m_pGlobalShaper);
	}

	if (m_pRealtimeShaper) {
		rtnl_class_delete(m_pSock, m_pRealtimeShaper);
		rtnl_class_put(m_pRealtimeShaper);
	}
	if (m_pNonRealtimeShaper) {
		rtnl_class_delete(m_pSock, m_pNonRealtimeShaper);
		rtnl_class_put(m_pNonRealtimeShaper);
	}
	if (m_pNormalShaper) {
		rtnl_class_delete(m_pSock, m_pNormalShaper);
		rtnl_class_put(m_pNormalShaper);
	}
	if (m_pHighShaper) {
		rtnl_class_delete(m_pSock, m_pHighShaper);
		rtnl_class_put(m_pHighShaper);
	}

	if (m_pRoot) {
		rtnl_qdisc_delete(m_pSock, m_pRoot);
		rtnl_qdisc_put(m_pRoot);
	}

	if (m_pCache)
		nl_cache_free(m_pCache);

	if (m_pLink)
		rtnl_link_put(m_pLink);

	if (m_pSock)
		nl_socket_free(m_pSock);

	m_pGlobalShaper = NULL;
	m_pRealtimeShaper = NULL;
	m_pNonRealtimeShaper = NULL;
	m_pNormalShaper = NULL;
	m_pHighShaper = NULL;
	m_pRoot = NULL;
	m_pCache = NULL;
	m_pLink = NULL;
	m_pSock = NULL;

	m_bInitDone = 0;

	ULOG_NOTE("TC clear\r\n");
}

void TrafficController::setTrafficClass(struct rtnl_cls * filter, int trafficClass)
{
	switch (trafficClass) 
	{
		case eTrafficClassLowPriority:
			rtnl_u32_set_classid(filter, TC_HANDLE(1,NORMAL_PRIORITY_SHAPER_HANDLE));
			break;
		case eTrafficClassNormalPriority:
		case eTrafficClassHighPriority:
			rtnl_u32_set_classid(filter, TC_HANDLE(1,HIGH_PRIORITY_SHAPER_HANDLE));
			break;
		case eTrafficClassCriticalPriority:
			rtnl_u32_set_classid(filter, TC_HANDLE(1,CRITICAL_PRIORITY_SHAPER_HANDLE));
			break;
	}
}


std::shared_ptr<TrafficClassifier> TrafficController::addFlowClassifier(unsigned int src_addr, unsigned short src_port, 
			unsigned int dst_addr, unsigned short dst_port, 
			unsigned char proto, int classId)
{
	struct rtnl_cls *filter = NULL;

	for (;m_bInitDone;) {

		int ret;

		int filterId = getFreeFilterIndex();

		if(filterId < 0)
			break;

		filter = rtnl_cls_alloc();

		if (filter == NULL)
			break;

		rtnl_tc_set_link(TC_CAST(filter), m_pLink);
		rtnl_tc_set_parent(TC_CAST(filter), rtnl_tc_get_handle(TC_CAST(m_pRoot)));
		rtnl_tc_set_handle(TC_CAST(filter), filterId);

		ret = rtnl_tc_set_kind(TC_CAST(filter), "u32");
		if (ret != 0)
			break;

		rtnl_cls_set_protocol(filter, 0x0800);
		rtnl_cls_set_prio(filter, 10);

		setTrafficClass(filter,classId);

		rtnl_u32_add_key(filter, (unsigned int)proto << 8,   0x0000FF00,  8, 0);
		rtnl_u32_add_key(filter, src_addr, 0xFFFFFFFF, 12, 0);
		rtnl_u32_add_key(filter, dst_addr, 0xFFFFFFFF, 16, 0);
		rtnl_u32_add_key(filter, (htons(src_port)) | (htons(dst_port)<<16), 0xFFFFFFFF, 20, 0);
		
		rtnl_u32_set_cls_terminal(filter);
		ret = rtnl_cls_add(m_pSock, filter, NLM_F_CREATE);
		if (ret != 0) {
			ULOG_WARN("Failed to add Flow rule\n");
			break;
		}

		return std::make_shared<TrafficClassifier>(pService->m_tc, filter);
	}

	ULOG_WARN("Failed to create Flow rule\n");

	return std::make_shared<TrafficClassifier>(TrafficClassifier(pService->m_tc, NULL));
}

std::shared_ptr<TrafficClassifier> TrafficController::addGateWayClassifier(std::shared_ptr<TrafficController> tc, unsigned int selfAddress)
{
	struct rtnl_cls *filter = NULL;

	for (;m_bInitDone;) {

		int ret;

		int filterId = getFreeFilterIndex();

		if(filterId < 0)
			break;

		filter = rtnl_cls_alloc();

		if (filter == NULL)
			break;

		rtnl_tc_set_link(TC_CAST(filter), m_pLink);
		rtnl_tc_set_parent(TC_CAST(filter), rtnl_tc_get_handle(TC_CAST(m_pRoot)));
		rtnl_tc_set_handle(TC_CAST(filter), filterId);

		ret = rtnl_tc_set_kind(TC_CAST(filter), "u32");
		if (ret != 0)
			break;

		rtnl_cls_set_protocol(filter, 0x0800);
		rtnl_cls_set_prio(filter, 10);

		setTrafficClass(filter, 1);

		rtnl_u32_add_key(filter, selfAddress, 0xFFFFFFFF, 12, 0);
		
		rtnl_u32_set_cls_terminal(filter);
		ret = rtnl_cls_add(m_pSock, filter, NLM_F_CREATE);
		if (ret != 0) {
			ULOG_WARN("Failed to add Flow rule\n");
			break;
		}

		return std::make_shared<TrafficClassifier>(tc, filter);
	}

	ULOG_WARN("Failed to create Flow rule\n");

	return std::make_shared<TrafficClassifier>(TrafficClassifier(pService->m_tc, NULL));
}

std::shared_ptr<TrafficClassifier> TrafficController::addMacClassifier(unsigned char hwAddr[6], int classId)
{
	struct rtnl_cls *filter = NULL;

	for (;m_bInitDone;) {

		int ret;
	
		int filterId = getFreeFilterIndex();

		if(filterId < 0)
			break;

		filter = rtnl_cls_alloc();

		if (filter == NULL)
			break;

		rtnl_tc_set_link(TC_CAST(filter), m_pLink);
		rtnl_tc_set_parent(TC_CAST(filter), rtnl_tc_get_handle(TC_CAST(m_pRoot)));
		rtnl_tc_set_handle(TC_CAST(filter), filterId);

		ret = rtnl_tc_set_kind(TC_CAST(filter), "u32");
		if (ret != 0)
			break;

		rtnl_cls_set_protocol(filter, 0x0800);
		rtnl_cls_set_prio(filter, 10);

		setTrafficClass(filter,classId);
		
		int32_t a = *(int32_t*)hwAddr<<16;
		int32_t b = *(int32_t *)(hwAddr + 2);
		rtnl_u32_add_key(filter, a, 0xFFFF0000, -16, 0);
		rtnl_u32_add_key(filter, b, 0xFFFFFFFF, -12, 0);
		
		rtnl_u32_set_cls_terminal(filter);

		ret = rtnl_cls_add(m_pSock, filter, NLM_F_CREATE);
		if (ret != 0) {
			ULOG_WARN("Failed to add mac rule\n");
			break;
		}

		return std::make_shared<TrafficClassifier>(pService->m_tc, filter);
	}

	ULOG_WARN("Failed to create MAC rule\n");

	return std::make_shared<TrafficClassifier>(TrafficClassifier(pService->m_tc, NULL));
}

int  TrafficController::getFreeFilterIndex()
{
	int filterIndex = m_idStack.topPop();

	ULOG_NOTE("TrafficController::getFreeFilterIndex() filterIndex = %i\n", filterIndex);

	return filterIndex;
}

void TrafficController::setFreeFilterIndex(int freeFilterIndex)
{
	ULOG_NOTE("TrafficController::setFreeFilterIndex() filterIndex = %i\n", freeFilterIndex);
	m_idStack.push(freeFilterIndex);
}

bool TrafficController::modifyMacClassifier(std::shared_ptr<TrafficClassifier> classifier, int newClassId)
{
	classifier->remove();
	setTrafficClass(classifier->getFilter(),newClassId);
	int ret = rtnl_cls_add(m_pSock, classifier->getFilter(), NLM_F_CREATE);
	if (ret != 0) {
		ULOG_WARN("Failed to add Flow rule\n");
	}
	
	return !ret;
}


bool TrafficController::modifyFlowClassifier(std::shared_ptr<TrafficClassifier> classifier, int newClassId)
{
	classifier->remove();
	setTrafficClass(classifier->getFilter(),newClassId);
	int ret = rtnl_cls_add(m_pSock, classifier->getFilter(), NLM_F_CREATE);
	if (ret != 0) {
		ULOG_WARN("Failed to add Flow rule\n");
	}
	
	return !ret;
}

int TrafficController::getNIfIndex ()
{
	return m_nIfIndex;
}
