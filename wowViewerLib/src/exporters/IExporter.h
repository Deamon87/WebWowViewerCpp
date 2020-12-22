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
    virtual void addM2Object(std::shared_ptr<M2Object> &m2Object) = 0;

protected:
    auto getM2Geom(std::shared_ptr<M2Object> &m2Object) {
        return m2Object->m_m2Geom;
    }
    auto getBoneMasterData(std::shared_ptr<M2Object> &m2Object) {
        return m2Object->m_boneMasterData;
    }
    auto getSkinGeom(std::shared_ptr<M2Object> &m2Object) {
        return m2Object->m_skinGeom;
    }
};


#endif //AWEBWOWVIEWERCPP_IEXPORTER_H
