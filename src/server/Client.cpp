/******************************************************************************
*                    Copyright 2018 aQuantia Corporation
*                    Confidential and Proprietary
******************************************************************************/

#include <uuid/uuid.h>
#include "json-c/json.h"
#include "Client.h"
#include "AQService.h"

std::string clientHandleAuthRequest(const std::string &method, const std::string &request, Session &session)
{
	std::string response;
	json_object *jsonRoot = json_object_new_object();
	json_object *successObj = NULL;
	json_object *dataObj = NULL;

	if (method.compare("POST") == 0) {

		char token[37];

		bool passwordMatch;

		if (pService->m_Settings.secureConnection) {

			struct json_object* requestObj = json_tokener_parse(request.c_str());

			passwordMatch = false;

			if (requestObj && json_object_is_type(requestObj, json_type_object)) {
			
				struct json_object *passObj = json_object_object_get(requestObj, "password");

				if (passObj && json_object_is_type(passObj, json_type_string)) {

					const char *password = json_object_get_string(passObj);

					passwordMatch = pService->m_Settings.password.compare(password) == 0;
				}
			}

			if (requestObj) {
				json_object_put(requestObj);
			}
		}
		else {
			passwordMatch = true;
		}
		
		if (passwordMatch) {

			successObj = json_object_new_boolean(true);
			dataObj = json_object_new_object();

			uuid_t uuid;
			uuid_generate(uuid);

			uuid_unparse(uuid, token);

			json_object *tokenObj = json_object_new_string(token);
			json_object_object_add(dataObj, "token", tokenObj);

			session.setToken(token);
		}
		else {
			successObj = json_object_new_boolean(false);
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
