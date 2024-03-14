//
// Created by Deamon on 12/1/2022.
//

#ifndef AWEBWOWVIEWERCPP_MAPSCENERENDERBINDLESSDEFERREDVLK_H
#define AWEBWOWVIEWERCPP_MAPSCENERENDERBINDLESSDEFERREDVLK_H

#include "MapSceneRenderBindlessVLK.h"

class COpaqueMeshCollectorBindlessVLK;

class MapSceneRenderBindlessDeferredVLK : public MapSceneRenderBindlessVLK {
    friend class COpaqueMeshCollectorBindlessVLK;
public:
    explicit MapSceneRenderBindlessDeferredVLK(const HGDeviceVLK &hDevice, Config *config);
    ~MapSceneRenderBindlessDeferredVLK() override = default;

};

#endif //AWEBWOWVIEWERCPP_MAPSCENERENDERBINDLESSDEFERREDVLK_H
