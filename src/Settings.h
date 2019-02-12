/******************************************************************************
*                    Copyright 2018 aQuantia Corporation
*                    Confidential and Proprietary
******************************************************************************/

#ifndef _SETTINGS_H

#define _SETTINGS_H

#include <string>

class Settings
{
public:
	Settings();
	void init(int argc, char *argv[]);
	void parseConfig();

	bool helpFlag;
	bool debugFlag;

	bool enabled;
	bool syslog;

	std::string addr;
	unsigned short port;

	std::string ethNameFilter;
	
	unsigned int rateLimit;
	bool isRateLimitSet;

	std::string ethNameOut;

	std::string password;
	bool secureConnection;

	bool isSSL;

	std::string configFile;
};

#endif
