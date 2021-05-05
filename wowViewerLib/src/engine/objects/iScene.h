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
#include "../../exporters/IExporter.h"

class IScene {
public:
    virtual ~IScene() = 0;
    virtual void setReplaceTextureArray(std::vector<int> &replaceTextureArray) = 0;
    virtual void setMeshIdArray(std::vector<uint8_t> &meshIds) = 0;
    virtual void setReplaceParticleColors(std::array<std::array<mathfu::vec4, 3>, 3> &particleColorReplacement) {};
    virtual void resetReplaceParticleColor() {};

    virtual void setAnimationId(int animationId) = 0;
    virtual void setMeshIds(std::vector<uint8_t> &meshIds) = 0;
    virtual void resetAnimation() = 0;

    virtual void produceDrawStage(HDrawStage resultDrawStage, HUpdateStage updateStage, std::vector<HGUniformBufferChunk> &additionalChunks) = 0;
    virtual void produceUpdateStage(HUpdateStage updateStage) = 0;

    virtual void checkCulling(HCullStage cullStage) = 0;

    virtual void doPostLoad(HCullStage cullStage) = 0;
    virtual void updateBuffers(HUpdateStage updateStage) = 0;

    virtual int getCameraNum() = 0;
    virtual std::shared_ptr<ICamera> createCamera(int cameraNum) = 0;

    virtual void exportScene(IExporter * exporter) {};

    virtual void setAdtBoundingBoxHolder(HADTBoundingBoxHolder bbHolder) {};
    virtual void setMandatoryADTs(std::vector<std::array<uint8_t, 2>> mandatoryADTs) {};
    virtual void getAdtAreaId(const mathfu::vec4 &cameraPos, int &areaId, int &parentAreaId) {};
};
#endif //WEBWOWVIEWERCPP_IINNERSCENEAPI_H
