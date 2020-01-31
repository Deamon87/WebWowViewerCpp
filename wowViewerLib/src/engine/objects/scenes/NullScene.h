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

    void doPostLoad(WoWFrameData *frameData) override {};
    void update(WoWFrameData *frameData) override {};
    void updateBuffers(WoWFrameData *frameData) override {};

    HDrawStage produceDrawStage(HUpdateStage updateStage) { return HDrawStage();};
};
#endif //AWEBWOWVIEWERCPP_NULLSCENE_H
