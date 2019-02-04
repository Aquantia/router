/******************************************************************************
*                    Copyright 2018 aQuantia Corporation
*                    Confidential and Proprietary
******************************************************************************/

#ifndef _REST_SERVER_H

#define _REST_SERVER_H

extern "C" {
	#include <uhttpd.h>
}

#include <string>
#include <map>
#include <memory>
#include "Session.h"
#include "InfoStorage.h"
#include "TrafficController.h"
#include "Option.h"

class RestServer
{
public:
	~RestServer();
	int init(const std::string &addr, unsigned short port, bool isSSL = true);
	void clear();
	int run();

	std::map<in_addr_t, Session> m_arrSessions;

private:
	static void onAccept_static(struct uh_client *cl);
	static int onRequest_static(struct uh_client *cl);
	static void onClientFree_static(struct uh_client *cl);
	

	void onAccept(struct uh_client *cl);
	int  onRequest(struct uh_client *cl);
	void onClientFree(struct uh_client *cl);	

	static void Poll_static(struct uloop_timeout *timeout);
	void Poll();

	struct uh_server *m_pSrv = NULL;

	struct uloop_timeout m_PollTimer;

	InfoStorage m_infoStorage;
};

#endif
