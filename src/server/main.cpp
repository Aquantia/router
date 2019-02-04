/******************************************************************************
*                    Copyright 2018 aQuantia Corporation
*                    Confidential and Proprietary
******************************************************************************/

#include <stdio.h>
#include "AQService.h"
#include "Settings.h"
#include "Log.h"

static const char* serverVersion = "1.0.10";

int main(int argc, char* argv[])
{
        Settings settings;

	settings.init(argc, argv);

	ulog_open(settings.syslog ? ULOG_SYSLOG : ULOG_STDIO, LOG_DAEMON, "AQService");

	ulog_threshold(LOG_DEBUG);

	ULOG_INFO("Start. Version: %s\n", serverVersion);
   
	settings.parseConfig();

	pService = new AQService(settings);

	pService->run();

	delete pService;

	ULOG_INFO("Stopped");

	ulog_close();

	return 0;
}

