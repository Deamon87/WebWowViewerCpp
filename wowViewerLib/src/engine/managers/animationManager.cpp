//
// Created by deamon on 26.06.17.
//

#include "animationManager.h"

AnimationManager::AnimationManager(M2Data* m2File) {
    this->m_m2File = m2File;

    this->mainAnimationId = 0;
    this->mainAnimationIndex = 0;

    this->currentAnimationIndex = 0;
    this->currentAnimationTime = 0;
    this->currentAnimationPlayedTimes = 0;

    this->nextSubAnimationIndex = -1;
    this->nextSubAnimationTime = 0;

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
    bonesIsCalculated = std::vector<bool>((unsigned long) m_m2File->bones.size);

    for (int i = 0; i <  m_m2File->bones.size; i++) {
        bonesIsCalculated[i] = false;
    }
}

void AnimationManager::initBlendMatrices() {
    unsigned long matCount = (unsigned long) std::max(m_m2File->bones.size, m_m2File->texture_transforms.size);
    blendMatrixArray = std::vector<mathfu::mat4>(matCount, mathfu::mat4::Identity());
}

void AnimationManager::initGlobalSequenceTimes() {
    globalSequenceTimes = std::vector<int>(
            (unsigned long) (m_m2File->global_loops.size > 0 ? m_m2File->global_loops.size : 0));

    for (int i = 0; i < globalSequenceTimes.size(); i++) {
        globalSequenceTimes[i] = 0;
    }
}

void AnimationManager::calculateBoneTree() {
    this->childBonesLookup = std::vector<std::vector<int>>(m_m2File->bones.size);
    for (int i = 0; i < m_m2File->bones.size; i++) {
        for (int j = 0; j < m_m2File->bones.size; j++) {
            if (m_m2File->bones[j]->parent_bone == i) {
                childBonesLookup[i].push_back(j);
            }
        }
    }
}

bool AnimationManager::setAnimationId(int animationId, bool reset) {
    int animationIndex = -1;
    if ((m_m2File->sequence_lookups.size == 0) && (m_m2File->sequences.size > 0)) {
        for (int i = 0; i < m_m2File->sequences.size; i++) {
            M2Sequence* animationRecord = m_m2File->sequences[i];
            if (animationRecord->id == animationId) {
                animationIndex = i;
                break;
            }
        }
    } else if (animationId < m_m2File->sequence_lookups.size) {
        animationIndex = *m_m2File->sequence_lookups[animationId];
    }
    if ((animationIndex > - 1)&& (reset || (animationIndex != this->mainAnimationIndex) )) {
        //Reset animation
        this->mainAnimationId = animationId;
        this->mainAnimationIndex = animationIndex;

        this->currentAnimationIndex = animationIndex;
        this->currentAnimationTime = 0;
        this->currentAnimationPlayedTimes = 0;

        this->nextSubAnimationIndex = -1;
        this->nextSubAnimationTime = 0;
        this->nextSubAnimationActive = false;

        this->firstCalc = true; //TODO: reset this on going to next subAnimation too
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

template<typename T>
mathfu::vec4 animateTrack(int timeIndex, M2Track<T> &animationBlock) {
    static_assert(false, "Called empty animateTrack");
}

template<>
mathfu::vec4 animateTrack<C3Vector>(int timeIndex, M2Track<C3Vector> &animationBlock) {
    animationBlock->

        var value1 = values[i - 1];
        var value2 = values[i];

        var time1 = times[i - 1];
        var time2 = times[i];

        value1 = convertValueTypeToVec4(value1, value_type);
        value2 = convertValueTypeToVec4(value2, value_type);

        result = this.interpolateValues(animTime,
                                        interpolType, time1, time2, value1, value2, value_type);
}

template<typename T>
mathfu::vec4 getTimedValue(unsigned int currTime,
            int maxTime,
            int animationIndex,
            M2Track<T> &animationBlock,
            M2Array<M2Loop> &global_loops,
            std::vector<int> &globalSequenceTimes) {
    int16_t globalSequence = animationBlock.global_sequence;
    uint16_t interpolType = animationBlock.interpolation_type;

    if (animationBlock.timestamps.size == 0) {
        return mathfu::vec4(0,0,0,0);
    }

    auto times = animationBlock.timestamps[animationIndex];
    auto values = animationBlock.values[animationIndex];

    //Hack
    if (times == nullptr) {
        animationIndex = 0;
        times = animationBlock.timestamps[animationIndex];
        values = animationBlock.values[animationIndex];
    }
    if (!times || times->size == 0) {
        return mathfu::vec4(0,0,0,0);
    }

    if (globalSequence >=0) {
        currTime = globalSequenceTimes[globalSequence];
        maxTime = global_loops[globalSequence]->timestamp;
    }

    int32_t times_len = times->size;
    mathfu::vec4 result;
    if (times_len > 1) {
        //var maxTime = times[times_len-1];
        unsigned int animTime = currTime % maxTime;

           if ((animTime > *times->getElement(times_len-1)) && (animTime <= maxTime)) {
//            result = convertValueTypeToVec4(values[0], value_type);
        } else {
//            result =  convertValueTypeToVec4(times[times_len-1], value_type);
            for (int i = 0; i < times_len; i++) {
                if (*times->getElement(i) > animTime) {
                    result = animateTrack(i, animationBlock);
                }
            }
        }
    } else {
//        result = convertValueTypeToVec4(values[0], value_type);
    }

    return result;
}

template<typename T>
inline void calcAnimationTransform(
        mathfu::mat4 &tranformMat,
        mathfu::mat4 *billboardMatrix,
        mathfu::vec4 &pivotPoint,
        mathfu::vec4 &negatePivotPoint,
        std::vector<int> &globalSequenceTimes,
        bool &isAnimated,
        T *animationData,
        M2Data * m2Data,
        int animationIndex,
        int time
        ) {
    M2Sequence *animationRecord = m2Data->sequences[animationIndex];
    tranformMat = tranformMat * mathfu::mat4::FromTranslationVector(pivotPoint.xyz());

    if (animationData->translation.values.size > 0) {
        mathfu::vec4 transVec = getTimedValue(
                time,
                animationRecord->duration,
                animationIndex,
                animationData->translation,
                m2Data->global_loops,
                globalSequenceTimes);

        tranformMat = tranformMat * mathfu::mat4::FromTranslationVector(transVec.xyz());
        isAnimated = true;
    }
    if (billboardMatrix != nullptr) {
        tranformMat = tranformMat * *billboardMatrix;
    } else if (animationData->rotation.values.size > 0) {
        mathfu::vec4 quaternionResult = getTimedValue(
            time,
            animationRecord->duration,
            animationIndex,
            animationData->rotation,
            m2Data->global_loops,
            globalSequenceTimes);


        mathfu::Quaternion<float> quat(quaternionResult.w, quaternionResult.x, quaternionResult.y, quaternionResult.z);
        tranformMat = tranformMat * quat.ToMatrix4();
        isAnimated = true;
    }

    if (animationData->scaling.values.size > 0) {

        mathfu::vec4 scaleResult = getTimedValue(
                time,
                animationRecord->duration,
                animationIndex,
                animationData->scaling,
                m2Data->global_loops,
                globalSequenceTimes);

        tranformMat = tranformMat * mathfu::mat4::FromScaleVector(scaleResult.xyz());
        isAnimated = true;
    }
    tranformMat = tranformMat * mathfu::mat4::FromTranslationVector(negatePivotPoint.xyz());
}


void AnimationManager::calcAnimMatrixes (std::vector<mathfu::mat4> textAnimMatrices, int animationIndex, int time) {

    mathfu::vec4 pivotPoint(0.5, 0.5, 0, 0);
    mathfu::vec4 negatePivotPoint = -pivotPoint;

    M2Sequence *animationRecord = m_m2File->sequences[animationIndex];
    for (int i = 0; i < m_m2File->texture_transforms.size; i++) {
        M2TextureTransform *textAnimData = m_m2File->texture_transforms[i];

        textAnimMatrices[i] = mathfu::mat4::Identity();
        bool isAnimated;
        calcAnimationTransform<M2TextureTransform>(textAnimMatrices[i],
                                                   nullptr,
                                                   pivotPoint, negatePivotPoint,
                                                   this->globalSequenceTimes,
                                                   isAnimated,
                                                   textAnimData,
                                                   m_m2File,
                                                   animationIndex, time
        );
        this->isAnimated = isAnimated ? isAnimated : this->isAnimated;
    }
}

void calcBoneBillboardMatrix(
                mathfu::mat4 *billboardMatrix,
                std::vector<mathfu::mat4> &boneMatrices,
                M2CompBone *boneDefinition,
                int parentBone,
                mathfu::vec4 pivotPoint,
                mathfu::vec3 cameraPosInLocal) {

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

    if ((boneDefinition->flags & 0x40) > 0) {
        //Cylindrical billboard
        modelUp = mathfu::vec3(0, 0, 1);

        modelRight = mathfu::vec3::CrossProduct(modelUp, modelForward).Normalized();
        modelForward = mathfu::vec3::CrossProduct(modelRight, modelUp).Normalized();

        modelRight = mathfu::vec3::CrossProduct(modelUp, modelForward).Normalized();
    } else {
        //Spherical billboard
        modelRight = mathfu::vec3::CrossProduct(mathfu::vec3(0, 0, 1), modelForward).Normalized();
        modelUp = mathfu::vec3::CrossProduct(modelForward, modelRight).Normalized();
    }


    *billboardMatrix = mathfu::mat4(
            modelForward[0], modelForward[1], modelForward[2], 0,
            modelRight[0], modelRight[1], modelRight[2], 0,
            modelUp[0], modelUp[1], modelUp[2], 0,
            0, 0, 0, 1
    );
}

void
AnimationManager::calcBoneMatrix(std::vector<mathfu::mat4> boneMatrices, int boneIndex, int animationIndex, int time,
                                 mathfu::vec3 cameraPosInLocal) {
    if (this->bonesIsCalculated[boneIndex]) return;

    M2Sequence *animationRecord = m_m2File->sequences[animationIndex];
    M2CompBone *boneDefinition = m_m2File->bones[boneIndex];

    int parentBone = boneDefinition->parent_bone;

    /* 2. Prepare bone part of animation process */
    boneMatrices[boneIndex] = mathfu::mat4::Identity();


    if (parentBone >= 0) {
        this->calcBoneMatrix(boneMatrices, parentBone, animationIndex, time, cameraPosInLocal);
        boneMatrices[boneIndex] = boneMatrices[boneIndex] * boneMatrices[parentBone];
    }

    if ((boneDefinition->flags & 0x278) == 0) {
        this->bonesIsCalculated[boneIndex] = true;
        return;
    }

    C3Vector pP = boneDefinition->pivot;
    mathfu::vec4 pivotPoint = mathfu::vec4(pP.x, pP.y, pP.z, 0);
    mathfu::vec4 negatePivotPoint = -mathfu::vec4(pP.x, pP.y, pP.z, 0);


    /* 2.1 Calculate billboard matrix if needed */
    mathfu::mat4 *billboardMatrix = nullptr;

    if (((boneDefinition->flags & 0x8) > 0) || ((boneDefinition->flags & 0x40) > 0)) {
        //From http://gamedev.stackexchange.com/questions/112270/calculating-rotation-matrix-for-an-object-relative-to-a-planets-surface-in-monog
        billboardMatrix = new mathfu::mat4();

        calcBoneBillboardMatrix(billboardMatrix, boneMatrices, boneDefinition, parentBone, pivotPoint,
                                                       cameraPosInLocal);
        this->isAnimated = true;
    }

    /* 3. Calculate matrix */
    calcAnimationTransform<M2CompBone>(boneMatrices[boneIndex],
           billboardMatrix,
           pivotPoint, negatePivotPoint,
           this->globalSequenceTimes,
           isAnimated,
           boneDefinition,
           m_m2File,
           animationIndex, time);

    if (billboardMatrix != nullptr) {
        delete billboardMatrix;
    }

    this->bonesIsCalculated[boneIndex] = true;
}

void AnimationManager::calcChildBones(std::vector<mathfu::mat4> &boneMatrices, int boneIndex,
                                      int animationIndex, int time, mathfu::vec3 cameraPosInLocal) {
    std::vector<int> *childBones = &this->childBonesLookup[boneIndex];
    for (int i = 0; i < childBones->size(); i++) {
        int childBoneIndex = (*childBones)[i];
        this->bonesIsCalculated[childBoneIndex] = false;
        this->calcBoneMatrix(boneMatrices, childBoneIndex, animationIndex, time, cameraPosInLocal);
        this->calcChildBones(boneMatrices, childBoneIndex, animationIndex, time, cameraPosInLocal);
    }
}

void AnimationManager::calcBones (std::vector<mathfu::mat4> &boneMatrices, int animation, int time, mathfu::vec3 &cameraPosInLocal) {


    if (this->firstCalc || this->isAnimated) {
        //Animate everything with standard animation
        for (int i = 0; i < m_m2File->bones.size; i++) {
            this->calcBoneMatrix(boneMatrices, i, animation, time, cameraPosInLocal);
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

        int closedHandAnimation = -1;
        if (m_m2File->sequence_lookups.size > 15 && m_m2File->sequence_lookups[15] > 0) { //ANIMATION_HANDSCLOSED = 15
            closedHandAnimation = *m_m2File->sequence_lookups[15];
        }

        if (closedHandAnimation >= 0) {
            if (this->leftHandClosed) {
                for (int j = 0; j < 5; j++) {
                    if (*m_m2File->key_bone_lookup[13 + j] > -1) { // BONE_LFINGER1 = 13
                        int boneId = *m_m2File->key_bone_lookup[13 + j];
                        this->bonesIsCalculated[boneId] = false;
                        this->calcBoneMatrix(boneMatrices, boneId, closedHandAnimation, 1, cameraPosInLocal);
                        this->calcChildBones(boneMatrices, boneId, closedHandAnimation, 1, cameraPosInLocal);
                    }
                }
            }
            if (this->rightHandClosed) {
                for (int j = 0; j < 5; j++) {
                    if (*m_m2File->key_bone_lookup[8 + j] > -1) { // BONE_RFINGER1 = 8
                        int boneId = *m_m2File->key_bone_lookup[8 + j];
                        this->bonesIsCalculated[boneId] = false;
                        this->calcBoneMatrix(boneMatrices, boneId, closedHandAnimation, 1, cameraPosInLocal);
                        this->calcChildBones(boneMatrices, boneId, closedHandAnimation, 1, cameraPosInLocal);
                    }
                }
            }
        }
    }
    this->firstCalc = false;
}

void AnimationManager::update(int deltaTime, mathfu::vec3 cameraPosInLocal, std::vector<mathfu::mat4> &bonesMatrices,
                              std::vector<mathfu::mat4> &textAnimMatrices,
                              std::vector<mathfu::vec4> &subMeshColors,
                              std::vector<float> transparencies
        /*cameraDetails, lights, particleEmitters*/) {

    M2Sequence* mainAnimationRecord = m_m2File->sequences[this->mainAnimationIndex];
    M2Sequence* currentAnimationRecord = m_m2File->sequences[this->currentAnimationIndex];

    this->currentAnimationTime += deltaTime;
    //Update global sequences
    for (int i = 0; i < this->globalSequenceTimes.size(); i++) {
        if (m_m2File->global_loops[i] > 0) { // Global sequence values can be 0's
            this->globalSequenceTimes[i] += deltaTime;
            this->globalSequenceTimes[i] = this->globalSequenceTimes[i] % m_m2File->global_loops[i]->timestamp;
        }
    }

    /* Pick next animation if there is one and no next animation was picked before */
    M2Sequence* subAnimRecord = nullptr;
    if (this->nextSubAnimationIndex < 0 && mainAnimationRecord->variationNext > -1) {
        //if (currentAnimationPlayedTimes)
        int probability = 3;//floor(Math.random() * (0x7fff + 1));
        float calcProb = 0;

        /* First iteration is out of loop */
        auto currentSubAnimIndex = this->mainAnimationIndex;
        subAnimRecord = m_m2File->sequences[currentSubAnimIndex];
        calcProb += subAnimRecord->frequency;
        while ((calcProb < probability) && (subAnimRecord->variationNext > -1)) {
            currentSubAnimIndex = subAnimRecord->variationNext;
            subAnimRecord = m_m2File->sequences[currentSubAnimIndex];

            calcProb += subAnimRecord->frequency;
        }

        this->nextSubAnimationIndex = currentSubAnimIndex;
        this->nextSubAnimationTime = 0;
    }

    int currAnimLeft = currentAnimationRecord->duration - this->currentAnimationTime;

    /*if (this->nextSubAnimationActive) {
        this->nextSubAnimationTime += deltaTime;
    }
    */

    int subAnimBlendTime = 0;
    float blendAlpha = 1.0;
    if (this->nextSubAnimationIndex > -1) {
        subAnimRecord = m_m2File->sequences[this->nextSubAnimationIndex];
        subAnimBlendTime = subAnimRecord->blendtime;
    }

    int blendAnimationIndex = -1;
    if ((subAnimBlendTime > 0) && (currAnimLeft < subAnimBlendTime)) {
        this->firstCalc = true;
        this->nextSubAnimationTime = (subAnimBlendTime - currAnimLeft) % subAnimRecord->duration;
        blendAlpha = currAnimLeft / subAnimBlendTime;
        blendAnimationIndex = this->nextSubAnimationIndex;
    }

    if (this->currentAnimationTime >= currentAnimationRecord->duration) {
        if (this->nextSubAnimationIndex > -1) {
            this->currentAnimationIndex = this->nextSubAnimationIndex;
            this->currentAnimationTime = this->nextSubAnimationTime;

            this->firstCalc = true;

            this->nextSubAnimationIndex = -1;
            this->nextSubAnimationActive = false;
        } else {
            this->currentAnimationTime = this->currentAnimationTime % currentAnimationRecord->duration;
        }
    }


    /* Update animated values */

    this->calcAnimMatrixes(textAnimMatrices, this->currentAnimationIndex, this->currentAnimationTime);
    if (blendAnimationIndex > -1) {
        this->calcAnimMatrixes(this->blendMatrixArray, blendAnimationIndex, this->nextSubAnimationTime);
        blendMatrices(textAnimMatrices, this->blendMatrixArray, m_m2File->texture_transforms.size, blendAlpha);
    }

    for (int i = 0; i < m_m2File->bones.size; i++) {
        this->bonesIsCalculated[i] = false;
    }
    this->calcBones(bonesMatrices, this->currentAnimationIndex, this->currentAnimationTime, cameraPosInLocal);
    if (blendAnimationIndex > -1) {
        for (int i = 0; i < m_m2File->bones.size; i++) {
            this->bonesIsCalculated[i] = false;
            this->blendMatrixArray[i] = mathfu::mat4::Identity();
        }

        this->calcBones(this->blendMatrixArray, blendAnimationIndex, this->nextSubAnimationTime, cameraPosInLocal);
        blendMatrices(bonesMatrices, this->blendMatrixArray, m_m2File->bones.size, blendAlpha);
    }

//    this->calcSubMeshColors(subMeshColors, this->currentAnimationIndex, this->currentAnimationTime, blendAnimationIndex,
//                           this->nextSubAnimationTime, blendAlpha);
//    this->calcTransparencies(transparencies, this->currentAnimationIndex, this->currentAnimationTime, blendAnimationIndex,
//                            this->nextSubAnimationTime, blendAlpha);

//    this->calcCameras(cameraDetails, this->currentAnimationIndex, this->currentAnimationTime);
//    this->calcLights(lights, bonesMatrices, this->currentAnimationIndex, this->currentAnimationTime);
//    this->calcParticleEmitters(particleEmitters, this->currentAnimationIndex, this->currentAnimationTime);
}

