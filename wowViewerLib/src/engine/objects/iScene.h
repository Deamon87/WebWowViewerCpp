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
    virtual void setMeshIdArray(std::vector<uint8_t> &meshIds) = 0;
    virtual void setReplaceParticleColors(std::array<std::array<mathfu::vec4, 3>, 3> &particleColorReplacement) {};
    virtual void resetReplaceParticleColor() {};

    virtual void setAnimationId(int animationId) = 0;
    virtual void resetAnimation() = 0;

    virtual void produceDrawStage(HDrawStage resultDrawStage, HUpdateStage updateStage, std::vector<HGUniformBufferChunk> &additionalChunks) = 0;
    virtual void produceUpdateStage(HUpdateStage updateStage) = 0;

    virtual void checkCulling(HCullStage cullStage) = 0;

    virtual void doPostLoad(HCullStage cullStage) = 0;
    virtual void updateBuffers(HUpdateStage updateStage) = 0;

    virtual int getCameraNum() = 0;
    virtual std::shared_ptr<ICamera> createCamera(int cameraNum) = 0;
};
#endif //WEBWOWVIEWERCPP_IINNERSCENEAPI_H
