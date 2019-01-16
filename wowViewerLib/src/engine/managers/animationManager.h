//
// Created by deamon on 26.06.17.
//

#ifndef WEBWOWVIEWERCPP_ANIMATIONMANAGER_H
#define WEBWOWVIEWERCPP_ANIMATIONMANAGER_H


#include <vector>
#include "../../include/wowScene.h"
#include "../persistance/header/M2FileHeader.h"
#include "particles/particleEmitter.h"


class AnimationManager {
private:
    IWoWInnerApi *m_api;
    HM2Geom m_m2Geom;
    M2Data *m_m2File;

    int mainAnimationId;
    int mainAnimationIndex;

    int currentAnimationIndex;
    animTime_t currentAnimationTime;
    int currentAnimationPlayedTimes;

    int nextSubAnimationIndex;
    animTime_t nextSubAnimationTime;
    bool nextSubAnimationActive;

    bool firstCalc;
    bool isAnimated;
    bool deferredLoadingStarted = false;

    int leftHandClosed = 0;
    int rightHandClosed = 0;

    std::vector<animTime_t> globalSequenceTimes;
    std::vector<bool> bonesIsCalculated;
    std::vector<mathfu::mat4> blendMatrixArray;
    std::vector<std::vector<int>> childBonesLookup;

    void initBonesIsCalc();
    void initBlendMatrices();
    void initGlobalSequenceTimes();

    int findAnimationIndex(uint32_t anim_id);

    void calculateBoneTree();
    void calcAnimMatrixes (std::vector<mathfu::mat4> &textAnimMatrices, int animationIndex, animTime_t time);

public:
    AnimationManager(IWoWInnerApi *api, HM2Geom m2File);
    bool setAnimationId(int animationId, bool reset);
    void update (
        animTime_t deltaTime,
        mathfu::vec3 &cameraPosInLocal,
        mathfu::vec3 &localUpVector,
        mathfu::vec3 &localRightVector,
        mathfu::mat4 &modelViewMatrix,
        std::vector<mathfu::mat4> &bonesMatrices,
        std::vector<mathfu::mat4> &textAnimMatrices,
        std::vector<mathfu::vec4> &subMeshColors,
        std::vector<float> &transparencies,
        std::vector<M2LightResult> &lights,
        std::vector<ParticleEmitter *> &particleEmitters
        /*cameraDetails, particleEmitters*/);

    void calcBones(std::vector<mathfu::mat4> &boneMatrices,
        int animation, animTime_t time,
        mathfu::vec3 &cameraPosInLocal,
        mathfu::vec3 &localUpVector,
        mathfu::vec3 &localRightVector,
        mathfu::mat4 &modelViewMatrix);

    void calcBoneMatrix(std::vector<mathfu::mat4> &boneMatrices, int boneIndex, int animationIndex, animTime_t time,
                        mathfu::vec3 cameraPosInLocal, mathfu::vec3 &localUpVector, mathfu::vec3 &localRightVector,
                        mathfu::mat4 &modelViewMatrix);

    void calcChildBones(std::vector<mathfu::mat4> &boneMatrices, int boneIndex, int animationIndex, animTime_t time,
                        mathfu::vec3 cameraPosInLocal, mathfu::vec3 &localUpVector, mathfu::vec3 &localRightVector,
                        mathfu::mat4 &modelViewMatrix);

    void calcSubMeshColors(std::vector<mathfu::vec4> &subMeshColors,
            int animationIndex,
            animTime_t time,
            int blendAnimationIndex,
            animTime_t blendAnimationTime,
            float blendAlpha);

    void calcTransparencies(std::vector<float> &transparencies, int animationIndex, animTime_t time,
                            int blendAnimationIndex,
                            animTime_t blendAnimationTime, double blendAlpha);

    bool getIsFirstCalc() {
        return firstCalc;
    }
    bool getIsAnimated() {
        return isAnimated;
    }

    void calcLights(std::vector<M2LightResult> &lights,
                    std::vector<mathfu::mat4> &bonesMatrices,
                    int animationIndex,
                    animTime_t animationTime);
    void calcParticleEmitters(std::vector<ParticleEmitter *> &particleEmitters,
                    std::vector<mathfu::mat4> &bonesMatrices,
                    int animationIndex,
                    animTime_t animationTime);

    void calcCamera(M2CameraResult &camera, int cameraId, mathfu::mat4 &placementMatrix);
};


#endif //WEBWOWVIEWERCPP_ANIMATIONMANAGER_H
