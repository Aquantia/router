/******************************************************************************
*                    Copyright 2018 aQuantia Corporation
*                    Confidential and Proprietary
******************************************************************************/

#include <vector>
#include "RuleMac.h"
#include "json-c/json.h"
#include "AQService.h"
#include <arpa/inet.h>
#include "Log.h"

static json_object *serializeRule(const RULE_FLOW &r)
{
	json_object *ruleObj = json_object_new_object();

	json_object *srcAddrObj = json_object_new_string(inet_ntoa(r.key.src_addr));
	json_object_object_add(ruleObj, "src_addr", srcAddrObj);

	json_object *srcPortObj = json_object_new_int(r.key.src_port);
	json_object_object_add(ruleObj, "src_port", srcPortObj);

	json_object *dstAddrObj = json_object_new_string(inet_ntoa(r.key.dst_addr));
	json_object_object_add(ruleObj, "dst_addr", dstAddrObj);

	json_object *dstPortObj = json_object_new_int(r.key.dst_port);
	json_object_object_add(ruleObj, "dst_port", dstPortObj);

	json_object *protoObj = json_object_new_int(r.key.proto);
	json_object_object_add(ruleObj, "proto", protoObj);

	json_object *prioObj = json_object_new_int(r.priority);
	json_object_object_add(ruleObj, "priority", prioObj);

	return ruleObj;
}

std::string ruleFlowHandleRequest(const std::string& method, const std::string &request, Session &session)
{
	std::string response;
	json_object *jsonRoot = json_object_new_object();
	json_object *successObj = NULL;
	json_object *dataObj = NULL;

	successObj = json_object_new_boolean(true);
	dataObj = json_object_new_array();

	if (method.compare("GET") == 0) {

		for (auto &it: session.m_arrFlows) {
			json_object_array_add(dataObj, serializeRule(it.second));
		}	
	}

	if (method.compare("POST") == 0) {

		struct json_object* requestObj = json_tokener_parse(request.c_str());

		if (requestObj && json_object_is_type(requestObj, json_type_array)) {

			size_t requestLen = json_object_array_length(requestObj);

			for (size_t i = 0; i < requestLen; i++) {

				RULE_FLOW r;

				struct json_object *ruleObj = json_object_array_get_idx(requestObj, i);	

				if (!ruleObj || !json_object_is_type(ruleObj, json_type_object))
					continue;

				struct json_object *flowCmdObj = NULL;
				struct json_object *srcAddrObj = NULL;
				struct json_object *srcPortObj = NULL;
				struct json_object *dstAddrObj = NULL;
				struct json_object *dstPortObj = NULL;
				struct json_object *protoObj   = NULL;
				struct json_object *prioObj    = NULL;

				json_object_object_get_ex(ruleObj, "flow_cmd", &flowCmdObj);
				json_object_object_get_ex(ruleObj, "src_addr", &srcAddrObj);
				json_object_object_get_ex(ruleObj, "src_port", &srcPortObj);
				json_object_object_get_ex(ruleObj, "dst_addr", &dstAddrObj);
				json_object_object_get_ex(ruleObj, "dst_port", &dstPortObj);
				json_object_object_get_ex(ruleObj, "proto",    &protoObj);
				json_object_object_get_ex(ruleObj, "priority", &prioObj);

				if (!flowCmdObj || !json_object_is_type(flowCmdObj, json_type_string))
					continue;

				if (!srcAddrObj || !json_object_is_type(srcAddrObj, json_type_string))
					continue;

				if (!srcPortObj || !json_object_is_type(srcPortObj, json_type_int))
					continue;

				if (!dstAddrObj || !json_object_is_type(dstAddrObj, json_type_string))
					continue;

				if (!dstPortObj || !json_object_is_type(dstPortObj, json_type_int))
					continue;

				if (!protoObj || !json_object_is_type(protoObj, json_type_int))
					continue;

				if (!prioObj || !json_object_is_type(prioObj, json_type_int))
					continue;

				std::string flow_cmd = json_object_get_string(flowCmdObj);
				inet_aton(json_object_get_string(srcAddrObj), &r.key.src_addr);
				r.key.src_port = json_object_get_int(srcPortObj);
				inet_aton(json_object_get_string(dstAddrObj), &r.key.dst_addr);
				r.key.dst_port = json_object_get_int(dstPortObj);
				r.key.proto    =  json_object_get_int(protoObj);
				r.priority = json_object_get_int(prioObj);

				auto it = session.m_arrFlows.find(r.key);
				bool isFlowFound = (it != session.m_arrFlows.end());

				if (flow_cmd == "flow_add") {
					if (isFlowFound) {
						// Change flow
						if (r.priority != it->second.priority) {
							pService->m_tc->modifyFlowClassifier(it->second.classifier, r.priority);
							session.m_arrFlows[r.key].priority = r.priority;
						}
						else {
							ULOG_NOTE("Nothing changed\n");
						}
					}
					else {
						// Add flow
						r.classifier = pService->m_tc->addFlowClassifier(r.key.dst_addr.s_addr, r.key.dst_port, r.key.src_addr.s_addr, r.key.src_port, r.key.proto, r.priority);
						session.m_arrFlows[r.key] = r;
					}
				}
				else if (flow_cmd == "flow_remove") {
					if (isFlowFound)
					{
						session.m_arrFlows.erase(it);
					}
					else {
						ULOG_NOTE("Remove flow warning: flow not found\n");
					}
					continue;
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
