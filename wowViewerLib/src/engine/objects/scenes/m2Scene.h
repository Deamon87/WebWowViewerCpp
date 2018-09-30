//
// Created by deamon on 08.09.17.
//

#ifndef WEBWOWVIEWERCPP_M2SCENE_H
#define WEBWOWVIEWERCPP_M2SCENE_H


#include "../iInnerSceneApi.h"
#include "../m2/m2Object.h"

class M2Scene : public iInnerSceneApi {
private:
    IWoWInnerApi *m_api;
    std::string m_m2Model;
    int m_cameraView;
    mathfu::vec4 m_ambientColorOverride;
    bool doOverride = false;

    M2Object *m_m2Object;
    bool m_drawModel = false;

public:
    M2Scene(IWoWInnerApi *api, std::string m2Model, int cameraView = - 1) : m_api(api), m_m2Model(m2Model), m_cameraView(cameraView){
        M2Object *m2Object = new M2Object(m_api);
        m2Object->setLoadParams(0, {},{});
        m2Object->setModelFileName(m_m2Model);
        m2Object->createPlacementMatrix(mathfu::vec3(0,0,0), 0, mathfu::vec3(1,1,1), nullptr);
        m2Object->setModelAsScene(cameraView != -1);
        m2Object->calcWorldPosition();

        m_m2Object = m2Object;
    };

    M2Scene(IWoWInnerApi *api, int fileDataId, int cameraView = - 1) : m_api(api), m_cameraView(cameraView){
            M2Object *m2Object = new M2Object(m_api);
            m2Object->setLoadParams(0, {},{});
            m2Object->setModelFileId(fileDataId);
            m2Object->createPlacementMatrix(mathfu::vec3(0,0,0), 0, mathfu::vec3(1,1,1), nullptr);
            m2Object->setModelAsScene(cameraView != -1);
            m2Object->calcWorldPosition();

            m_m2Object = m2Object;
    };

    M2Object * getM2Object() { return m_m2Object; };

    void checkCulling(WoWFrameData *frameData) override;
    void draw(WoWFrameData *frameData) override;
    void collectMeshes(WoWFrameData*) override;
    void doPostLoad(WoWFrameData *frameData) override;
    void update(WoWFrameData *frameData) override;
    mathfu::vec4 getAmbientColor() override;
    void setAmbientColorOverride(mathfu::vec4 &ambientColor, bool override) override;
    bool getCameraSettings(M2CameraResult &result) override ;

};


#endif //WEBWOWVIEWERCPP_M2SCENE_H
