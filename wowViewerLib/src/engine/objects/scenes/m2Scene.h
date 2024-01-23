//
// Created by deamon on 08.09.17.
//

#ifndef WEBWOWVIEWERCPP_M2SCENE_H
#define WEBWOWVIEWERCPP_M2SCENE_H


#include "map.h"
#include "../m2/m2Object.h"
#include "../../../exporters/IExporter.h"

class M2Scene : public Map {
private:
    std::string m_m2Model;
    std::shared_ptr<M2Object> m_m2Object = nullptr;

    void getPotentialEntities(const MathHelper::FrustumCullingData &frustumData,
                              const mathfu::vec4 &cameraPos,
                              const HMapRenderPlan &mapRenderPlan,
                              M2ObjectListContainer &potentialM2,
                              WMOListContainer &potentialWmo) override;

    void getCandidatesEntities(const MathHelper::FrustumCullingData &frustumData,
                               const mathfu::vec4 &cameraPos,
                               const HMapRenderPlan &mapRenderPlan,
                               M2ObjectListContainer &m2ObjectsCandidates,
                               WMOListContainer &wmoCandidates) override;

    void updateLightAndSkyboxData(const HMapRenderPlan &mapRenderPlan,
                                  MathHelper::FrustumCullingData &frustumData,
                                  StateForConditions &stateForConditions, const AreaRecord &areaRecord) override;

public:
    explicit M2Scene(HApiContainer api, std::string m2Model);
    explicit M2Scene(HApiContainer api, int fileDataId);


    ~M2Scene() override {

    }

    void setReplaceTextureArray(const HMapSceneBufferCreate &sceneRenderer, const std::vector<int> &replaceTextureArray) ;
    void setMeshIdArray(const HMapSceneBufferCreate &sceneRenderer, const std::vector<uint8_t> &meshIds) ;
    void setReplaceParticleColors(std::array<std::array<mathfu::vec4, 3>, 3> &particleColorReplacement) ;
    void resetReplaceParticleColor() ;


    int getCameraNum() ;
    std::shared_ptr<ICamera> createCamera(int cameraNum);
    void exportScene(IExporter* exporter);
};


#endif //WEBWOWVIEWERCPP_M2SCENE_H
