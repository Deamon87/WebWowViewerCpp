//
// Created by Deamon on 29.09.22.
//

#ifndef AWEBWOWVIEWERCPP_MATHHELPER_CULLING_H
#define AWEBWOWVIEWERCPP_MATHHELPER_CULLING_H

#include "mathHelper.h"

template<typename T>
class ObjectCulling {
public:
    static void cull(const MathHelper::FrustumCullingData &cullingData,
                     const int start,
                     const int end,
                     const std::vector<T> &objects,
                     std::vector<uint32_t> &culling_res) {
        for (int i = start; i < end; i++) {
            culling_res[i] &= MathHelper::checkFrustum(cullingData.frustums, retrieveAABB(objects[i])) ? 0 : 0xFFFFFFFF;
        }
    }
};


#endif //AWEBWOWVIEWERCPP_MATHHELPER_CULLING_H
