//
// Created by deamon on 26.06.17.
//

#ifndef WEBWOWVIEWERCPP_ANIMATIONMANAGER_H
#define WEBWOWVIEWERCPP_ANIMATIONMANAGER_H


#include "../persistance/M2File.h"

class AnimationManager {
private:
    M2Data *m_m2File;
    int mainAnimationId;
    int mainAnimationIndex;

    int currentAnimationIndex;
    int currentAnimationTime;
    int currentAnimationPlayedTimes;

    int nextSubAnimationIndex;
    int nextSubAnimationTime;
    bool nextSubAnimationActive;

    bool firstCalc;

    void initBonesIsCalc();
    void initBlendMatrices();
    void initGlobalSequenceTimes();
    void calculateBoneTree();
public:
    AnimationManager(M2Data* m2File);
    bool setAnimationId(int animationId, bool reset);
    void update (int deltaTime, mathfu::vec3 cameraPosInLocal, std::vector<mathfu::mat4> &bonesMatrices,
                 std::vector<mathfu::mat4> &textAnimMatrices,
                 std::vector<mathfu::vec3> &subMeshColors,
                 std::vector<float> transparencies
            /*cameraDetails, lights, particleEmitters*/);
};


#endif //WEBWOWVIEWERCPP_ANIMATIONMANAGER_H
