//
// Created by Deamon on 12/21/2019.
//

#ifndef AWEBWOWVIEWERCPP_NULLSCENE_H
#define AWEBWOWVIEWERCPP_NULLSCENE_H

#include "../iInnerSceneApi.h"

class NullScene : public iInnerSceneApi {
public:
    void setReplaceTextureArray(std::vector<int> &replaceTextureArray) override {};
    void setAnimationId(int animationId) override {};

    void checkCulling(WoWFrameData *frameData) override {};
    void collectMeshes(WoWFrameData *frameData) override {};
    void draw(WoWFrameData *frameData) override {};

    void doPostLoad(WoWFrameData *frameData) override {};
    void update(WoWFrameData *frameData) override {};
    void updateBuffers(WoWFrameData *frameData) override {};
    mathfu::vec4 getAmbientColor() override {return mathfu::vec4(1,1,1,1);};
    void setAmbientColorOverride(mathfu::vec4 &ambientColor, bool override) override {};
    bool getCameraSettings(M2CameraResult &cameraResult) override {return false;};
};
#endif //AWEBWOWVIEWERCPP_NULLSCENE_H
