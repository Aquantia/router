/******************************************************************************
*                    Copyright 2018 aQuantia Corporation
*                    Confidential and Proprietary
******************************************************************************/

#include <map>
#include <set>
#include "RuleMac.h"
#include "json-c/json.h"
#include "AQService.h"

static std::map<unsigned long long, RULE_MAC> rules;

static std::map<unsigned long long,int> disabledMacs;


static json_object *serializeRule(const RULE_MAC &r)
{
	json_object *ruleObj = json_object_new_object();
	char hwAddr[18];
	snprintf(hwAddr, sizeof(hwAddr), "%02hhX-%02hhX-%02hhX-%02hhX-%02hhX-%02hhX", r.hwAddr[0], r.hwAddr[1], r.hwAddr[2], r.hwAddr[3], r.hwAddr[4], r.hwAddr[5]);

	json_object *addr = json_object_new_string(hwAddr);
	json_object_object_add(ruleObj, "hwAddr", addr);
	json_object *prio = json_object_new_int(r.priority);
	json_object_object_add(ruleObj, "priority", prio);

	return ruleObj;
}

std::string ruleMacHandleRequest(const std::string method, const std::string &request)
{
	std::string response;
	json_object *jsonRoot = json_object_new_object();
	json_object *successObj = NULL;
	json_object *dataObj = NULL;

	successObj = json_object_new_boolean(true);
	dataObj = json_object_new_array();

	if (method.compare("GET") == 0) {

		for (auto &it: rules) {
			json_object_array_add(dataObj, serializeRule(it.second));
		}	
	}

	if (method.compare("POST") == 0) {

		struct json_object* requestObj = json_tokener_parse(request.c_str());

		if (requestObj && json_object_is_type(requestObj, json_type_array)) {

			size_t requestLen = json_object_array_length(requestObj);

			for (size_t i = 0; i < requestLen; i++) {

				RULE_MAC r;
				int hwAddr[6];
				unsigned long long key = 0;

				struct json_object *ruleObj = json_object_array_get_idx(requestObj, i);	
		
				if (!ruleObj || !json_object_is_type(ruleObj, json_type_object))
					continue;

				struct json_object *addrObj = json_object_object_get(ruleObj, "hwAddr");
				struct json_object *prioObj = json_object_object_get(ruleObj, "priority");

				if (!addrObj || !json_object_is_type(addrObj, json_type_string))
					continue;

				if (!prioObj || !json_object_is_type(prioObj, json_type_int))
					continue;

				const char *addrStr = json_object_get_string(addrObj);
				sscanf(addrStr, "%02X-%02X-%02X-%02X-%02X-%02X", &hwAddr[0], &hwAddr[1], &hwAddr[2], &hwAddr[3], &hwAddr[4], &hwAddr[5]);
				for (int k = 0; k < 6; k++)
					r.hwAddr[k] = hwAddr[k];

				memcpy(&key, r.hwAddr, 6);

				r.priority = json_object_get_int(prioObj);
				
				auto it = rules.find(key);

				if (r.priority == TrafficController::eTrafficClassLowPriority) {
					if (it != rules.end())
					{
						rules.erase(it);
					}
					continue;
				}

				if (it == rules.end()) {
    				// Rule doesn't exist	
    				if(!disabledMacs.count(key))
    				{
    				
        				r.classifier = pService->m_tc->addMacClassifier(r.hwAddr, r.priority);
        				rules[key] = r;
    				}
				}
				else
				if (r.priority == TrafficController::eTrafficClassLowPriority) {
					// Normal priority means we should remove the rule
					rules.erase(it);
					continue;
				}
				else
				if (r.priority != it->second.priority) {
					// Priority has been changed, update rule
					pService->m_tc->modifyMacClassifier(it->second.classifier, r.priority);
					rules[key].priority = r.priority;
				}
				else {
					// Nothing changed
				}
				
				json_object_array_add(dataObj, serializeRule(r));
			}
		}

		if (requestObj) {
			json_object_put(requestObj);
		}
	}

	if (successObj)
		json_object_object_add(jsonRoot, "success", successObj);

	if (dataObj) 
		json_object_object_add(jsonRoot, "data", dataObj);	

	const char *jsonStr = json_object_to_json_string(jsonRoot);

	response = std::string(jsonStr);

	json_object_put(jsonRoot);

	return response;
}

void ruleMacClear()
{
	rules.clear();
	disabledMacs.clear();
}

void ruleMacDisableNeighbour(unsigned long long key)
{
	auto it = rules.find(key);
    int priority = TrafficController::eTrafficClassLowPriority;
	if (it != rules.end())
	{
    	priority = it->second.priority;
		rules.erase(it);
	}
    disabledMacs.insert(std::pair <unsigned long long,int>(key, priority));
}

void ruleMacEnableNeighbour(unsigned long long key)
{
    auto it = disabledMacs.find(key);
    if (it != disabledMacs.end())
	{
    	if (it->second != TrafficController::eTrafficClassLowPriority)
    	{
        	RULE_MAC r;
        	memcpy(r.hwAddr, &key, 6);
        	r.priority = it->second;
        	r.classifier = pService->m_tc->addMacClassifier(r.hwAddr, r.priority);
        	rules[it->first] = r;
    	}
    	
		disabledMacs.erase(key);
	}
}
