//
// Created by deamon on 10.07.17.
//

#ifndef WEBWOWVIEWERCPP_WMOGROUPOBJECT_H
#define WEBWOWVIEWERCPP_WMOGROUPOBJECT_H

class WmoObject;
class WmoGroupObject;

#include "../wowInnerApi.h"
#include "../persistance/wmoFile.h"
#include "wmoObject.h"

class WmoGroupObject {
public:
    WmoGroupObject(WmoObject &wmoObject, IWoWInnerApi *api, std::string fileName, SMOGroupInfo &groupInfo) : m_wmoObject(wmoObject), m_fileName(fileName){

    }

    void draw(SMOMaterial *materials, std::function <BlpTexture&(int materialId)> m_getTextureFunc);
private:
    IWoWInnerApi *m_api;
    WmoObject &m_wmoObject;
    WmoGroupGeom *m_geom;
    std::string m_fileName;


    bool m_loading;
    bool m_loaded;

    void startLoading();

};


#endif //WEBWOWVIEWERCPP_WMOGROUPOBJECT_H
