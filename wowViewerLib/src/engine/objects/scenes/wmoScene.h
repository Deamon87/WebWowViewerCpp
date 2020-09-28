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

    void getPotentialEntities(const mathfu::vec4 &cameraPos, std::vector<std::shared_ptr<M2Object>> &potentialM2,
                                        HCullStage &cullStage, mathfu::mat4 &lookAtMat4, mathfu::vec4 &camera4,
                                        std::vector<mathfu::vec4> &frustumPlanes, std::vector<mathfu::vec3> &frustumPoints,
                                        std::vector<std::shared_ptr<WmoObject>> &potentialWmo) override;

    void getCandidatesEntities(std::vector<mathfu::vec3> &hullLines, mathfu::mat4 &lookAtMat4, mathfu::vec4 &cameraPos,
                               std::vector<mathfu::vec3> &frustumPoints, HCullStage &cullStage,
                               std::vector<std::shared_ptr<M2Object>> &m2ObjectsCandidates,
                               std::vector<std::shared_ptr<WmoObject>> &wmoCandidates) override;
public:
    void setDefaultLightParams(ApiContainer *api) {
        api->getConfig()->setExteriorAmbientColor(0.8,0.8,0.8,0.8);
        api->getConfig()->setExteriorHorizontAmbientColor(1.0,1.0,1.0,1.0);
        api->getConfig()->setExteriorGroundAmbientColor(1.0,1.0,1.0,1.0);
        api->getConfig()->setExteriorDirectColor(0.3,0.3,0.3,1.3);
        api->getConfig()->setExteriorDirectColorDir(0.0,0.0,0.0);
    }

    explicit WmoScene(ApiContainer *api, std::string wmoModel) {
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

        auto wmoObject = std::make_shared<WmoObject>(m_api);
        wmoObject->setLoadingParam(mapObjDef);
        wmoObject->setModelFileName(m_wmoModel);

        m_wmoObject = wmoObject;

        this->setDefaultLightParams(api);
        api->getConfig()->setDisableFog(false);
    };

    explicit WmoScene(ApiContainer *api, int fileDataId) {
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

        auto wmoObject = std::make_shared<WmoObject>(m_api);
        wmoObject->setLoadingParam(mapObjDef);
        wmoObject->setModelFileId(fileDataId);

        m_wmoObject = wmoObject;

        this->setDefaultLightParams(api);
        api->getConfig()->setDisableFog(false);
    };

    ~WmoScene() override {

    }


//    mathfu::vec4 getAmbientColor() override {
////        if (m_wmoObject->isLoaded()) {
////            return mathfu::vec4(m_wmoObject->getAmbientLight(), 0.0);
////        } else
////        return mathfu::vec4(0.0, 0.0, 0.0, 0.0);
//        return m_api->getGlobalAmbientColor();
//    };
//
//    bool getCameraSettings(M2CameraResult&) override {
//        return false;
//    }
//
//    void setAmbientColorOverride(mathfu::vec4 &ambientColor, bool override) override {
//
//    };
};


#endif //WEBWOWVIEWERCPP_WMOSCENE_H
