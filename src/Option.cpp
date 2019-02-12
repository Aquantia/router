/******************************************************************************
*                    Copyright 2018 aQuantia Corporation
*                    Confidential and Proprietary
******************************************************************************/

#include "Option.h"
#include "Log.h"
#include "json-c/json.h"
#include "AQService.h"

void handleOptionRequest(const std::string &method, const std::string &request)
{
    if (method.compare("POST") == 0) {

        struct json_object* requestObj = json_tokener_parse(request.c_str());

        if (requestObj && json_object_is_type(requestObj, json_type_array)) {

            size_t requestLen = json_object_array_length(requestObj);

            for (size_t i = 0; i < requestLen; i++) {


                struct json_object *optionObj = json_object_array_get_idx(requestObj, i); 

                if (!optionObj || !json_object_is_type(optionObj, json_type_object))
                    continue;

                struct json_object *downloadLimitObj = NULL;
                struct json_object *uploadLimitObj = NULL;

                json_object_object_get_ex(optionObj, "download_limit", &downloadLimitObj);
                json_object_object_get_ex(optionObj, "upload_limit", &uploadLimitObj);

                if (!downloadLimitObj || !json_object_is_type(downloadLimitObj, json_type_int))
                    continue;

                if (!uploadLimitObj || !json_object_is_type(uploadLimitObj, json_type_int))
                    continue;

                int dlKbps = json_object_get_int(downloadLimitObj);

                // TODO: add upload speed limit handling.
                int linkSpeedMbps = pService->getSpeedMbs();
                unsigned int downloadLimit = 0;
                
                if (linkSpeedMbps > 0)
                {
                    unsigned int linkSpeed = linkSpeedMbps * 1000 * 1000;
                    downloadLimit = dlKbps * 1000 > linkSpeed ? linkSpeed : dlKbps * 1000;
                }
                else
                {
                    downloadLimit = dlKbps * 1000;
                }

                pService->m_tc->setSpeedLimit(downloadLimit);
            }
        }
    }
    
}