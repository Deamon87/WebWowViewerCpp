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
#include "../objects/m2/m2Helpers/M2GpuAnimData.h"

class AnimationManager {
private:
    HApiContainer m_api;
    std::shared_ptr<CBoneMasterData> boneMasterData;

    FullAnimationInfo animationInfo;

    bool firstCalc = true;
    bool deferredLoadingStarted = false;
    bool m_needToUpdateBB = false;


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
    void initGlobalSequenceTimes();

    void calculateBoneTree();
    void calcAnimMatrixes (std::vector<mathfu::mat4, tbb::cache_aligned_allocator<mathfu::mat4>> &textAnimMatrices);

    void calcAnimRepetition(AnimationStruct &animationStruct);
public:
    AnimationManager(const HApiContainer &api, const std::shared_ptr<CBoneMasterData> &boneMasterData, bool hasExp2);

    void resetCurrentAnimation();
    bool setAnimationId(int animationId, bool reset);
    int getCurrentAnimationIndex();
    void setAnimationPercent(float percent);

    // Sequencing-only part of update(): advances animation/global-sequence time, picks and
    // blends sub-animations, applies the deferred (.anim) loading gate.
    // Returns false when sequence data is not loaded yet and evaluation must be skipped
    // this frame (GPU buffers then keep last frame's values, same as the CPU arrays do).
    bool updateSequencing(animTime_t deltaTime, animTime_t deltaTimeForGS);

    // Evaluation part of update(): computes bone matrices, texture animation matrices,
    // colors, transparencies, lights and emitter properties from the current sequencing state.
    void evaluateAnimation(
        const mathfu::mat4 &modelMatrix,
        const mathfu::mat4 &modelViewMatrix,
        std::vector<mathfu::mat4, tbb::cache_aligned_allocator<mathfu::mat4>> &bonesMatrices,
        std::vector<mathfu::mat4, tbb::cache_aligned_allocator<mathfu::mat4>> &textAnimMatrices,
        std::vector<mathfu::vec4, tbb::cache_aligned_allocator<mathfu::vec4>> &subMeshColors,
        std::vector<float> &transparencies,
        std::vector<M2LightResult> &lights,
        std::vector<std::unique_ptr<ParticleEmitter>> &particleEmitters,
        std::vector<std::unique_ptr<CRibbonEmitter>> &ribbonEmitters);

    void update (
        animTime_t deltaTime,
        animTime_t deltaTimeForGS,
        mathfu::vec3 &cameraPosInLocal,
        mathfu::vec3 &localUpVector,
        mathfu::vec3 &localRightVector,
        const mathfu::mat4 &modelMatrix,
        const mathfu::mat4 &modelViewMatrix,
        std::vector<mathfu::mat4, tbb::cache_aligned_allocator<mathfu::mat4>> &bonesMatrices,
        std::vector<mathfu::mat4, tbb::cache_aligned_allocator<mathfu::mat4>> &textAnimMatrices,
        std::vector<mathfu::vec4, tbb::cache_aligned_allocator<mathfu::vec4>> &subMeshColors,
        std::vector<float> &transparencies,
        std::vector<M2LightResult> &lights,
        std::vector<std::unique_ptr<ParticleEmitter>> &particleEmitters,
        std::vector<std::unique_ptr<CRibbonEmitter>> &ribbonEmitters

        /*cameraDetails, particleEmitters*/);

    void calcBones(std::vector<mathfu::mat4, tbb::cache_aligned_allocator<mathfu::mat4>> &boneMatrices, const mathfu::mat4 &modelViewMatrix);

    // Evaluates only the listed bones (parent chains are covered by recursion).
    // Used on the GPU animation path for the few bones the CPU still needs
    // (transparent-sort centers, light attachments). Results are model-space,
    // same as calcBones output.
    void calcBoneSubset(std::vector<mathfu::mat4, tbb::cache_aligned_allocator<mathfu::mat4>> &boneMatrices,
                        const std::vector<int> &boneIndices,
                        const mathfu::mat4 &modelViewMatrix);

    // Fills the per-object GPU animation state from the current sequencing state.
    // Returns false when the state doesn't fit GPU constraints (too many global sequences).
    bool fillGpuAnimState(GpuM2AnimState &state) const;

    // GPU animation path: evaluates everything the CPU still owns when bone matrices
    // are computed by the GPU compute pass: texture transforms, colors, transparencies,
    // the cpuBoneSubset bones (sorting centers + attachment bones, model-space), lights,
    // and (until the sims move to GPU) the particle/ribbon emitter properties.
    void evaluateForGpuPath(
        const mathfu::mat4 &modelMatrix,
        const mathfu::mat4 &modelViewMatrix,
        std::vector<mathfu::mat4, tbb::cache_aligned_allocator<mathfu::mat4>> &bonesMatrices,
        std::vector<mathfu::mat4, tbb::cache_aligned_allocator<mathfu::mat4>> &textAnimMatrices,
        std::vector<mathfu::vec4, tbb::cache_aligned_allocator<mathfu::vec4>> &subMeshColors,
        std::vector<float> &transparencies,
        std::vector<M2LightResult> &lights,
        std::vector<std::unique_ptr<ParticleEmitter>> &particleEmitters,
        std::vector<std::unique_ptr<CRibbonEmitter>> &ribbonEmitters,
        const std::vector<int> &cpuBoneSubset);

    // The firstUpdate bookkeeping evaluateAnimation() does at the end of a frame;
    // kept separate so the GPU animation path (which skips evaluateAnimation) keeps
    // identical changedData semantics.
    void finishFrameEvaluation();

    void calcBoneMatrix(std::vector<mathfu::mat4, tbb::cache_aligned_allocator<mathfu::mat4>> &boneMatrices, int boneIndex, const mathfu::mat4 &modelViewMatrix);

    void calcChildBones(std::vector<mathfu::mat4, tbb::cache_aligned_allocator<mathfu::mat4>> &boneMatrices, int boneIndex, const mathfu::mat4 &modelViewMatrix);

    void calcSubMeshColors(std::vector<mathfu::vec4, tbb::cache_aligned_allocator<mathfu::vec4>> &subMeshColors);

    void calcTransparencies(std::vector<float> &transparencies);

    bool isNeedUpdateBB() {
        return m_needToUpdateBB;
    }

    M2Bounds getAnimatinonBB() {
        m_needToUpdateBB = false;

        if (this->animationInfo.currentAnimation.animationRecord != nullptr)
            return this->animationInfo.currentAnimation.animationRecord->bounds;

        return {};
    }

    void calcLights(std::vector<M2LightResult> &lights,
                    const std::vector<mathfu::mat4, tbb::cache_aligned_allocator<mathfu::mat4>> &bonesMatrices,
                    const mathfu::mat4 &modelMatrix);
    void calcParticleEmitters(
        const std::vector<std::unique_ptr<ParticleEmitter>> &particleEmitters,
        std::vector<mathfu::mat4, tbb::cache_aligned_allocator<mathfu::mat4>> &bonesMatrices);

    void calcRibbonEmitters(std::vector<std::unique_ptr<CRibbonEmitter>> &ribbonEmitters);

    void calcCamera(M2CameraResult &camera, int cameraId, mathfu::mat4 &placementMatrix);

    inline const std::array<bool, EAnimDataTypeToInt(EAnimDataType::MAX_ANIM_DATA_TYPE)> getCombinedChangedData() const {
        if (animationInfo.blendFactor < 1.0) {
            std::array<bool, EAnimDataTypeToInt(EAnimDataType::MAX_ANIM_DATA_TYPE)> result;

            for (int i = 0; i < EAnimDataTypeToInt(EAnimDataType::MAX_ANIM_DATA_TYPE); i++) {

                auto const &currentChangedData = animationInfo.currentAnimation.changedData;
                auto const &nextSubChangedData = animationInfo.nextSubAnimation.changedData;
                result[i] = currentChangedData[i] || nextSubChangedData[i] ;
            }
            return result;
        }

        return animationInfo.currentAnimation.changedData;
    }
};


#endif //WEBWOWVIEWERCPP_ANIMATIONMANAGER_H
