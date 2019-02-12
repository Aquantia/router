/******************************************************************************
*                    Copyright 2018 aQuantia Corporation
*                    Confidential and Proprietary
******************************************************************************/

#include <arpa/inet.h>
#include "RestServer.h"
#include "RuleMac.h"
#include "RuleFlow.h"
#include "Neighbour.h"
#include "Client.h"
#include "Log.h"
#include "AQService.h"

static RestServer *server;

RestServer::~RestServer()
{
	clear();
}

int RestServer::init(const std::string &addr, unsigned short port, bool isSSL)
{
	m_pSrv = uh_server_new(addr.c_str(), port);
	if (isSSL)
	{
		m_pSrv->ssl_init(m_pSrv, "/etc/ssl/aqservice-key.pem", "/etc/ssl/aqservice-cert.pem");
	}
	m_pSrv->on_accept = onAccept_static;
	m_pSrv->on_request = onRequest_static;
	m_pSrv->on_client_free = onClientFree_static;

	memset(&m_PollTimer, 0, sizeof(m_PollTimer));
	m_PollTimer.cb = Poll_static;
	uloop_timeout_set(&m_PollTimer, 1000);
	
	server = this;

	return 0;
}

int RestServer::run(void)
{
	return 0;
}

void RestServer::clear(void)
{
	if (m_pSrv != NULL) {
		m_pSrv->free(m_pSrv);
		m_pSrv = NULL;
	}

	TrafficClassifier::fastRemoveFlag = true;
	ruleMacClear();
	m_arrSessions.clear();
}

void RestServer::onAccept_static(struct uh_client *cl)
{
	server->onAccept(cl);
}

int RestServer::onRequest_static(struct uh_client *cl)
{
	return server->onRequest(cl);
}

void RestServer::onClientFree_static(struct uh_client *cl)
{
	server->onClientFree(cl);
}

void RestServer::onAccept(struct uh_client *cl)
{
	ULOG_NOTE("New connection from: %s:%d\n", cl->get_peer_addr(cl), cl->get_peer_port(cl));
}

int  RestServer::onRequest(struct uh_client *cl)
{
	ULOG_NOTE("New request from: %s:%d\n", cl->get_peer_addr(cl), cl->get_peer_port(cl));
	
	const std::string  method = cl->get_method(cl);
	
	const std::string  path  = cl->get_path(cl);

	int inSz;
	const char * in = cl->get_body(cl, &inSz);
	std::string request(in, inSz);

	//ULOG_NOTE("New request from: %s:%d method %s path %s request %s \n", cl->get_peer_addr(cl), cl->get_peer_port(cl), method.c_str(), path.c_str(), request.c_str());

	const in_addr_t clientAddr = inet_addr(cl->get_peer_addr(cl));

	// Create or find a session for this ip address
	Session & session = m_arrSessions.count(clientAddr) ?  m_arrSessions.at(clientAddr) : m_arrSessions.emplace(clientAddr,clientAddr).first->second;
 

	if (path.compare("/client/auth") == 0)
	{
		std::string response = clientHandleAuthRequest(method, request, session);

		cl->send_header(cl, 200, "OK", -1);
		cl->append_header(cl, "AQGaming", "Router Service");
		cl->header_end(cl);

		cl->chunk_send(cl, response.c_str(), response.length());
	}
	else {
	
		// Starting from this point, only authorized clients can proceed
		const char *token = cl->get_header(cl, "token");
		if ((!token) || (!session.validateToken(token))) {
			cl->send_header(cl, 401, "Unauthorized", -1);
			cl->append_header(cl, "AQGaming", "Router Service");
			cl->header_end(cl);
		}
		else {

			session.poll();

			if (path.compare("/info") == 0)
			{
				std::string response = m_infoStorage.handleRequest(method, request);

				cl->send_header(cl, 200, "OK", -1);
				cl->append_header(cl, "AQGaming", "Router Service");
				cl->header_end(cl);
				
				cl->chunk_send(cl, response.c_str(), response.length());
			}
			else
			if (path.compare("/rules/mac") == 0)
			{
				std::string response = ruleMacHandleRequest(method, request);

				if(method.compare("POST") == 0)
				{
					m_infoStorage.setSynced(true);
				}

				cl->send_header(cl, 200, "OK", -1);
				cl->append_header(cl, "AQGaming", "Router Service");
				cl->header_end(cl);
				
				cl->chunk_send(cl, response.c_str(), response.length());
			}
			else
			if (path.compare("/rules/flow") == 0)
			{
				std::string response = ruleFlowHandleRequest(method, request, session);

				cl->send_header(cl, 200, "OK", -1);
				cl->append_header(cl, "AQGaming", "Router Service");
				cl->header_end(cl);
				
				cl->chunk_send(cl, response.c_str(), response.length());
			}
			else
			if (path.compare("/neighbours") == 0)
			{
				std::string response = neigHandleRequest(method, request);
				
				cl->send_header(cl, 200, "OK", -1);
				cl->append_header(cl, "AQGaming", "Router Service");
				cl->header_end(cl);

				cl->chunk_send(cl, response.c_str(), response.length());
			}
			else
			if (path.compare("/options") == 0)
			{
				handleOptionRequest(method, request);
				
				cl->send_header(cl, 200, "OK", -1);
				cl->append_header(cl, "AQGaming", "Router Service");
				cl->header_end(cl);
			}
			else
			if (path.compare("/close") == 0)
			{

				cl->send_header(cl, 200, "OK", -1);
				cl->append_header(cl, "AQGaming", "Router Service");
				cl->header_end(cl);
				
				m_arrSessions.erase(clientAddr);

				if (m_arrSessions.empty())
				{
					int linkSpeedMbps = pService->getSpeedMbs();
					
					if (linkSpeedMbps > 0)
					{
						pService->m_tc->setSpeedLimit(linkSpeedMbps * 1000 * 1000);
					}
				}
			}
			else {
				cl->send_header(cl, 404, "Not found", -1);
				cl->append_header(cl, "AQGaming", "Router Service");
				cl->header_end(cl);
			}
		}
	}

	cl->request_done(cl);

	return UH_REQUEST_DONE;
}

void RestServer::onClientFree(struct uh_client *cl)
{
	ULOG_NOTE("Connection closed: %s:%d\n", cl->get_peer_addr(cl), cl->get_peer_port(cl));
}


void RestServer::Poll_static(struct uloop_timeout *timeout)
{
	server->Poll();
	uloop_timeout_set(timeout, 1000);
}

void RestServer::Poll()
{
	for (auto it = m_arrSessions.begin(), ite = m_arrSessions.end(); it != ite;) {
		if (it->second.isExpired()) {
				it = m_arrSessions.erase(it);
		}
		else {
			++it;
		}
	}
}
