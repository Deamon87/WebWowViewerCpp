//
// Created by deamon on 26.06.17.
//

#ifndef WEBWOWVIEWERCPP_ANIMATIONMANAGER_H
#define WEBWOWVIEWERCPP_ANIMATIONMANAGER_H


#include <vector>
#include "../persistance/header/M2FileHeader.h"

class AnimationManager {
private:
    M2Data *m_m2File;
    int mainAnimationId;
    int mainAnimationIndex;

    int currentAnimationIndex;
    double currentAnimationTime;
    int currentAnimationPlayedTimes;

    int nextSubAnimationIndex;
    double nextSubAnimationTime;
    bool nextSubAnimationActive;

    bool firstCalc;
    bool isAnimated;

    int leftHandClosed = 0;
    int rightHandClosed = 0;

    std::vector<double> globalSequenceTimes;
    std::vector<bool> bonesIsCalculated;
    std::vector<mathfu::mat4> blendMatrixArray;
    std::vector<std::vector<int>> childBonesLookup;

    void initBonesIsCalc();
    void initBlendMatrices();
    void initGlobalSequenceTimes();

    void calculateBoneTree();
    void calcAnimMatrixes (std::vector<mathfu::mat4> &textAnimMatrices, int animationIndex, double time);


public:
    AnimationManager(M2Data* m2File);
    bool setAnimationId(int animationId, bool reset);
    void update (double deltaTime, mathfu::vec3 cameraPosInLocal, std::vector<mathfu::mat4> &bonesMatrices,
                 std::vector<mathfu::mat4> &textAnimMatrices,
                 std::vector<mathfu::vec4> &subMeshColors,
                 std::vector<float> &transparencies
            /*cameraDetails, lights, particleEmitters*/);

    void calcBones(std::vector<mathfu::mat4> &boneMatrices, int animation, double time, mathfu::vec3 &cameraPosInLocal);

    void calcBoneMatrix(std::vector<mathfu::mat4> &boneMatrices, int boneIndex, int animationIndex, double time,
                        mathfu::vec3 cameraPosInLocal);

    void calcChildBones(std::vector<mathfu::mat4> &boneMatrices, int boneIndex, int animationIndex, double time,
                        mathfu::vec3 cameraPosInLocal);

    void calcSubMeshColors(std::vector<mathfu::vec4> &subMeshColors,
            int animationIndex,
            double time,
            int blendAnimationIndex,
            double blendAnimationTime,
            double blendAlpha);

    void calcTransparencies(std::vector<float> &transparencies, int animationIndex, double time, int blendAnimationIndex,
                            double blendAnimationTime, double blendAlpha);
};


#endif //WEBWOWVIEWERCPP_ANIMATIONMANAGER_H
