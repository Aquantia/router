/******************************************************************************
*                    Copyright 2018 aQuantia Corporation
*                    Confidential and Proprietary
******************************************************************************/

#ifndef _INFO_STORAGE_H
#define _INFO_STORAGE_H
#include <string>

class InfoStorage
{
public:
    InfoStorage();
    ~InfoStorage();

    bool getSynced() const;
    void setSynced(bool isSynced);

    std::string handleRequest(const std::string& method, const std::string& request);

private:
    bool m_isSynced;
};

#endif
