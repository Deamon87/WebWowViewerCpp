//
// Created by deamon on 10.07.17.
//

#include "wmoGroupObject.h"
void WmoGroupObject::draw(SMOMaterial *materials, std::function <BlpTexture&(int materialId)> getTextureFunc) {
    if (!this->m_loaded) {
        if (m_geom != nullptr && m_geom->isLoaded()){
            this->m_loaded = true;
            return;
        }

        this->startLoading();
        return;
    }

    if (m_geom->batchesLen <= 0) return;

    m_geom->draw(m_api, materials, getTextureFunc);

}

void WmoGroupObject::startLoading() {
    if (!this->m_loading) {
        this->m_loading = true;

        m_geom = m_api->getWmoGroupGeomCache()->get(m_fileName);
    }
}
