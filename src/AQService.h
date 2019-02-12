/******************************************************************************
*                    Copyright 2018 aQuantia Corporation
*                    Confidential and Proprietary
******************************************************************************/

#ifndef _AQSERVICE_H

#define _AQSERVICE_H

#include <string>
#include "RestServer.h"
#include "TrafficController.h"
#include "Settings.h"

class AQService
{
public:
	explicit AQService(Settings &settings);
	~AQService();

	void run();

	RestServer m_Rest;
	std::shared_ptr<TrafficController> m_tc;
	std::shared_ptr<TrafficClassifier> m_gatewayClassifier;

	Settings &m_Settings;
	int getSpeedMbs();

private:
	static void sigUsr1Handler(int signo);
	static bool m_bUsr1Signal;

	static void Poll_static(struct uloop_timeout *timeout);
	struct uloop_timeout m_PollTimer;
	
	int getSpeedMbs(char const* adapterName);
};

extern AQService *pService;

#endif