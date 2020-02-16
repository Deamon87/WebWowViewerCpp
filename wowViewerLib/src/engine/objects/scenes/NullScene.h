//
// Created by Deamon on 12/21/2019.
//

#ifndef AWEBWOWVIEWERCPP_NULLSCENE_H
#define AWEBWOWVIEWERCPP_NULLSCENE_H

#include "../iScene.h"

class NullScene : public IScene {
public:
    void setReplaceTextureArray(std::vector<int> &replaceTextureArray) override {};
    void setAnimationId(int animationId) override {};

    void checkCulling(WoWFrameData *frameData) override {};
    void collectMeshes(WoWFrameData *frameData) override {};

    void doPostLoad(WoWFrameData *frameData) override {};
    void update(WoWFrameData *frameData) override {};
    void updateBuffers(WoWFrameData *frameData) override {};

    void produceDrawStage(HDrawStage resultDrawStage, HUpdateStage updateStage) override {return ; };
};
#endif //AWEBWOWVIEWERCPP_NULLSCENE_H
