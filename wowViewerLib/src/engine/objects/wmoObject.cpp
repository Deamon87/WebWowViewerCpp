//
// Created by deamon on 03.07.17.
//

#include "wmoObject.h"

std::string WmoObject::getTextureName(int index) {
    return std::__cxx11::string();
}

M2WmoObject &WmoObject::getDoodad(int index) {
    return <#initializer#>;
}

void WmoObject::startLoading() {
    if (!m_loading) {
        m_loading = true;

        Cache<WmoMainGeom> *wmoGeomCache =  m_api->getWmoMainCache();

        mainGeom = wmoGeomCache->get(m_modelName);
    }
}
