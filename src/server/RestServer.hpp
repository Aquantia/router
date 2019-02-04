#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include "abstract_server.h"
	
struct AbstructServer * create_RestServer(struct AbstructServer *);
void desory_RestServer(struct AbstructServer *);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
class RestServerImpl;

class RestServer
{
public:
	RestServer();
	~RestServer();
	
	int serverInit();
	int serverPoll();
	void serverUninit();
private:
	RestServerImpl* _impl;
};
#endif
