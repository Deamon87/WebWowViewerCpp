//
// Created by deamon on 03.07.17.
//

#include "wmoObject.h"

std::string WmoObject::getTextureName(int index) {
    return std::__cxx11::string();
}

//M2Object &WmoObject::getDoodad(int index) {
//    return M2Object;
//}

void WmoObject::startLoading() {
    if (!m_loading) {
        m_loading = true;

        Cache<WmoMainGeom> *wmoGeomCache = m_api->getWmoMainCache();

        mainGeom = wmoGeomCache->get(m_modelName);

    }
}

void WmoObject::createGroupObjects(){
    groupObjects = std::vector<WmoGroupObject*>(mainGeom->groupsLen, nullptr);

    std::string nameTemplate = m_modelName.substr(0, m_modelName.find_last_of("."));
    for(int i = 0; i < mainGeom->groupsLen; i++) {
        std::string numStr = std::to_string(i);
        for (int j = numStr.size(); j < 3; j++) numStr = '0'+numStr;

        std::string groupFilename = nameTemplate + "_" + numStr + ".wmo";


        groupObjects[i] = new WmoGroupObject(*this, m_api, groupFilename, mainGeom->groups[i]);
    }
}

void WmoObject::draw(){
    if (!m_loaded) {
        if (mainGeom != nullptr && mainGeom->getIsLoaded()){
            m_loaded = true;
            m_loading = false;

            this->createGroupObjects();
        } else {
            this->startLoading();
        }

        return;
    }
    auto wmoShader = m_api->getWmoShader();

    glUniformMatrix4fv(wmoShader->getUnf("uPlacementMat"), 1, GL_FALSE, &this->m_placementMatrix[0]);

    for (int i= 0; i < groupObjects.size(); i++) {
        if(groupObjects[i] != nullptr) {
            groupObjects[i]->draw(mainGeom->materials, m_getTextureFunc);
        }
    }
}

void WmoObject::setLoadingParam(std::string modelName, SMMapObjDef &mapObjDef) {
    m_modelName = modelName;

    this->m_placementMatrix = mathfu::mat4::Identity();
//    createPlacementMatrix(mapObjDef);
}

BlpTexture &WmoObject::getTexture(int textureId) {
    std::string materialTexture((char *)&mainGeom->textureNamesField[textureId]);

    //TODO: cache Textures used in WMO
    return *m_api->getTextureCache()->get(materialTexture);
}
