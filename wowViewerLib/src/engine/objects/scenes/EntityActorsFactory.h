//
// Created by Deamon on 12/6/2023.
//

#ifndef AWEBWOWVIEWERCPP_ENTITYACTORSFACTORY_H
#define AWEBWOWVIEWERCPP_ENTITYACTORSFACTORY_H

#include <memory>
#include "../m2/m2Object.h"
#include "../wmo/wmoObject.h"

class EntityActorsFactory {
public:
    EntityActorsFactory(const HApiContainer &api) : m_api(api){};

    std::shared_ptr<M2Object> createM2Object() {
        return std::make_shared<M2Object>(m_api);
    }
    M2Object * getM2ObjectById(int id) {
        return m2ObjectCache[id];
    }

private:
    HApiContainer m_api;

    std::vector<M2Object *> m2ObjectCache;
    std::vector<WmoObject *> wmoObjectCache;
    std::vector<WmoGroupObject *> wmoGroupObjectCache;
};


#endif //AWEBWOWVIEWERCPP_ENTITYACTORSFACTORY_H
