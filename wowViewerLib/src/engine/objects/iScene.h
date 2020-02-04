//
// Created by deamon on 08.09.17.
//

#ifndef WEBWOWVIEWERCPP_IINNERSCENEAPI_H
#define WEBWOWVIEWERCPP_IINNERSCENEAPI_H

#include "mathfu/glsl_mappings.h"
#include "../persistance/header/M2FileHeader.h"
#include "../DrawStage.h"
#include "wowFrameData.h"
#include "../SceneScenario.h"

class IScene {
public:
    virtual ~IScene() = default;
    virtual void setReplaceTextureArray(std::vector<int> &replaceTextureArray) = 0;

    virtual void setAnimationId(int animationId) = 0;

    virtual HDrawStage produceDrawStage(std::vector<HDrawStage> fbInputs, HUpdateStage updateStage) = 0;

    virtual void checkCulling(HCullStage cullStage) = 0;
    virtual void collectMeshes(WoWFrameData *frameData) = 0;

    virtual void doPostLoad(WoWFrameData *frameData) = 0;
    virtual void update(WoWFrameData *frameData) = 0;
    virtual void updateBuffers(WoWFrameData *frameData) = 0;
};
#endif //WEBWOWVIEWERCPP_IINNERSCENEAPI_H
