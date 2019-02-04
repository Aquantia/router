/******************************************************************************
*                    Copyright 2018 aQuantia Corporation
*                    Confidential and Proprietary
******************************************************************************/

#ifndef _CLIENT_H

#define _CLIENT_H

#include <string>
#include "Session.h"

std::string clientHandleAuthRequest(const std::string &method, const std::string &request, Session &session);

#endif