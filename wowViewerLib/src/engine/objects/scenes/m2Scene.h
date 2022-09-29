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

    void getPotentialEntities(const MathHelper::FrustumCullingData &frustumData,
                              const mathfu::vec4 &cameraPos,
                              HCullStage &cullStage,
                              M2ObjectListContainer &potentialM2,
                              WMOListContainer &potentialWmo) override;

    void getCandidatesEntities(const MathHelper::FrustumCullingData &frustumData,
                               const mathfu::vec4 &cameraPos,
                               HCullStage &cullStage,
                               M2ObjectListContainer &m2ObjectsCandidates,
                               WMOListContainer &wmoCandidates) override;

    void updateLightAndSkyboxData(const HCullStage &cullStage, mathfu::vec3 &cameraVec3,
                             StateForConditions &stateForConditions, const AreaRecord &areaRecord) override;

public:
    explicit M2Scene(HApiContainer api, std::string m2Model, int cameraView = - 1);
    explicit M2Scene(HApiContainer api, int fileDataId, int cameraView = - 1);


    ~M2Scene() override {

    }

    void setAnimationId(int animationId) override {
        m_m2Object->setAnimationId(animationId);
    };
    void setMeshIds(std::vector<uint8_t> &meshIds) override {
        m_m2Object->setMeshIds(meshIds);
    };
    void resetAnimation() override {
        m_m2Object->resetCurrentAnimation();
    }

    void setReplaceTextureArray(std::vector<int> &replaceTextureArray) override;
    void setMeshIdArray(std::vector<uint8_t> &meshIds) override ;
    void setReplaceParticleColors(std::array<std::array<mathfu::vec4, 3>, 3> &particleColorReplacement) override;
    void resetReplaceParticleColor() override;
    void doPostLoad(HCullStage cullStage) override;

    int getCameraNum() override ;
    std::shared_ptr<ICamera> createCamera(int cameraNum) override;

    void exportScene(IExporter* exporter) override;
};


#endif //WEBWOWVIEWERCPP_M2SCENE_H
