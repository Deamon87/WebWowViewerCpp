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
    int m_viewRenderOrder = 0;

    WmoObject *m_currentWMO = nullptr;
public:
    WmoScene(IWoWInnerApi *api, std::string wmoModel) : m_api (api), m_wmoModel(wmoModel) {
        SMMapObjDef mapObjDef;
        mapObjDef.position = C3Vector(mathfu::vec3(17064.6621f, 0, 17066.6738f));
        mapObjDef.rotation = C3Vector(mathfu::vec3(0,0,0));
        mapObjDef.unk = 1024;
        mapObjDef.extents.min = C3Vector(mathfu::vec3(-9999,-9999,-9999));
        mapObjDef.extents.max = C3Vector(mathfu::vec3(9999,9999,9999));
        mapObjDef.doodadSet = 0;

        auto *wmoObject = new WmoObject(m_api);
        wmoObject->setLoadingParam(mapObjDef);
        wmoObject->setModelFileName(m_wmoModel);

        m_wmoObject = wmoObject;
    };

    WmoScene(IWoWInnerApi *api, int fileDataId) : m_api (api) {
        SMMapObjDef mapObjDef;
        mapObjDef.position = C3Vector(mathfu::vec3(17064.6621f, 0, 17066.6738f));
        mapObjDef.rotation = C3Vector(mathfu::vec3(0,0,0));
        mapObjDef.unk = 1024;
        mapObjDef.extents.min = C3Vector(mathfu::vec3(-9999,-9999,-9999));
        mapObjDef.extents.max = C3Vector(mathfu::vec3(9999,9999,9999));
        mapObjDef.doodadSet = 0;

        auto *wmoObject = new WmoObject(m_api);
        wmoObject->setLoadingParam(mapObjDef);
        wmoObject->setModelFileId(fileDataId);

        m_wmoObject = wmoObject;
    };

    void setReplaceTextureArray(std::vector<int> &replaceTextureArray) override {};

    void checkCulling(WoWFrameData *frameData) override;
    void collectMeshes(WoWFrameData*) override ;
    void draw(WoWFrameData *frameData) override;

    void doPostLoad(WoWFrameData *frameData) override;
    void update(WoWFrameData *frameData) override;


    mathfu::vec4 getAmbientColor() override {
//        if (m_wmoObject->isLoaded()) {
//            return mathfu::vec4(m_wmoObject->getAmbientLight(), 0.0);
//        } else
//        return mathfu::vec4(0.0, 0.0, 0.0, 0.0);
        return m_api->getGlobalAmbientColor();
    };

    bool getCameraSettings(M2CameraResult&) override {
        return false;
    }

    void setAmbientColorOverride(mathfu::vec4 &ambientColor, bool override) override {

    };

    void cullExterior(WoWFrameData *frameData, int viewRenderOrder);
};


#endif //WEBWOWVIEWERCPP_WMOSCENE_H
