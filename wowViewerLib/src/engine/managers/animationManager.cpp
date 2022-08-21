//
// Created by deamon on 26.06.17.
//

#include <cmath>
#include <iostream>
#include "animationManager.h"
#include "../algorithms/animate.h"
#include "../persistance/header/M2FileHeader.h"
#include "mathfu/glsl_mappings.h"

AnimationManager::AnimationManager(HApiContainer api, std::shared_ptr<CBoneMasterData> boneMasterData, bool hasExp2) {
    this->m_api = api;
    this->boneMasterData = boneMasterData;

    this->m_hasExp2 = hasExp2;

    this->animationInfo.currentAnimation.animationIndex = 0;
    this->animationInfo.currentAnimation.animationTime = 0;
    this->animationInfo.currentAnimation.animationRecord = boneMasterData->getSkelData()->m_sequences->getElement(0);
    this->animationInfo.currentAnimation.animationFoundInParent = false;
    this->animationInfo.currentAnimation.mainVariationIndex = 0;
    this->animationInfo.currentAnimation.mainVariationRecord = this->animationInfo.currentAnimation.animationRecord;
    calcAnimRepetition(this->animationInfo.currentAnimation);

    this->animationInfo.nextSubAnimation.animationIndex = -1;
    this->animationInfo.nextSubAnimation.animationTime = 0;
    this->animationInfo.nextSubAnimation.animationRecord = nullptr;

    this->animationInfo.blendFactor = 0;

    this->firstCalc = true;

    this->initBonesIsCalc();
    this->initBlendMatrices();
    this->initGlobalSequenceTimes();
    this->calculateBoneTree();

    if (!this->setAnimationId(0, false)) { // try Stand(0) animation
        this->setAnimationId(147, false); // otherwise try Closed(147) animation
    }
}

void AnimationManager::initBonesIsCalc() {
    auto bones = boneMasterData->getSkelData()->m_m2CompBones;
    bonesIsCalculated = std::vector<bool>((unsigned long) bones->size);

    for (int i = 0; i < bones->size; i++) {
        bonesIsCalculated[i] = false;
    }
}

void AnimationManager::initBlendMatrices() {
    auto &bones = *boneMasterData->getSkelData()->m_m2CompBones;

    unsigned long matCount = (unsigned long) std::max(bones.size, boneMasterData->getM2Geom()->getM2Data()->texture_transforms.size);
    blendMatrixArray = std::vector<mathfu::mat4>(matCount, mathfu::mat4::Identity());
}

void AnimationManager::initGlobalSequenceTimes() {
    globalSequenceTimes = std::vector<animTime_t>(
            (unsigned long) (boneMasterData->getSkelData()->m_globalSequences->size > 0 ?
            boneMasterData->getSkelData()->m_globalSequences->size :
            0));

    for (int i = 0; i < globalSequenceTimes.size(); i++) {
        globalSequenceTimes[i] = 0;
    }

    if (boneMasterData->getParentSkelData() != nullptr) {
        parentGlobalSequenceTimes = std::vector<animTime_t>(
            (unsigned long) (boneMasterData->getParentSkelData()->m_globalSequences->size > 0 ?
                             boneMasterData->getParentSkelData()->m_globalSequences->size :
                             0));

        for (int i = 0; i < parentGlobalSequenceTimes.size(); i++) {
            globalSequenceTimes[i] = 0;
        }
    }
}

void AnimationManager::calculateBoneTree() {
    auto &bones = *boneMasterData->getSkelData()->m_m2CompBones;
    this->childBonesLookup = std::vector<std::vector<int>>(bones.size);
    for (int i = 0; i < bones.size; i++) {
        for (int j = 0; j < bones.size; j++) {
            if (bones[j]->parent_bone == i) {
                childBonesLookup[i].push_back(j);
            }
        }
    }
}

void AnimationManager::resetCurrentAnimation() {
    this->animationInfo.currentAnimation.animationTime = 0;

    for (auto &a : globalSequenceTimes) {
        a = 0;
    }
    for (auto &a : parentGlobalSequenceTimes) {
        a = 0;
    }
}

bool AnimationManager::setAnimationId(int animationId, bool reset) {
    int animationIndex = -1;
    bool animationFoundInParent = false;

    animationIndex = findAnimationIndex(animationId,
                                        boneMasterData->getSkelData()->m_sequence_lookups,
                                        boneMasterData->getSkelData()->m_sequences);

    auto sequences = boneMasterData->getSkelData()->m_sequences;

    if (animationIndex <= -1 && boneMasterData->getParentSkelData() != nullptr) {
        bool animationIsBanned = false;
        //Test against PABC
        auto &bannedAnims = boneMasterData->getM2Geom()->blackListAnimations;
        for (auto const a : bannedAnims) {
            if (a == animationId) {
                animationIsBanned = true;
                break;
            }
        }

        if (!animationIsBanned) {
            animationIndex = findAnimationIndex(animationId,
                                                boneMasterData->getParentSkelData()->m_sequence_lookups,
                                                boneMasterData->getParentSkelData()->m_sequences);
            if (animationIndex > -1) {
                sequences = boneMasterData->getParentSkelData()->m_sequences;
                animationFoundInParent = true;
            }
        }
    }


    if (animationIndex > -1) {
        while (
            (((*sequences)[animationIndex]->flags & 0x20) == 0) &&
            (((*sequences)[animationIndex]->flags & 0x40) > 0)
        ) {
            animationIndex = (*sequences)[animationIndex]->aliasNext;
            if (animationIndex < 0) break;
        }
    }

    if ((animationIndex > - 1) && (reset || (animationIndex != this->animationInfo.currentAnimation.animationIndex) )) {
        //Reset animation
        this->animationInfo.currentAnimation.animationIndex = animationIndex;
        this->animationInfo.currentAnimation.animationRecord = (*sequences)[animationIndex];
        this->animationInfo.currentAnimation.animationTime = 0;
        this->animationInfo.currentAnimation.animationFoundInParent = animationFoundInParent;
        this->animationInfo.currentAnimation.mainVariationIndex = animationIndex;
        this->animationInfo.currentAnimation.mainVariationRecord = (*sequences)[animationIndex];

        calcAnimRepetition(this->animationInfo.currentAnimation);

        this->animationInfo.nextSubAnimation.animationIndex = -1;
        this->animationInfo.nextSubAnimation.animationRecord = nullptr;
        this->animationInfo.nextSubAnimation.animationTime = 0;
        this->animationInfo.nextSubAnimation.animationFoundInParent = false;
        this->animationInfo.nextSubAnimation.mainVariationIndex = -1;
        this->animationInfo.nextSubAnimation.mainVariationRecord = nullptr;

        this->firstCalc = true;

        deferredLoadingStarted = false;
    }
    return (animationIndex > -1);
}

void blendMatrices(std::vector<mathfu::mat4> &origMat, std::vector<mathfu::mat4> &blendMat, int count, float blendAlpha) {
//Actual blend
    for (int i = 0; i < count; i++) {
//        mathfu::mat4 &blendTransformMatrix = blendMat[i];
//        mathfu::mat4 &tranformMat = origMat[i];
        origMat[i] = ((blendMat[i] - origMat[i]) * (const float &) (1.0 - blendAlpha)) + origMat[i];
    }
}

template <typename T>
inline void calcAnimationTransform(
        mathfu::mat4 &tranformMat,
        mathfu::mat4 *billboardMatrix,
        mathfu::vec4 &pivotPoint,
        mathfu::vec4 &negatePivotPoint,
        M2Array<M2Loop> &global_loops,
        std::vector<animTime_t> &globalSequenceTimes,

        bool &isAnimated,
        M2Track<C3Vector> &translationTrack,
        M2Track<T> &rotationTrack,
        M2Track<C3Vector> &scaleTrack,
        const FullAnimationInfo &animationInfo
        ) {
    tranformMat = tranformMat * mathfu::mat4::FromTranslationVector(pivotPoint.xyz());
//
    if (translationTrack.values.size > 0) {
        mathfu::vec4 defaultValue = mathfu::vec4(0,0,0,0);
        mathfu::vec4 transVec = animateTrackWithBlend<C3Vector, mathfu::vec4>(
            animationInfo,
            translationTrack,
            global_loops,
            globalSequenceTimes,
            defaultValue
        );

        tranformMat = tranformMat * mathfu::mat4::FromTranslationVector(transVec.xyz());
        isAnimated = true;
    }

    if (billboardMatrix != nullptr) {
        tranformMat = tranformMat * *billboardMatrix;
    } else if (rotationTrack.values.size > 0) {
        mathfu::quat defaultValue = mathfu::quat(1,0,0,0);
        mathfu::quat quaternionResult = animateTrackWithBlend<T, mathfu::quat>(
            animationInfo,
            rotationTrack,
            global_loops,
            globalSequenceTimes,
            defaultValue);

        tranformMat = tranformMat * quaternionResult.ToMatrix4();
        isAnimated = true;
    }

    if (scaleTrack.values.size > 0) {
        mathfu::vec4 defaultValue = mathfu::vec4(1,1,1,1);
        mathfu::vec4 scaleResult = animateTrackWithBlend<C3Vector, mathfu::vec4>(
            animationInfo,
            scaleTrack,
            global_loops,
            globalSequenceTimes,
            defaultValue);

        tranformMat = tranformMat * mathfu::mat4::FromScaleVector(scaleResult.xyz());
        isAnimated = true;
    }
    tranformMat = tranformMat * mathfu::mat4::FromTranslationVector(negatePivotPoint.xyz());
}
template <typename T>
inline void calcTextureAnimationTransform(
    mathfu::mat4 &tranformMat,
    mathfu::vec4 &pivotPoint,
    mathfu::vec4 &negatePivotPoint,
    M2Array<M2Loop> &global_loops,
    std::vector<animTime_t> &globalSequenceTimes,
    bool &isAnimated,
    M2Track<C3Vector> &translationTrack,
    M2Track<T> &rotationTrack,
    M2Track<C3Vector> &scaleTrack,
    const FullAnimationInfo &animationInfo
) {

    if (rotationTrack.values.size > 0) {
        mathfu::quat defaultValue = mathfu::quat(1,0,0,0);
        mathfu::quat quaternionResult = animateTrackWithBlend<T, mathfu::quat>(
            animationInfo,
            rotationTrack,
            global_loops,
            globalSequenceTimes,
            defaultValue);

        tranformMat = tranformMat * mathfu::mat4::FromTranslationVector(pivotPoint.xyz());
        tranformMat = tranformMat * quaternionResult.ToMatrix4();
        tranformMat = tranformMat * mathfu::mat4::FromTranslationVector(negatePivotPoint.xyz());
        isAnimated = true;
    }
    if (scaleTrack.values.size > 0) {
        mathfu::vec4 defaultValue = mathfu::vec4(1,1,1,1);
        mathfu::vec4 scaleResult = animateTrackWithBlend<C3Vector, mathfu::vec4>(
            animationInfo,
            scaleTrack,
            global_loops,
            globalSequenceTimes,
            defaultValue);

        tranformMat = tranformMat * mathfu::mat4::FromTranslationVector(pivotPoint.xyz());
        tranformMat = tranformMat * mathfu::mat4::FromScaleVector(scaleResult.xyz());
        tranformMat = tranformMat * mathfu::mat4::FromTranslationVector(negatePivotPoint.xyz());
        isAnimated = true;
    }

    if (translationTrack.values.size > 0) {
        mathfu::vec4 defaultValue = mathfu::vec4(0,0,0,0);
        mathfu::vec4 transVec = animateTrackWithBlend<C3Vector, mathfu::vec4>(
            animationInfo,
            translationTrack,
            global_loops,
            globalSequenceTimes,
            defaultValue
        );

        tranformMat = tranformMat * mathfu::mat4::FromTranslationVector(transVec.xyz());
        isAnimated = true;
    }
}


void AnimationManager::calcAnimMatrixes (std::vector<mathfu::mat4> &textAnimMatrices) {

    mathfu::vec4 pivotPoint(0.5, 0.5, 0, 0);
    mathfu::vec4 negatePivotPoint = -pivotPoint;

    auto &global_loops = *boneMasterData->getSkelData()->m_globalSequences;

    auto &texture_transforms = boneMasterData->getM2Geom()->getM2Data()->texture_transforms;
    for (int i = 0; i < texture_transforms.size; i++) {
        M2TextureTransform *textAnimData = texture_transforms[i];

        textAnimMatrices[i] = mathfu::mat4::Identity();
        bool isAnimated;
        calcTextureAnimationTransform(textAnimMatrices[i],
            pivotPoint, negatePivotPoint,
            global_loops,
            this->globalSequenceTimes,
            isAnimated,
            textAnimData->translation,
            textAnimData->rotation,
            textAnimData->scaling,
            animationInfo
        );
    }
}

void calcBoneBillboardMatrix(
                mathfu::mat4 *billboardMatrix,
                std::vector<mathfu::mat4> &boneMatrices,
                M2CompBone *boneDefinition,
                int parentBone,
                mathfu::vec4 &pivotPoint,
                mathfu::vec3 &cameraPosInLocal,
                mathfu::vec3 &localUpVector,
                mathfu::vec3 &localRightVector
                ) {

    mathfu::vec3 modelForward;
    mathfu::vec3 modelUp;
    mathfu::vec3 modelRight;

    mathfu::vec4 cameraPoint = mathfu::vec4(cameraPosInLocal, 0);
    if (parentBone >= 0) {
        mathfu::mat4 *parentMatrix = &boneMatrices[parentBone];
        cameraPoint = parentMatrix->Inverse() * cameraPoint;
    }

    cameraPoint = cameraPoint - pivotPoint;

    modelForward = cameraPoint.xyz().Normalized();

    if ((boneDefinition->flags.cylindrical_billboard_lock_z) > 0) {
        //Cylindrical billboard
//        if (mathfu::vec3::DotProduct(modelForward, localUpVector) > 0.01) {
            modelUp = localUpVector;
            modelRight = mathfu::vec3::CrossProduct(modelUp, modelForward).Normalized();
            modelForward = mathfu::vec3::CrossProduct(modelRight, modelUp).Normalized();

            modelRight = mathfu::vec3::CrossProduct(modelUp, modelForward).Normalized();
//        } else {
//
//        }
    } else if ((boneDefinition->flags.spherical_billboard) > 0) {
//        float dotresult = mathfu::vec3::DotProduct(modelForward, localUpVector);
//        dotresult = 1.0;
//        if (fabs(dotresult) <= 0.98) {
            //Spherical billboard
            modelUp = localUpVector;
            modelRight = mathfu::vec3::CrossProduct(modelUp, modelForward).Normalized();
            if (mathfu::vec3::DotProduct(modelRight, localRightVector) < 0) {
//                modelRight = -modelRight;
            }

            modelUp = mathfu::vec3::CrossProduct(modelForward, modelRight).Normalized();
//        } else {
//            modelRight = localRightVector;
//            modelUp = mathfu::vec3::CrossProduct(modelForward, modelRight).Normalized();
//            if (mathfu::vec3::DotProduct(modelUp, localUpVector) < 0) {
//                modelUp = -modelUp;
//            }
//
//            modelRight = mathfu::vec3::CrossProduct(modelUp, modelForward).Normalized();
//        }
    } else {
        std::cout << "This billboarding is not implemented";
    }


    *billboardMatrix = mathfu::mat4(
            modelForward[0], modelForward[1], modelForward[2], 0,
            modelRight[0], modelRight[1], modelRight[2], 0,
            modelUp[0], modelUp[1], modelUp[2], 0,
            0, 0, 0, 1
    );
}

void
AnimationManager::calcBoneMatrix(
    std::vector<mathfu::mat4> &boneMatrices,
    int boneIndex,
    const mathfu::mat4 &modelViewMatrix
    ) {
    if (this->bonesIsCalculated[boneIndex]) return;

    auto skelData = boneMasterData->getSkelData();
    auto globalSequenceTimes = &this->globalSequenceTimes;
    if (this->animationInfo.currentAnimation.animationFoundInParent) {
        skelData = boneMasterData->getParentSkelData();
        globalSequenceTimes = &this->parentGlobalSequenceTimes;
    }



    M2CompBone *boneDefinition = skelData->m_m2CompBones->getElement(boneIndex);
    auto &globalSequences = skelData->m_globalSequences;

    int parentBone = boneDefinition->parent_bone;

    /* 2. Prepare bone part of animation process */
    boneMatrices[boneIndex] = mathfu::mat4::Identity();


    mathfu::mat4 parentBoneMat = modelViewMatrix;
    if (parentBone >= 0) {
        this->calcBoneMatrix(boneMatrices, parentBone, modelViewMatrix);
        parentBoneMat = boneMatrices[parentBone];
        mathfu::mat4 modifiedMatrixUnder0x7 = parentBoneMat;

        if (boneDefinition->flags.unk_0x1 || boneDefinition->flags.unk_0x2 || boneDefinition->flags.unk_0x4) {
            if (boneDefinition->flags.unk_0x4 && boneDefinition->flags.unk_0x2) {
                //6
                modifiedMatrixUnder0x7.GetColumn(0) = modelViewMatrix.GetColumn(0);
                modifiedMatrixUnder0x7.GetColumn(1) = modelViewMatrix.GetColumn(1);
                modifiedMatrixUnder0x7.GetColumn(2) = modelViewMatrix.GetColumn(2);
            } else if (boneDefinition->flags.unk_0x4) {
                //4
                float coeff = 1.0f;
                coeff = modelViewMatrix.GetColumn(0).LengthSquared() ;
                coeff = (coeff > 0.0000099999997) ? sqrtf(modifiedMatrixUnder0x7.GetColumn(0).LengthSquared() / coeff) : 1.0f;
                modifiedMatrixUnder0x7.GetColumn(0) = modelViewMatrix.GetColumn(0) * coeff;

                coeff = modelViewMatrix.GetColumn(1).LengthSquared() ;
                coeff = (coeff > 0.0000099999997) ? sqrtf(modifiedMatrixUnder0x7.GetColumn(1).LengthSquared() / coeff) : 1.0f;
                modifiedMatrixUnder0x7.GetColumn(1) = modelViewMatrix.GetColumn(1) * coeff;

                coeff = modelViewMatrix.GetColumn(2).LengthSquared() ;
                coeff = (coeff > 0.0000099999997) ? sqrtf(modifiedMatrixUnder0x7.GetColumn(2).LengthSquared() / coeff) : 1.0f;
                modifiedMatrixUnder0x7.GetColumn(2) = modelViewMatrix.GetColumn(2) * coeff;
            } else if (boneDefinition->flags.unk_0x2) {
                float coeff = 1.0f;
                coeff = modifiedMatrixUnder0x7.GetColumn(0).LengthSquared() ;
                if ( coeff > 0.0000099999997) {
                    modifiedMatrixUnder0x7.GetColumn(0) = modelViewMatrix.GetColumn(0) * (1.0f / sqrt(coeff));
                }
                coeff = modifiedMatrixUnder0x7.GetColumn(1).LengthSquared() ;
                if ( coeff > 0.0000099999997) {
                    modifiedMatrixUnder0x7.GetColumn(1) = modelViewMatrix.GetColumn(1) * (1.0f / sqrt(coeff));
                }
                coeff = modifiedMatrixUnder0x7.GetColumn(2).LengthSquared() ;
                if ( coeff > 0.0000099999997) {
                    modifiedMatrixUnder0x7.GetColumn(2) = modelViewMatrix.GetColumn(2) * (1.0f / sqrt(coeff));
                }

                modifiedMatrixUnder0x7.GetColumn(0) = modifiedMatrixUnder0x7.GetColumn(0) * modelViewMatrix.GetColumn(0).Length();
                modifiedMatrixUnder0x7.GetColumn(1) = modifiedMatrixUnder0x7.GetColumn(1) * modelViewMatrix.GetColumn(1).Length();
                modifiedMatrixUnder0x7.GetColumn(2) = modifiedMatrixUnder0x7.GetColumn(2) * modelViewMatrix.GetColumn(2).Length();
            }
            if (boneDefinition->flags.unk_0x1) {
                modifiedMatrixUnder0x7.GetColumn(3) = modelViewMatrix.GetColumn(3);
            } else {
                mathfu::vec4 pivotVec4 = mathfu::vec4(mathfu::vec3(boneDefinition->pivot), 1.0);
                mathfu::vec4 pivotVec3 = pivotVec4; pivotVec3.w = 0.0;

                modifiedMatrixUnder0x7.GetColumn(3) = (parentBoneMat * pivotVec4 ) - (modifiedMatrixUnder0x7 * pivotVec3);
                modifiedMatrixUnder0x7.GetColumn(3).w = 1.0f;
            }


            parentBoneMat = modifiedMatrixUnder0x7;
        }
    }

    C3Vector pP = boneDefinition->pivot;
    mathfu::vec4 pivotPoint = mathfu::vec4(pP.x, pP.y, pP.z, 0);
    mathfu::vec4 negatePivotPoint = -mathfu::vec4(pP.x, pP.y, pP.z, 0);


    /* 2.1 Calculate billboard matrix if needed */
    mathfu::mat4 *billboardMatrix = nullptr;

    /* 3. Calculate matrix */
    bool isAnimated = (boneDefinition->flags_raw & 0x280) > 0;
    mathfu::mat4 animatedMatrix = mathfu::mat4::Identity();
    if (isAnimated) {
        calcAnimationTransform(animatedMatrix,
                               billboardMatrix,
                               pivotPoint, negatePivotPoint,
                               *globalSequences,
                               *globalSequenceTimes,
                               isAnimated,
                               boneDefinition->translation,
                               boneDefinition->rotation,
                               boneDefinition->scaling,
                               animationInfo);
        boneMatrices[boneIndex] = parentBoneMat * animatedMatrix;
    } else {
        boneMatrices[boneIndex] = parentBoneMat;
    }


    int boneBillboardFlags = boneDefinition->flags_raw & 0x4000078;
    if (boneBillboardFlags) {
        mathfu::mat4 &currentBoneMat = boneMatrices[boneIndex];
        mathfu::mat4 currentBoneMatCopy = currentBoneMat;
        mathfu::vec3 scaleVector = mathfu::vec3(
            currentBoneMat.GetColumn(0).Length(),
            currentBoneMat.GetColumn(1).Length(),
            currentBoneMat.GetColumn(2).Length()
        );

        switch (boneBillboardFlags) {
            case 0x8: {
                if (isAnimated) {
                    mathfu::vec4 &xAxis = animatedMatrix.GetColumn(0);
                    currentBoneMat.GetColumn(0) = mathfu::vec4(
                        xAxis.y,
                        xAxis.z,
                        -xAxis.x,
                        0
                    ).Normalized();

                    mathfu::vec4 &yAxis = animatedMatrix.GetColumn(1);
                    if (isMirrored) {
                        currentBoneMat.GetColumn(1) = mathfu::vec4(
                            -yAxis.y,
                            -yAxis.z,
                            yAxis.x,
                            0
                        ).Normalized();
                    } else {
                        currentBoneMat.GetColumn(1) = mathfu::vec4(
                            yAxis.y,
                            yAxis.z,
                            -yAxis.x,
                            0
                        ).Normalized();
                    }

                    mathfu::vec4 &zAxis = animatedMatrix.GetColumn(2);
                    currentBoneMat.GetColumn(2) = mathfu::vec4(
                        zAxis.y,
                        zAxis.z,
                        -zAxis.x,
                        0
                    ).Normalized();
                } else {
                    currentBoneMat.GetColumn(0) = mathfu::vec4(0, 0, -1, 0);

                    if (isMirrored) {
                        currentBoneMat.GetColumn(1) = mathfu::vec4(-1.0, 0, 0, 0);
                    } else {
                        currentBoneMat.GetColumn(1) = mathfu::vec4(1.0, 0, 0, 0);
                    }

                    currentBoneMat.GetColumn(2) = mathfu::vec4(0, 1.0, 0, 0);
                }
                break;
            }
            case 0x10: {
                currentBoneMat.GetColumn(0) = currentBoneMat.GetColumn(0).Normalized();

                mathfu::vec4 newYAxis = mathfu::vec4(
                    currentBoneMat(1, 0),
                    -currentBoneMat(0, 0),
                    0.0f,
                    0
                );
                currentBoneMat.GetColumn(1) = newYAxis.Normalized();
                currentBoneMat.GetColumn(2) = mathfu::vec4(
                    mathfu::CrossProductHelper(
                        currentBoneMat.GetColumn(1).xyz(),
                        currentBoneMat.GetColumn(0).xyz()
                    ),
                    0.0f
                );
                break;
            }
            case 0x20: {
                float yAxisLen = currentBoneMat.GetColumn(1).Length();
                currentBoneMat.GetColumn(1) = currentBoneMat.GetColumn(1) * (1.0f /yAxisLen);

                mathfu::vec4 newXAxis = mathfu::vec4(
                    -currentBoneMat(1, 1),
                    currentBoneMat(0, 1),
                    0.0f,
                    0
                );
                currentBoneMat.GetColumn(0) = newXAxis.Normalized();
                currentBoneMat.GetColumn(2) = mathfu::vec4(
                    mathfu::CrossProductHelper(
                        currentBoneMat.GetColumn(1).xyz(),
                        currentBoneMat.GetColumn(0).xyz()
                    ),
                    0.0f
                );
                break;
            }
            case 0x40: {
                currentBoneMat.GetColumn(2) = currentBoneMat.GetColumn(2).Normalized();
                mathfu::vec4 newYAxis = mathfu::vec4(
                    currentBoneMat.GetColumn(2).y,
                    -currentBoneMat.GetColumn(2).x,
                    0.0f,
                    0
                );
                currentBoneMat.GetColumn(1) = newYAxis.Normalized();
                currentBoneMat.GetColumn(0) = mathfu::vec4(
                    mathfu::CrossProductHelper(
                        currentBoneMat.GetColumn(2).xyz(),
                        currentBoneMat.GetColumn(1).xyz()
                    ),
                    0.0f
                );
                break;
            }
            case 0x4000000: {
                mathfu::vec4 pivotVec4 = mathfu::vec4(mathfu::vec3(boneDefinition->pivot), 1.0);
                auto transfVec = currentBoneMatCopy * pivotVec4;

                currentBoneMat.GetColumn(0) = mathfu::vec4(-transfVec.x, -transfVec.y, -transfVec.z, 0).Normalized();
                currentBoneMat.GetColumn(1) = mathfu::vec4(
                    -currentBoneMat.GetColumn(0).z,
                    0,
                    0,
                    0).Normalized();
                currentBoneMat.GetColumn(2) = mathfu::vec4(
                    mathfu::CrossProductHelper(
                        currentBoneMat.GetColumn(1).xyz(),
                        currentBoneMat.GetColumn(0).xyz()
                        ),
                    0.0f
                );

                break;
            }

        }

        mathfu::vec4 pivotVec4 = mathfu::vec4(mathfu::vec3(boneDefinition->pivot), 1.0);
        mathfu::vec4 pivotVec3 = pivotVec4; pivotVec3.w = 0.0;

        currentBoneMat.GetColumn(0) *= scaleVector.x;
        currentBoneMat.GetColumn(1) *= scaleVector.y;
        currentBoneMat.GetColumn(2) *= scaleVector.z;
//        currentBoneMat *= mathfu::mat4::FromScaleVector(scaleVector);
        currentBoneMat.GetColumn(3) = (currentBoneMatCopy * pivotVec4 ) - (currentBoneMat * pivotVec3);
        currentBoneMat.GetColumn(3).w = 1.0;
    }

    this->bonesIsCalculated[boneIndex] = true;
}

void AnimationManager::calcChildBones(
    std::vector<mathfu::mat4> &boneMatrices,
    int boneIndex,
    const mathfu::mat4 &modelViewMatrix) {
    std::vector<int> *childBones = &this->childBonesLookup[boneIndex];
    for (int i = 0; i < childBones->size(); i++) {
        int childBoneIndex = (*childBones)[i];
        this->bonesIsCalculated[childBoneIndex] = false;
        this->calcBoneMatrix(boneMatrices, childBoneIndex, modelViewMatrix);
        this->calcChildBones(boneMatrices, childBoneIndex, modelViewMatrix);
    }
}
std::string dumpMatrix(mathfu::mat4 &mat4) {
    return "[ "+
           std::to_string(mat4[0])+" "+std::to_string(mat4[1])+" "+std::to_string(mat4[2])+" "+std::to_string(mat4[3])+"\n"+
           std::to_string(mat4[4])+" "+std::to_string(mat4[5])+" "+std::to_string(mat4[6])+" "+std::to_string(mat4[7])+"\n"+
           std::to_string(mat4[8])+" "+std::to_string(mat4[9])+" "+std::to_string(mat4[10])+" "+std::to_string(mat4[11])+"\n"+
           std::to_string(mat4[12])+" "+std::to_string(mat4[13])+" "+std::to_string(mat4[14])+" "+std::to_string(mat4[15])+"\n";
}
void AnimationManager::calcBones (
    std::vector<mathfu::mat4> &boneMatrices,
    const mathfu::mat4 &modelViewMatrix) {


    if (true) {
        //Animate everything with standard animation
        auto &bones = *boneMasterData->getSkelData()->m_m2CompBones;
        for (int i = 0; i < bones.size; i++) {
            this->calcBoneMatrix(boneMatrices, i, modelViewMatrix);
//            std::cout << "boneMatrices[" << std::to_string(i) << "] = " << dumpMatrix(boneMatrices[i]) << std::endl;
        }

        /* Animate mouth */
        /*
         if (m2File.keyBoneLookup[6] > -1) { // BONE_HEAD = 6
         var boneId = m2File.keyBoneLookup[6];
         this.calcBoneMatrix(boneId, this.bones[boneId], animation, time, cameraPos, invPlacementMat);
         }
         if (m2File.keyBoneLookup[7] > -1) { // BONE_JAW = 7
         var boneId = m2File.keyBoneLookup[7];
         this.calcBoneMatrix(boneId, this.bones[boneId], animation, time, cameraPos, invPlacementMat);
         }
         */

//        int closedHandAnimation = -1;
//        closedHandAnimation = findAnimationIndex(15); //ANIMATION_HANDSCLOSED = 15

//        if (closedHandAnimation >= 0) {
//            if (this->leftHandClosed && m_m2File->key_bone_lookup.size > (13+5)) {
//                for (int j = 0; j < 5; j++) {
//                    if (*m_m2File->key_bone_lookup[13 + j] > -1) { // BONE_LFINGER1 = 13
//                        int boneId = *m_m2File->key_bone_lookup[13 + j];
//                        this->bonesIsCalculated[boneId] = false;
//                        this->calcBoneMatrix(boneMatrices, boneId, closedHandAnimation, 1, modelViewMatrix);
//                        this->calcChildBones(boneMatrices, boneId, closedHandAnimation, 1, modelViewMatrix);
//                    }
//                }
//            }
//            if (this->rightHandClosed && m_m2File->key_bone_lookup.size > (8+5)) {
//                for (int j = 0; j < 5; j++) {
//                    if (*m_m2File->key_bone_lookup[8 + j] > -1) { // BONE_RFINGER1 = 8
//                        int boneId = *m_m2File->key_bone_lookup[8 + j];
//                        this->bonesIsCalculated[boneId] = false;
//                        this->calcBoneMatrix(boneMatrices, boneId, closedHandAnimation, 1, modelViewMatrix);
//                        this->calcChildBones(boneMatrices, boneId, closedHandAnimation, 1, modelViewMatrix);
//                    }
//                }
//            }
//        }
    }
    this->firstCalc = false;
}


static bool dump = false;

void AnimationManager::update(
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
    std::vector<CRibbonEmitter *> &ribbonEmitters) {


    auto &global_loops = *boneMasterData->getSkelData()->m_globalSequences;
    auto &bones = *boneMasterData->getSkelData()->m_m2CompBones;

    {
        auto sequences = *boneMasterData->getSkelData()->m_sequences;
        if (sequences.size <= 0) return;
    }

    const M2Sequence* currentAnimationRecord = this->animationInfo.currentAnimation.animationRecord;

    this->animationInfo.currentAnimation.animationTime += deltaTime;
    //Update global sequences
    int minGlobal = 0;
    int maxGlobal = (int) this->globalSequenceTimes.size();

    for (int i = minGlobal; i < maxGlobal; i++) {
        this->globalSequenceTimes[i] += deltaTimeForGS;
        if (global_loops[i]->timestamp > 0) { // Global sequence values can be 0's
            this->globalSequenceTimes[i] = fmodf(this->globalSequenceTimes[i], (float)global_loops[i]->timestamp);
        }
    }

    if (boneMasterData->getParentSkelData() != nullptr) {
        minGlobal = 0;
        maxGlobal = (int) this->parentGlobalSequenceTimes.size();

        for (int i = minGlobal; i < maxGlobal; i++) {
            this->parentGlobalSequenceTimes[i] += deltaTimeForGS;
            if (global_loops[i]->timestamp > 0) { // Global sequence values can be 0's
                this->parentGlobalSequenceTimes[i] = fmodf(this->parentGlobalSequenceTimes[i], (float)global_loops[i]->timestamp);
            }
        }
    }

    /* Pick next animation if there is one and no next animation was picked before */
    const M2Sequence* subAnimRecord = nullptr;
    if (this->animationInfo.nextSubAnimation.animationIndex < 0 &&
        this->animationInfo.currentAnimation.mainVariationRecord->variationNext > -1 &&
        this->animationInfo.currentAnimation.repeatTimes <= 0
    ) {
        M2Array<M2Sequence> * sequences = boneMasterData->getSkelData()->m_sequences;
        if (this->animationInfo.currentAnimation.animationFoundInParent) {
            sequences = boneMasterData->getParentSkelData()->m_sequences;;
        }

        //if (currentAnimationPlayedTimes)
        int probability = ((float)rand() / (float)RAND_MAX) * 0x7FFF;
        int calcProb = 0;

        /* First iteration is out of loop */
        auto currentSubAnimIndex = this->animationInfo.currentAnimation.mainVariationIndex;
        subAnimRecord = (*sequences)[currentSubAnimIndex];
        calcProb += subAnimRecord->frequency;
        while ((calcProb < probability) && (subAnimRecord->variationNext > -1)) {
            currentSubAnimIndex = subAnimRecord->variationNext;
            subAnimRecord = (*sequences)[currentSubAnimIndex];

            if (this->animationInfo.currentAnimation.animationIndex != currentSubAnimIndex) {
                calcProb += subAnimRecord->frequency;
            }
        }

        this->animationInfo.nextSubAnimation.animationIndex = currentSubAnimIndex;
        this->animationInfo.nextSubAnimation.animationRecord = (*sequences)[currentSubAnimIndex];
        this->animationInfo.nextSubAnimation.animationTime = 0;
        this->animationInfo.nextSubAnimation.animationFoundInParent = this->animationInfo.currentAnimation.animationFoundInParent;
        this->animationInfo.nextSubAnimation.mainVariationIndex = this->animationInfo.currentAnimation.mainVariationIndex;
        this->animationInfo.nextSubAnimation.mainVariationRecord = this->animationInfo.currentAnimation.mainVariationRecord;
        calcAnimRepetition(this->animationInfo.nextSubAnimation);
    } else {
        //This is done to trigger blending in transition start and end of same variation when it's repeated for whatever reason
        if (this->animationInfo.currentAnimation.repeatTimes > 0) {
            this->animationInfo.nextSubAnimation = this->animationInfo.currentAnimation;
            this->animationInfo.nextSubAnimation.repeatTimes--;
        }
    }

    animTime_t currAnimLeft = currentAnimationRecord->duration - this->animationInfo.currentAnimation.animationTime;

    animTime_t subAnimBlendTime = 0;

    if (this->animationInfo.nextSubAnimation.animationIndex > -1
        //&& ((this->animationInfo.nextSubAnimation.animationRecord->flags & 0x80) > 0)
    ) {
        M2Array<M2Sequence> * sequences = boneMasterData->getSkelData()->m_sequences;
        if (this->animationInfo.nextSubAnimation.animationFoundInParent) {
            sequences = boneMasterData->getParentSkelData()->m_sequences;;
        }

        subAnimRecord = (*sequences)[this->animationInfo.nextSubAnimation.animationIndex];
        subAnimBlendTime = subAnimRecord->blendtime;
    }

    const M2Sequence* blendAnimationRecord = nullptr;
    if ((subAnimBlendTime > 0) && (currAnimLeft < subAnimBlendTime)) {
        this->firstCalc = true;
        this->animationInfo.nextSubAnimation.animationTime = fmod((subAnimBlendTime - currAnimLeft), (subAnimRecord->duration));
        this->animationInfo.blendFactor = (float) (currAnimLeft / subAnimBlendTime);
    } else {
        this->animationInfo.blendFactor = 1.0;
    }

    if (this->animationInfo.currentAnimation.animationTime >= currentAnimationRecord->duration) {
        this->animationInfo.currentAnimation.repeatTimes--;

        if (this->animationInfo.nextSubAnimation.animationIndex > -1) {
            M2Array<M2Sequence> * sequences = boneMasterData->getSkelData()->m_sequences;
            if (this->animationInfo.nextSubAnimation.animationFoundInParent) {
                sequences = boneMasterData->getParentSkelData()->m_sequences;;
            }

            while (
                (((*sequences)[this->animationInfo.nextSubAnimation.animationIndex]->flags & 0x20) == 0) &&
                (((*sequences)[this->animationInfo.nextSubAnimation.animationIndex]->flags & 0x40) > 0)
                ) {
                this->animationInfo.nextSubAnimation.animationIndex = (*sequences)[this->animationInfo.nextSubAnimation.animationIndex]->aliasNext;
                this->animationInfo.nextSubAnimation.animationRecord = (*sequences)[this->animationInfo.nextSubAnimation.animationIndex];
                if (this->animationInfo.nextSubAnimation.animationIndex < 0) break;
            }

            this->animationInfo.currentAnimation = this->animationInfo.nextSubAnimation;

            this->firstCalc = true;

            this->animationInfo.nextSubAnimation.animationIndex = -1;
            this->animationInfo.nextSubAnimation.animationRecord = nullptr;
            this->animationInfo.blendFactor = 1.0;
            deferredLoadingStarted = false;
        } else if (currentAnimationRecord->duration > 0) {
            //Ideally this code would never be called again
            this->animationInfo.currentAnimation.animationTime = fmod(this->animationInfo.currentAnimation.animationTime, currentAnimationRecord->duration);
        }
    }


    /* Update animated values */
    if ((currentAnimationRecord->flags & 0x20) == 0) {
        if (!deferredLoadingStarted) {
            boneMasterData->loadLowPriority(m_api,
                                            currentAnimationRecord->id,
                                            currentAnimationRecord->variationIndex,
                                            animationInfo.currentAnimation.animationFoundInParent);
            deferredLoadingStarted = true;
        }
        return;
    } else {
        deferredLoadingStarted = false;
    };
    if ((animationInfo.nextSubAnimation.animationRecord != nullptr) && ((animationInfo.nextSubAnimation.animationRecord->flags & 0x20) == 0)) {
        if (!deferredLoadingStarted) {
            boneMasterData->loadLowPriority(m_api,
                                            animationInfo.nextSubAnimation.animationRecord->id,
                                            animationInfo.nextSubAnimation.animationRecord->variationIndex,
                                            animationInfo.nextSubAnimation.animationFoundInParent);
            deferredLoadingStarted = true;
        }
        return;
    } else {
        deferredLoadingStarted = false;
    };

    this->calcAnimMatrixes(textAnimMatrices);

    for (int i = 0; i < bones.size; i++) {
        this->bonesIsCalculated[i] = false;
    }
    this->calcBones(bonesMatrices, modelViewMatrix);

    mathfu::mat4 invModelViewMatrix = modelViewMatrix.Inverse();
    for (int i = 0; i < bones.size; i++) {
        bonesMatrices[i] = invModelViewMatrix * bonesMatrices[i];
    }

    this->calcSubMeshColors(subMeshColors);
    this->calcTransparencies(transparencies);

//    this->calcCameras(cameraDetails, this->currentAnimationIndex, this->currentAnimationTime);
    this->calcLights(lights, bonesMatrices);
    this->calcParticleEmitters(particleEmitters, bonesMatrices);
    this->calcRibbonEmitters(ribbonEmitters);
}

void AnimationManager::calcSubMeshColors(std::vector<mathfu::vec4> &subMeshColors) {
    M2Array<M2Color> &colors = boneMasterData->getM2Geom()->getM2Data()->colors;
    auto &global_loops = *boneMasterData->getSkelData()->m_globalSequences;

    static mathfu::vec3 defaultVector(1.0, 1.0, 1.0);
    static float defaultAlpha = 1.0;

    for (int i = 0; i < colors.size; i++) {
        mathfu::vec3 colorResult1 = animateTrackWithBlend<C3Vector, mathfu::vec3>(
            animationInfo,
            colors[i]->color,
            global_loops,
            this->globalSequenceTimes,
            defaultVector
        );

        subMeshColors[i].x = colorResult1.x;
        subMeshColors[i].y = colorResult1.y;
        subMeshColors[i].z = colorResult1.z;

        float resultAlpha1 = animateTrackWithBlend<fixed16, float>(
            animationInfo,
            colors[i]->alpha,
            global_loops,
            this->globalSequenceTimes,
            defaultAlpha
        );

        subMeshColors[i][3] = resultAlpha1;
    }
}

void AnimationManager::calcTransparencies(std::vector<float> &transparencies) {

    M2Array<M2TextureWeight> &transparencyRecords = boneMasterData->getM2Geom()->getM2Data()->texture_weights;
    auto &global_loops = *boneMasterData->getSkelData()->m_globalSequences;

    static float defaultAlpha = 1.0;
    for (int i = 0; i < transparencyRecords.size; i++) {
        float result1 = animateTrackWithBlend<fixed16, float>(
            animationInfo,
            transparencyRecords[i]->weight,
            global_loops,
            this->globalSequenceTimes,
            defaultAlpha
        );

        transparencies[i] = result1;
    }
}


void AnimationManager::calcLights(std::vector<M2LightResult> &lights, std::vector<mathfu::mat4> &bonesMatrices) {
    auto &lightRecords = boneMasterData->getM2Geom()->getM2Data()->lights;
    auto &global_loops = *boneMasterData->getSkelData()->m_globalSequences;

    if (lightRecords.size <= 0) return;

    static mathfu::vec3 defaultVector(1.0, 1.0, 1.0);
    static float defaultFloat = 1.0;

    for (int i = 0; i < lightRecords.size; i++) {
        M2Light * lightRecord = lightRecords.getElement(i);

        mathfu::vec4 ambient_color =
            mathfu::vec4(animateTrackWithBlend<C3Vector, mathfu::vec3>(
                animationInfo,
                lightRecord->ambient_color,
                global_loops,
                this->globalSequenceTimes,
                defaultVector
            ), 1.0);

        float ambient_intensity =
            animateTrackWithBlend<float, float>(
                animationInfo,
                lightRecord->ambient_intensity,
                global_loops,
                this->globalSequenceTimes,
                defaultFloat
            );
        mathfu::vec4 diffuse_color =
            mathfu::vec4(animateTrackWithBlend<C3Vector, mathfu::vec3>(
                animationInfo,
                lightRecord->diffuse_color,
                global_loops,
                this->globalSequenceTimes,
                defaultVector
            ), 1.0);


        switch (m_api->getConfig()->diffuseColorHack) {
            case 0:
                diffuse_color = mathfu::vec4(diffuse_color.x, diffuse_color.y, diffuse_color.z, diffuse_color.w);
                break;
            case 1:
                diffuse_color = mathfu::vec4(diffuse_color.x, diffuse_color.z, diffuse_color.y, diffuse_color.w);
                break;
            case 2:
                diffuse_color = mathfu::vec4(diffuse_color.y, diffuse_color.x, diffuse_color.z, diffuse_color.w);
                break;
            case 3:
                diffuse_color = mathfu::vec4(diffuse_color.y, diffuse_color.z, diffuse_color.x, diffuse_color.w);
                break;
            case 4:
                diffuse_color = mathfu::vec4(diffuse_color.z, diffuse_color.x, diffuse_color.y, diffuse_color.w);
                break;
            case 5:
                diffuse_color = mathfu::vec4(diffuse_color.z, diffuse_color.y, diffuse_color.x, diffuse_color.w);
                break;
        }

//        if (i == 1) {
//            diffuse_color = mathfu::vec4(diffuse_color.x, diffuse_color.z, diffuse_color.y, diffuse_color.w);
//        }
//        if (i == 0) {

//        }


        float diffuse_intensity =
            animateTrackWithBlend<float, float>(
                animationInfo,
                lightRecord->diffuse_intensity,
                global_loops,
                this->globalSequenceTimes,
                defaultFloat
            );

        float attenuation_start =
            animateTrackWithBlend<float, float>(
                animationInfo,
                lightRecord->attenuation_start,
                global_loops,
                this->globalSequenceTimes,
                defaultFloat
            );
        float attenuation_end =
                animateTrackWithBlend<float, float>(
                    animationInfo,
                    lightRecord->attenuation_end,
                    global_loops,
                    this->globalSequenceTimes,
                    defaultFloat
                );

        static unsigned char defaultChar = 0;
        unsigned char visibility =
                animateTrackWithBlend<unsigned char, unsigned char>(
                        animationInfo,
                        lightRecord->visibility,
                        global_loops,
                        this->globalSequenceTimes,
                        defaultChar
                );

        mathfu::mat4 &boneMat = bonesMatrices[lightRecord->bone];
        C3Vector &pos_vec = lightRecord->position;

        mathfu::vec4 lightWorldPos = boneMat * mathfu::vec4(mathfu::vec3(pos_vec), 1.0);
        lightWorldPos.w = 1.0;

//        if (i == 0) {
//            diffuse_intensity = 1.0;
//        }

        if (boneMasterData->getM2Geom()->getM2Data()->global_flags.flag_unk_0x8000 == 0) {
            attenuation_start =  1.6666f;
            attenuation_end = 5.2666602f;
        }

        lights[i].ambient_color = ambient_color;
        lights[i].ambient_intensity = ambient_intensity;
        lights[i].ambient_color = ambient_color * ambient_intensity;

        lights[i].diffuse_color = diffuse_color;
        lights[i].diffuse_intensity = diffuse_intensity;
        lights[i].attenuation_start = attenuation_start;
        lights[i].attenuation_end = attenuation_end;
        lights[i].position = lightWorldPos;
        lights[i].visibility = visibility;
    }
}

void AnimationManager::calcCamera(M2CameraResult &camera, int cameraId, mathfu::mat4 &placementMatrix) {
    int animationIndex = this->animationInfo.currentAnimation.animationIndex;
    animTime_t animationTime = this->animationInfo.currentAnimation.animationTime;

    auto &global_loops = *boneMasterData->getSkelData()->m_globalSequences;

    auto &cameraRecords = boneMasterData->getM2Geom()->getM2Data()->cameras;
    if (cameraRecords.size <= 0) return;
    static mathfu::vec3 defaultVector(0.0, 0.0, 0.0);
    static float defaultFloat = 1.0;

    auto animationRecord = animationInfo.currentAnimation.animationRecord;
    M2Camera * cameraRecord = cameraRecords.getElement(cameraId);

    mathfu::vec3 currentPosition =
        animateSplineTrack<C3Vector, mathfu::vec3>(
            animationTime,
            animationRecord->duration,
            animationIndex,
            cameraRecord->positions,
            global_loops,
            this->globalSequenceTimes,
            defaultVector
    );
    currentPosition = currentPosition + mathfu::vec3(cameraRecord->position_base);

    mathfu::vec3 currentTarget =
        animateSplineTrack<C3Vector, mathfu::vec3>(
            animationTime,
            animationRecord->duration,
            animationIndex,
            cameraRecord->target_position,
            global_loops,
            this->globalSequenceTimes,
            defaultVector
        );

    currentTarget = currentTarget + mathfu::vec3(cameraRecord->target_position_base);

    float fov =
        animateSplineTrack<float, float>(
            animationTime,
            animationRecord->duration,
            animationIndex,
            cameraRecord->FoV,
            global_loops,
            this->globalSequenceTimes,
            defaultFloat
        );

    float roll =
        animateSplineTrack<float, float>(
            animationTime,
            animationRecord->duration,
            animationIndex,
            cameraRecord->roll,
            global_loops,
            this->globalSequenceTimes,
            defaultFloat
        );

    //Write values
    camera.position = placementMatrix*mathfu::vec4(currentPosition, 1.0);
    camera.target_position = placementMatrix*mathfu::vec4(currentTarget, 1.0);
    camera.far_clip = cameraRecord->far_clip;
    camera.near_clip = cameraRecord->near_clip;
    camera.roll = roll;
    camera.diagFov = fov;
}

void AnimationManager::calcParticleEmitters(std::vector<ParticleEmitter *> &particleEmitters,
                                            std::vector<mathfu::mat4> &bonesMatrices) {
    auto &peRecords = boneMasterData->getM2Geom()->getM2Data()->particle_emitters;
    if (peRecords.size <= 0) return;

    auto &global_loops = *boneMasterData->getSkelData()->m_globalSequences;

    static mathfu::vec3 defaultVector(1.0, 1.0, 1.0);
    static float defaultFloat = 0.0;
    static unsigned char defaultChar = 1;

//    check_offset<offsetof(M2Particle, old.geometry_model_filename), 24>();
//    check_offset<offsetof(M2Particle, old.blendingType), 40>();
    check_offset<offsetof(M2Particle, old.textureDimensions_rows), 48>();
    check_size<M2ParticleOld, 476>();
    check_size<M2Particle, 492>();

    for (int i = 0; i < particleEmitters.size(); i++) {
        auto &peRecord = *peRecords.getElement(i);
        auto &particleEmitter = particleEmitters[i];
        if (particleEmitter->getGenerator() == nullptr) continue;
        CGeneratorAniProp *aniProp = particleEmitters[i]->getGenerator()->getAniProp();

        unsigned char enabledIn = 1;
        if (peRecord.old.enabledIn.timestamps.size > 0) {
            enabledIn = animateTrackWithBlend<unsigned char, unsigned char>(
                animationInfo,
                peRecord.old.enabledIn,
                global_loops,
                this->globalSequenceTimes,
                defaultChar
            );
        }

        particleEmitter->isEnabled = enabledIn;

        if (enabledIn) {
            aniProp->emissionSpeed =
                animateTrackWithBlend<float, float>(
                    animationInfo,
                    peRecord.old.emissionSpeed,
                    global_loops,
                    this->globalSequenceTimes,
                    defaultFloat
                );
            aniProp->speedVariation =
                animateTrackWithBlend<float, float>(
                    animationInfo,
                    peRecord.old.speedVariation,
                    global_loops,
                    this->globalSequenceTimes,
                    defaultFloat
                );
            aniProp->verticalRange =
                animateTrackWithBlend<float, float>(
                    animationInfo,
                    peRecord.old.verticalRange,
                    global_loops,
                    this->globalSequenceTimes,
                    defaultFloat
                );
            aniProp->horizontalRange =
                animateTrackWithBlend<float, float>(
                    animationInfo,
                    peRecord.old.horizontalRange,
                    global_loops,
                    this->globalSequenceTimes,
                    defaultFloat
                );
            if (peRecord.old.flags & 0x800000) {
                aniProp->gravity = animateTrackWithBlend<CompressedParticleGravity, mathfu::vec3>(
                    animationInfo,
                    peRecord.old.gravityCompr,
                    global_loops,
                    this->globalSequenceTimes,
                    defaultVector
                );
            } else {
                aniProp->gravity = mathfu::vec3(
                    0,
                    0,
                    -animateTrackWithBlend<float, float>(
                        animationInfo,
                        peRecord.old.gravity,
                        global_loops,
                        this->globalSequenceTimes,
                        defaultFloat
                    ));
            }


            aniProp->lifespan =
                animateTrackWithBlend<float, float>(
                    animationInfo,
                    peRecord.old.lifespan,
                    global_loops,
                    this->globalSequenceTimes,
                    defaultFloat
                );
            aniProp->emissionRate =
                animateTrackWithBlend<float, float>(
                    animationInfo,
                    peRecord.old.emissionRate,
                    global_loops,
                    this->globalSequenceTimes,
                    defaultFloat
                );
            aniProp->emissionAreaY =
                animateTrackWithBlend<float, float>(
                    animationInfo,
                    peRecord.old.emissionAreaWidth,
                    global_loops,
                    this->globalSequenceTimes,
                    defaultFloat
                );
            aniProp->emissionAreaX =
                animateTrackWithBlend<float, float>(
                    animationInfo,
                    peRecord.old.emissionAreaLength,
                    global_loops,
                    this->globalSequenceTimes,
                    defaultFloat
                );
            if (!m_hasExp2) {
                aniProp->zSource =
                    animateTrackWithBlend<float, float>(
                        animationInfo,
                        peRecord.old.zSource,
                        global_loops,
                        this->globalSequenceTimes,
                        defaultFloat
                    );
            }
        }
    }

}

void AnimationManager::calcRibbonEmitters(std::vector<CRibbonEmitter *> &ribbonEmitters){
    auto &ribbonRecords = boneMasterData->getM2Geom()->getM2Data()->ribbon_emitters;
    if (ribbonRecords.size <= 0) return;

    auto &global_loops = *boneMasterData->getSkelData()->m_globalSequences;

    static mathfu::vec3 defaultVector(1.0, 1.0, 1.0);
    static mathfu::vec4 defaultVector4(1.0, 1.0, 1.0, 1.0);
    static float defaultFloat = 1.0;
    static unsigned char defaultChar = 1;
    static unsigned short defaultInt = 0;

    for (int i = 0; i < ribbonEmitters.size(); i++) {
        auto *ribbonRecord = ribbonRecords.getElement(i);
        auto &ribbonEmitter = ribbonEmitters[i];

        mathfu::vec4 colorRGBA =
            animateTrackWithBlend<C3Vector, mathfu::vec4>(
                animationInfo,
                ribbonRecord->colorTrack,
                global_loops,
                this->globalSequenceTimes,
                defaultVector4
            );
        ribbonEmitter->SetColor(colorRGBA.x, colorRGBA.y, colorRGBA.z);

        colorRGBA.w =
            animateTrackWithBlend<fixed16, float>(
                animationInfo,
                ribbonRecord->alphaTrack,
                global_loops,
                this->globalSequenceTimes,
                defaultFloat
            );

        ribbonEmitter->SetAlpha(colorRGBA.w);

        float above =
            animateTrackWithBlend<float, float>(
                animationInfo,
                ribbonRecord->heightAboveTrack,
                global_loops,
                this->globalSequenceTimes,
                defaultFloat
            );
        ribbonEmitter->SetAbove(above);

        float below =
            animateTrackWithBlend<float, float>(
                animationInfo,
                ribbonRecord->heightBelowTrack,
                global_loops,
                this->globalSequenceTimes,
                defaultFloat
            );
        ribbonEmitter->SetBelow(below);

        unsigned short texSlot =
            animateTrackWithBlend<uint16_t, unsigned short>(
            animationInfo,
            ribbonRecord->texSlotTrack,
            global_loops,
            this->globalSequenceTimes,
            defaultInt
        );

        ribbonEmitter->SetTexSlot(texSlot);

        unsigned char dataEnabled =
            animateTrackWithBlend<unsigned char, unsigned char>(
                animationInfo,
                ribbonRecord->visibilityTrack,
                global_loops,
                this->globalSequenceTimes,
                defaultChar
            );
        ribbonEmitter->SetDataEnabled(dataEnabled);

    }
}

void AnimationManager::setAnimationPercent(float percent) {
    if (animationInfo.currentAnimation.animationRecord != nullptr) {
        animationInfo.currentAnimation.animationTime =
            animationInfo.currentAnimation.animationRecord->duration  * percent;
    }
}

void AnimationManager::calcAnimRepetition(AnimationStruct &animationStruct) {
    auto &seqRec = animationStruct.animationRecord;
    animationStruct.repeatTimes =
        seqRec->replay.min + (
            (seqRec->replay.max - seqRec->replay.min) * ((float)std::rand() / (float)RAND_MAX)
        ) - 1;
}

int AnimationManager::getCurrentAnimationIndex() {
    return this->animationInfo.currentAnimation.animationIndex;
}
