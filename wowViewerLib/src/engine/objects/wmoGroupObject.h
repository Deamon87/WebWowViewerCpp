//
// Created by deamon on 10.07.17.
//

#ifndef WEBWOWVIEWERCPP_WMOGROUPOBJECT_H
#define WEBWOWVIEWERCPP_WMOGROUPOBJECT_H


#include "../persistance/wmoFile.h"
#include "../wowInnerApi.h"
#include "wmoObject.h"

class WmoGroupObject {
public:
    WmoGroupObject(IWoWInnerApi *api, std::string fileName, SMOGroupInfo &groupInfo) : m_api(api), m_fileName(fileName){

    }
private:
    IWoWInnerApi *m_api;
    std::string m_fileName;

};


#endif //WEBWOWVIEWERCPP_WMOGROUPOBJECT_H
