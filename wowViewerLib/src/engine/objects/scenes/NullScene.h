//
// Created by Deamon on 12/21/2019.
//

#ifndef AWEBWOWVIEWERCPP_NULLSCENE_H
#define AWEBWOWVIEWERCPP_NULLSCENE_H

#include "../iScene.h"

class NullScene : public IScene {
public:
    virtual void setReplaceTextureArray(std::vector<int> &replaceTextureArray) override {};
    virtual void setMeshIdArray(std::vector<uint8_t> &meshIds) override {};
    virtual void setAnimationId(int animationId) override {};

    virtual void produceDrawStage(HDrawStage resultDrawStage, HUpdateStage updateStage, std::vector<HGUniformBufferChunk> &additionalChunks) override {
        resultDrawStage->meshesToRender = std::make_shared<MeshesToRender>();
    };

    virtual void checkCulling(HCullStage cullStage) override {};

    virtual void doPostLoad(HCullStage cullStage) override {};
    virtual void update(HUpdateStage updateStage) override {};
    virtual void updateBuffers(HCullStage cullStage) override {};
    virtual void resetAnimation() override {};
    virtual int getCameraNum() override {return 0;};
    virtual std::shared_ptr<ICamera> createCamera(int cameraNum) override { return nullptr;};
};
#endif //AWEBWOWVIEWERCPP_NULLSCENE_H
