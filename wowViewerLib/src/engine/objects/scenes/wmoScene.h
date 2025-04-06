//
// Created by Deamon on 10/10/2017.
//

#ifndef WEBWOWVIEWERCPP_WMOSCENE_H
#define WEBWOWVIEWERCPP_WMOSCENE_H

#include "map.h"
#include "../wmo/wmoObject.h"
#include "../objectCache.h"

class WmoScene : public Map {
private:
    std::string m_wmoModel;

    std::shared_ptr<WmoObject> m_wmoObject;

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
public:

    explicit WmoScene(HApiContainer api, std::string wmoModel) {
        m_api = api; m_wmoModel = wmoModel;
        m_sceneMode = SceneMode::smWMO;
        m_suppressDrawingSky = true;

        SMMapObjDef mapObjDef;
        mapObjDef.position = C3Vector(mathfu::vec3(17064.6621f, 0, 17066.6738f));
        mapObjDef.rotation = C3Vector(mathfu::vec3(0,0,0));
        mapObjDef.unk = 1024;
        mapObjDef.extents.min = C3Vector(mathfu::vec3(-9999,-9999,-9999));
        mapObjDef.extents.max = C3Vector(mathfu::vec3(9999,9999,9999));
        mapObjDef.doodadSet = 0;

        auto wmoObject = wmoFactory->createObject(m_api);
        wmoObject->setLoadingParam(mapObjDef);
        wmoObject->setModelFileName(m_wmoModel);

        m_wmoObject = wmoObject;
    };

    explicit WmoScene(HApiContainer api, int fileDataId) {
        m_api = api;
        m_sceneMode = SceneMode::smWMO;
        m_suppressDrawingSky = true;

        SMMapObjDef mapObjDef;
        mapObjDef.position = C3Vector(mathfu::vec3(17064.6621f, 0, 17066.6738f));
        mapObjDef.rotation = C3Vector(mathfu::vec3(0,0,0));
        mapObjDef.unk = 1024;
        mapObjDef.extents.min = C3Vector(mathfu::vec3(-9999,-9999,-9999));
        mapObjDef.extents.max = C3Vector(mathfu::vec3(9999,9999,9999));
        mapObjDef.doodadSet = 0;

        auto wmoObject = wmoFactory->createObject(m_api);
        wmoObject->setLoadingParam(mapObjDef);
        wmoObject->setModelFileId(fileDataId);

        m_wmoObject = wmoObject;
    };

    ~WmoScene() override {

    }

    void updateLightAndSkyboxData(const HMapRenderPlan &mapRenderPlan, MathHelper::FrustumCullingData &frustumData,
                                  StateForConditions &stateForConditions, const AreaRecord &areaRecord) override;
};


#endif //WEBWOWVIEWERCPP_WMOSCENE_H
