//
// Created by deamon on 08.09.17.
//

#ifndef WEBWOWVIEWERCPP_M2SCENE_H
#define WEBWOWVIEWERCPP_M2SCENE_H


#include "map.h"
#include "../m2/m2Object.h"

class M2Scene : public Map {
private:
    std::string m_m2Model;
    std::shared_ptr<M2Object> m_m2Object = nullptr;

    void getPotentialEntities(const mathfu::vec4 &cameraPos, std::vector<std::shared_ptr<M2Object>> &potentialM2,
                                      HCullStage &cullStage, mathfu::mat4 &lookAtMat4, mathfu::vec4 &camera4,
                                      std::vector<mathfu::vec4> &frustumPlanes, std::vector<mathfu::vec3> &frustumPoints,
                                      std::vector<std::shared_ptr<WmoObject>> &potentialWmo) override;

    void getCandidatesEntities(std::vector<mathfu::vec3> &hullLines, mathfu::mat4 &lookAtMat4, mathfu::vec4 &cameraPos,
                                       std::vector<mathfu::vec3> &frustumPoints, HCullStage &cullStage,
                                       std::vector<std::shared_ptr<M2Object>> &m2ObjectsCandidates,
                                       std::vector<std::shared_ptr<WmoObject>> &wmoCandidates) override;

    void updateLightAndSkyboxData(const HCullStage &cullStage, mathfu::vec3 &cameraVec3,
                             StateForConditions &stateForConditions, const AreaRecord &areaRecord) override;

public:
    explicit M2Scene(ApiContainer *api, std::string m2Model, int cameraView = - 1) {
        m_api = api; m_m2Model = m2Model; m_cameraView = cameraView;
        m_sceneMode = SceneMode::smM2;
        m_suppressDrawingSky = true;

        auto  m2Object = std::make_shared<M2Object>(m_api);
        m2Object->setLoadParams(0, {}, {});
        m2Object->setModelFileName(m_m2Model);
        m2Object->createPlacementMatrix(mathfu::vec3(0,0,0), 0, mathfu::vec3(1,1,1), nullptr);

        m2Object->calcWorldPosition();

        m_m2Object = m2Object;
    }
    explicit M2Scene(ApiContainer *api, int fileDataId, int cameraView = - 1) {
        m_api = api; m_cameraView = cameraView;
        m_sceneMode = SceneMode::smM2;
        m_suppressDrawingSky = true;

        auto m2Object = std::make_shared<M2Object>(m_api);
        m2Object->setLoadParams(0, {}, {});
        m2Object->setModelFileId(fileDataId);
        m2Object->createPlacementMatrix(mathfu::vec3(0,0,0), 0, mathfu::vec3(1,1,1), nullptr);
        m2Object->calcWorldPosition();

        m_m2Object = m2Object;
    };


    ~M2Scene() override {

    }

    void setAnimationId(int animationId) override {
        m_m2Object->setAnimationId(animationId);
    };
    void resetAnimation() override {
        m_m2Object->resetCurrentAnimation();
    }

    void setReplaceTextureArray(std::vector<int> &replaceTextureArray) override;
    void doPostLoad(HCullStage cullStage) override;

    int getCameraNum() override ;
    std::shared_ptr<ICamera> createCamera(int cameraNum);
};


#endif //WEBWOWVIEWERCPP_M2SCENE_H
