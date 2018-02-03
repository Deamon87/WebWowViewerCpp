//
// Created by Deamon on 10/10/2017.
//

#ifndef WEBWOWVIEWERCPP_WMOSCENE_H
#define WEBWOWVIEWERCPP_WMOSCENE_H

#include "../iInnerSceneApi.h"
#include "../wmo/wmoObject.h"
#include "../objectCache.h"

class WmoScene : public iInnerSceneApi {
private:
    IWoWInnerApi *m_api;
    std::string m_wmoModel;

    WmoObject *m_wmoObject;

    float m_currentTime = 0;
    float m_lastTimeSort = 0;
    float m_lastTimeDistanceCalc = 0;

    std::vector<WmoGroupResult> m_currentInteriorGroups;
    WmoObject *m_currentWMO = nullptr;

    std::unordered_set<M2Object*> m2RenderedThisFrame;
    std::unordered_set<WmoObject*> wmoRenderedThisFrame;

    std::vector<M2Object*> m2RenderedThisFrameArr;
    std::vector<WmoObject*> wmoRenderedThisFrameArr;

public:
    WmoScene(IWoWInnerApi *api, std::string wmoModel) : m_api (api), m_wmoModel(wmoModel) {
        SMMapObjDef mapObjDef;
        mapObjDef.position = C3Vector(mathfu::vec3(17064.6621f, 0, 17066.6738f));
        mapObjDef.rotation = C3Vector(mathfu::vec3(0,0,0));
        mapObjDef.extents.min = C3Vector(mathfu::vec3(-9999,-9999,-9999));
        mapObjDef.extents.max = C3Vector(mathfu::vec3(9999,9999,9999));
        mapObjDef.doodadSet = 0;

        auto *wmoObject = new WmoObject(m_api);
        wmoObject->setLoadingParam(mapObjDef);
        wmoObject->setModelFileName(m_wmoModel);

        m_wmoObject = wmoObject;
    };

    void checkCulling(mathfu::mat4 &frustumMat, mathfu::mat4 &lookAtMat4, mathfu::vec4 &cameraPos) override;

    void draw() override;

    void update(double deltaTime, mathfu::vec3 &cameraVec3, mathfu::mat4 &frustumMat, mathfu::mat4 &lookAtMat) override;

    void drawM2s();

    mathfu::vec4 getAmbientColor() override {
        return mathfu::vec4(1.0, 1.0, 1.0, 1.0);
    };

    bool getCameraSettings(M2CameraResult&) override {
        return false;
    }
};


#endif //WEBWOWVIEWERCPP_WMOSCENE_H
