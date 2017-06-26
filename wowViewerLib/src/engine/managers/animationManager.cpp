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

}

void AnimationManager::initBlendMatrices() {

}

void AnimationManager::initGlobalSequenceTimes() {

}

void AnimationManager::calculateBoneTree() {

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

void blendMatrices(mathfu::mat4 *origMat, mathfu::mat4 *blendMat, int count, float blendAlpha) {
//Actual blend
    for (int i = 0; i < count; i++) {
        mathfu::mat4 &blendTransformMatrix = blendMat[i];
        mathfu::mat4 &tranformMat = origMat[i];
        tranformMat = (blendTransformMatrix - tranformMat) * (1.0 - blendAlpha) + tranformMat;
    }
}

void AnimationManager::update(int deltaTime, mathfu::vec3 cameraPosInLocal, std::vector<mathfu::mat4> &bonesMatrices,
                              std::vector<mathfu::mat4> &textAnimMatrices,
                              std::vector<mathfu::vec3> &subMeshColors,
                              std::vector<float> transparencies
        /*cameraDetails, lights, particleEmitters*/) {

    M2Sequence* mainAnimationRecord = m_m2File->sequences[this->mainAnimationIndex];
    M2Sequence* currentAnimationRecord = m_m2File->sequences[this->currentAnimationIndex];

    this->currentAnimationTime += deltaTime;
    //Update global sequences
    for (var i = 0; i < this->globalSequenceTimes.length; i++) {
        if (m2File.globalSequences[i] > 0) { // Global sequence values can be 0's
            this->globalSequenceTimes[i] += deltaTime;
            this->globalSequenceTimes[i] = this->globalSequenceTimes[i] % m2File.globalSequences[i];
        }
    }

    /* Pick next animation if there is one and no next animation was picked before */
    M2Sequence* subAnimRecord = nullptr;
    if (this->nextSubAnimationIndex < 0 && mainAnimationRecord->variationNext > -1) {
        //if (currentAnimationPlayedTimes)
        var probability = floor(Math.random() * (0x7fff + 1));
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
        this->blendMatrices(textAnimMatrices, this->blendMatrixArray, m2File.nTexAnims, blendAlpha);
    }

    for (var i = 0; i < m2File.nBones; i++) {
        this->bonesIsCalculated[i] = false;
    }
    this->calcBones(bonesMatrices, this->currentAnimationIndex, this->currentAnimationTime, cameraPosInLocal);
    if (blendAnimationIndex > -1) {
        for (var i = 0; i < m2File.nBones; i++) {
            this->bonesIsCalculated[i] = false;
            mat4.identity(this->blendMatrixArray[i]);
        }

        this->calcBones(this->blendMatrixArray, blendAnimationIndex, this->nextSubAnimationTime, cameraPosInLocal);
        this->blendMatrices(bonesMatrices, this->blendMatrixArray, m2File.nBones, blendAlpha)
    }

    this->calcSubMeshColors(subMeshColors, this->currentAnimationIndex, this->currentAnimationTime, blendAnimationIndex,
                           this->nextSubAnimationTime, blendAlpha);
    this->calcTransparencies(transparencies, this->currentAnimationIndex, this->currentAnimationTime, blendAnimationIndex,
                            this->nextSubAnimationTime, blendAlpha);

    this->calcCameras(cameraDetails, this->currentAnimationIndex, this->currentAnimationTime);
    this->calcLights(lights, bonesMatrices, this->currentAnimationIndex, this->currentAnimationTime);
    this->calcParticleEmitters(particleEmitters, this->currentAnimationIndex, this->currentAnimationTime);
}

