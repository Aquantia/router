/******************************************************************************
*                    Copyright 2018 aQuantia Corporation
*                    Confidential and Proprietary
******************************************************************************/

#ifndef _RULE_MAC

#define _RULE_MAC

#include <string>
#include <memory>
#include "TrafficClassifier.h"

typedef struct {
	char hwAddr[6];
	int priority;	
	std::shared_ptr<TrafficClassifier> classifier;
} RULE_MAC;

std::string ruleMacHandleRequest(const std::string method, const std::string &request);

void ruleMacDisableNeighbour(unsigned long long key);

void ruleMacEnableNeighbour(unsigned long long key);

void ruleMacClear();
#endif
