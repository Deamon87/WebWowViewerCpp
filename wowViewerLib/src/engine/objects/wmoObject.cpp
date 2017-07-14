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

        Cache<WmoMainGeom> *wmoGeomCache = m_api->getWmoMainCache();

        mainGeom = wmoGeomCache->get(m_modelName);
    }
}

void WmoObject::createGroupObjects(){
    groupObjects = std::vector<WmoGroupObject>(mainGeom->groupsLen);

    std::string nameTemplate = m_modelName.substr(0, m_modelName.find_last_of("."));
    for(int i = 0; i < mainGeom->groupsLen; i++) {
        std::string numStr = std::to_string(i);
        for (int j = numStr.size(); j < 3; j++) numStr = '0'+numStr;

        std::string groupFilename = nameTemplate + "_" + numStr + ".wmo";


        groupObjects[i] = WmoGroupObject(*this, m_api, groupFilename, mainGeom->groups[i]);
    }
}

void WmoObject::draw(){
    if (m_loaded) {
        if (mainGeom != nullptr && mainGeom->getIsLoaded()){
            m_loaded = true;
            m_loading = false;

            this->createGroupObjects();
        } else {
            this->startLoading();
        }

        return;
    }
}

void WmoObject::setLoadingParam(std::string modelName, SMMapObjDef &mapObjDef) {
    m_modelName = modelName;

    createPlacementMatrix(mapObjDef);
}

BlpTexture &WmoObject::getTexture(int textureId) {
    std::string materialTexture(&mainGeom->textureNamesField[textureId]);

    //TODO: cache Textures used in WMO
    return *m_api->getTextureCache()->get(materialTexture);
}
