//
// Created by Deamon on 11/26/2022.
//

#ifndef AWEBWOWVIEWERCPP_MAPSCENERENDERER_H
#define AWEBWOWVIEWERCPP_MAPSCENERENDERER_H


#include "../engine/objects/scenes/map.h"

class MapSceneRenderer {
public:
    MapSceneRenderer() = default;
    virtual ~MapSceneRenderer() = 0;

    virtual HFrameBuffer putIntoQueue(HCullStage &cullStage) = 0;

};


#endif //AWEBWOWVIEWERCPP_MAPSCENERENDERER_H
