//
// Created by deamon on 03.07.17.
//

#ifndef WEBWOWVIEWERCPP_WMOOBJECT_H
#define WEBWOWVIEWERCPP_WMOOBJECT_H

#include <string>
#include "m2WmoObject.h"

class WmoObject {
    WmoObject(IWoWInnerApi *api) : m_api(api) {

    }

    IWoWInnerApi *m_api;

    WmoMainGeom *mainGeom = nullptr;
    bool m_loading = false;
    bool m_loaded = false;

public:
    std::string getTextureName(int index);

    M2WmoObject& getDoodad(int index);
    void setLoadingParam();
    void startLoading();

};


#endif //WEBWOWVIEWERCPP_WMOOBJECT_H
