//
// Created by Deamon on 12/1/2022.
//

#ifndef AWEBWOWVIEWERCPP_MAPSCENERENDERERFACTORY_H
#define AWEBWOWVIEWERCPP_MAPSCENERENDERERFACTORY_H


#include <memory>
#include "MapSceneRenderer.h"

class MapSceneRendererFactory {
public:
    static std::shared_ptr<MapSceneRenderer> createForwardRenderer(const HGDevice &device, Config * config);
};


#endif //AWEBWOWVIEWERCPP_MAPSCENERENDERERFACTORY_H
