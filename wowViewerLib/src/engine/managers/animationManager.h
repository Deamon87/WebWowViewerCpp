//
// Created by deamon on 26.06.17.
//

#ifndef WEBWOWVIEWERCPP_ANIMATIONMANAGER_H
#define WEBWOWVIEWERCPP_ANIMATIONMANAGER_H

class AnimationManager;

#include <vector>
#include "../persistance/header/M2FileHeader.h"
#include "particles/particleEmitter.h"
#include "CRibbonEmitter.h"
#include "../algorithms/animate.h"
#include "../objects/m2/m2Helpers/CBoneMasterData.h"

class AnimationManager {
private:
    HApiContainer m_api;
    std::shared_ptr<CBoneMasterData> boneMasterData;

    FullAnimationInfo animationInfo;

    bool firstCalc = true;
    bool deferredLoadingStarted = false;

    bool isMirrored = false;
    bool m_hasExp2 = false;

    int leftHandClosed = 0;
    int rightHandClosed = 0;

    std::vector<animTime_t> globalSequenceTimes;
    std::vector<animTime_t> parentGlobalSequenceTimes;
    std::vector<bool> bonesIsCalculated;
    std::vector<mathfu::mat4> blendMatrixArray;
    std::vector<std::vector<int>> childBonesLookup;

    void initBonesIsCalc();
    void initBlendMatrices();
    void initGlobalSequenceTimes();

    void calculateBoneTree();
    void calcAnimMatrixes (std::vector<mathfu::mat4> &textAnimMatrices);

    void calcAnimRepetition(AnimationStruct &animationStruct);
public:
    AnimationManager(HApiContainer api, std::shared_ptr<CBoneMasterData> boneMasterData, bool hasExp2);

    void resetCurrentAnimation();
    bool setAnimationId(int animationId, bool reset);
    int getCurrentAnimationIndex();
    void setAnimationPercent(float percent);
    void update (
        animTime_t deltaTime,
        animTime_t deltaTimeForGS,
        mathfu::vec3 &cameraPosInLocal,
        mathfu::vec3 &localUpVector,
        mathfu::vec3 &localRightVector,
        const mathfu::mat4 &modelViewMatrix,
        std::vector<mathfu::mat4> &bonesMatrices,
        std::vector<mathfu::mat4> &textAnimMatrices,
        std::vector<mathfu::vec4> &subMeshColors,
        std::vector<float> &transparencies,
        std::vector<M2LightResult> &lights,
        std::vector<ParticleEmitter *> &particleEmitters,
        std::vector<CRibbonEmitter *> &ribbonEmitters

        /*cameraDetails, particleEmitters*/);

    void calcBones(std::vector<mathfu::mat4> &boneMatrices, const mathfu::mat4 &modelViewMatrix);

    void calcBoneMatrix(std::vector<mathfu::mat4> &boneMatrices, int boneIndex, const mathfu::mat4 &modelViewMatrix);

    void calcChildBones(std::vector<mathfu::mat4> &boneMatrices, int boneIndex, const mathfu::mat4 &modelViewMatrix);

    void calcSubMeshColors(std::vector<mathfu::vec4> &subMeshColors);

    void calcTransparencies(std::vector<float> &transparencies);

    bool getIsFirstCalc() {
        return firstCalc;
    }

    void calcLights(std::vector<M2LightResult> &lights,
                    std::vector<mathfu::mat4> &bonesMatrices);
    void calcParticleEmitters(std::vector<ParticleEmitter *> &particleEmitters,
                    std::vector<mathfu::mat4> &bonesMatrices);

    void calcRibbonEmitters(std::vector<CRibbonEmitter *> &ribbonEmitters);

    void calcCamera(M2CameraResult &camera, int cameraId, mathfu::mat4 &placementMatrix);
};


#endif //WEBWOWVIEWERCPP_ANIMATIONMANAGER_H
