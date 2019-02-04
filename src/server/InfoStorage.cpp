/******************************************************************************
*                    Copyright 2018 aQuantia Corporation
*                    Confidential and Proprietary
******************************************************************************/

#include "InfoStorage.h"
#include "Log.h"
#include "json-c/json.h"


InfoStorage::InfoStorage() : m_isSynced(false)
{

}

InfoStorage::~InfoStorage()
{

}

bool InfoStorage::getSynced() const
{
    return m_isSynced;
}

void InfoStorage::setSynced(bool isSynced)
{
    m_isSynced = isSynced;
}

std::string InfoStorage::handleRequest(const std::string& method, const std::string& request)
{
    std::string response;
    json_object *jsonRoot = json_object_new_object();
    json_object *successObj = NULL;
    json_object *dataObj = NULL;

    successObj = json_object_new_boolean(true);
    dataObj = json_object_new_array();

    if (method.compare("GET") == 0) {

        json_object *info = json_object_new_object();
        json_object *synced = json_object_new_boolean(m_isSynced);
        json_object_object_add(info, "synced", synced);
        json_object_array_add(dataObj, info);
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