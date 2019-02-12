/******************************************************************************
*                    Copyright 2018 aQuantia Corporation
*                    Confidential and Proprietary
******************************************************************************/
#include <arpa/inet.h>
#include "AQService.h"
#include "Log.h"
#include "cstring"

AQService *pService = NULL;

bool AQService::m_bUsr1Signal = false;

AQService::AQService(Settings &settings) : m_Settings(settings), m_tc(std::make_shared<TrafficController>())
{
	ULOG_NOTE("Create\n");

	struct sigaction s;

	s.sa_handler = sigUsr1Handler;
	s.sa_flags = 0;
	sigaction(SIGUSR1, &s, NULL);

	uloop_init();

	memset(&m_PollTimer, 0, sizeof(m_PollTimer));
	m_PollTimer.cb = Poll_static;
	uloop_timeout_set(&m_PollTimer, 1000);

	int speed = getSpeedMbs();

	m_tc->init(settings.ethNameFilter, speed*1000*1000 );

	struct in_addr selfAddress;
	inet_aton(m_Settings.addr.c_str(), &selfAddress);
	m_gatewayClassifier = m_tc->addGateWayClassifier(m_tc, selfAddress.s_addr);

	if (settings.enabled)
		m_Rest.init(settings.addr, settings.port, settings.isSSL);
}

AQService::~AQService()
{
	uloop_done();	
	ULOG_NOTE("Destroy\n");
}

void AQService::run()
{
	ULOG_NOTE("Run\n");

	m_Rest.run();

	uloop_run();

	ULOG_NOTE("Stop\n");
}

void AQService::sigUsr1Handler(int signo)
{
	m_bUsr1Signal = true;
}

void AQService::Poll_static(struct uloop_timeout *timeout)
{
	uloop_timeout_set(timeout, 1000);

	if (m_bUsr1Signal) {

		uloop_end();

		m_bUsr1Signal = false;
	}
}


int AQService::getSpeedMbs(char const * adapterName)
{
	char buff[256];
	int result = -1;
	snprintf(buff,
		sizeof(buff),
		"/sys/class/net/%s/speed",
		adapterName);
	FILE * fd = fopen(buff, "r");
	
	if (fd)
	{
		fread(buff, 1, sizeof(buff), fd);
		result = atoi(buff);
	}
	else
	{
		ULOG_ERR("failed to open %s with %d", buff, errno);
	}
	return result;	
}

int AQService::getSpeedMbs()
{
	return getSpeedMbs(m_Settings.ethNameOut.c_str());
}