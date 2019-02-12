/******************************************************************************
*                    Copyright 2018 aQuantia Corporation
*                    Confidential and Proprietary
******************************************************************************/

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "Settings.h"
#include "INIReader.h"
#include "Log.h"

Settings::Settings() : 	enabled(true), addr("0.0.0.0"), port(8080), ethNameFilter("br-lan"), rateLimit(1000000000), 
			password(""), secureConnection(false), ethNameOut("eth0.2"),
			helpFlag(false), debugFlag(false), syslog(false), isRateLimitSet(false), isSSL(true)
{
}

void Settings::init(int argc, char *argv[])
{
	const char* aviableOpts = "p:d:a:r:o:hvsltc:";
	int opt;

	while((opt = getopt(argc, argv, aviableOpts)) != -1) {
		switch(opt) {
		case 'p': port = atoi(optarg); break;
		case 'd': ethNameFilter = optarg; break;
		case 'a': addr = optarg; break;
		case 'c': configFile = optarg; break;
		case 'o': ethNameOut = optarg; break;
		case 'r': rateLimit = atoi(optarg); isRateLimitSet = true; break;
		case 'h': helpFlag = true; break;
		case 'v': debugFlag = true; break;
		case 's': secureConnection = true; break;
		case 'l': syslog = true; break;
		case 't': isSSL = false; break;
		default: printf("Unknown key provided\n"); break;
		}
	}

	if (helpFlag) {
		printf("Aviable keys are "
			"-a -p -d -r -h -s\n"
			"-a <addr> bind address\n"
			"-p <num> bind port\n"
			"-d <eth> network interface\n"
			"-o <eth> external network interface\n"
			"-r <num> Rate limit\n"
			"-s Use password protection\n"
			"-v debug flag\n"
			"-c <path> Full path to config file\n"
			"-t disable SSL\n"
			"-h this help.\n");
	}

}

void Settings::parseConfig()
{
	if (!configFile.empty()) {

		INIReader reader(configFile.c_str());

		if (reader.ParseError() >= 0) {

			ULOG_INFO("Using config file: %s\n", configFile.c_str());

			enabled = reader.GetInteger("general", "enable", enabled);
			addr = reader.Get("general", "addr", addr.c_str());
			port = reader.GetInteger("general", "port", port);
			password = reader.Get("general", "password", password.c_str());
			ethNameFilter = reader.Get("general", "ifname", ethNameFilter.c_str());
			ethNameOut = reader.Get("general", "ifnameOut", ethNameOut.c_str());
		}
		else {
			ULOG_WARN("Error pasring config file\n");
		}
	}

	ULOG_INFO("Options, Enbled: %d\n", enabled);
	ULOG_INFO("Options, Listen address: %s\n", addr.c_str());
	ULOG_INFO("Options, Listen port: %d\n", port);
	ULOG_INFO("Options, Secure connection: %d\n", secureConnection);
	ULOG_INFO("Options, Device name: %s\n", ethNameFilter.c_str());
}
