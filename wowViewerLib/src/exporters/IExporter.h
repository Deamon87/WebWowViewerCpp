//
// Created by Deamon on 12/22/2020.
//

#ifndef AWEBWOWVIEWERCPP_IEXPORTER_H
#define AWEBWOWVIEWERCPP_IEXPORTER_H

class M2Object;

#include <memory>
#include "../engine/objects/m2/m2Object.h"

class IExporter {
public:
    virtual ~IExporter() = default;

public:
    virtual void addM2Object(std::shared_ptr<M2Object> &m2Object) = 0;
    virtual void saveToFile(std::string filePath) = 0;
protected:
    static auto getM2Geom(std::shared_ptr<M2Object> &m2Object) {
        return m2Object->m_m2Geom;
    }
    static auto getM2BlpTextureData(std::shared_ptr<M2Object> &m2Object, int index) {
        return m2Object->getBlpTextureData(index);
    }
    static auto getBoneMasterData(std::shared_ptr<M2Object> &m2Object) {
        return m2Object->m_boneMasterData;
    }
    static auto getSkinGeom(std::shared_ptr<M2Object> &m2Object) {
        return m2Object->m_skinGeom;
    }
};


#endif //AWEBWOWVIEWERCPP_IEXPORTER_H
