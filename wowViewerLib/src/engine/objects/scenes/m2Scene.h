//
// Created by deamon on 08.09.17.
//

#ifndef WEBWOWVIEWERCPP_M2SCENE_H
#define WEBWOWVIEWERCPP_M2SCENE_H


#include "../iScene.h"
#include "../m2/m2Object.h"

class M2Scene : public IScene {
private:
    ApiContainer *m_api;
    std::string m_m2Model;
    int m_cameraView;
    mathfu::vec4 m_ambientColorOverride;
    bool doOverride = false;

    M2Object *m_m2Object;
    bool m_drawModel = false;

public:
    M2Scene(ApiContainer *api, std::string m2Model, int cameraView = - 1) : m_api(api), m_m2Model(m2Model), m_cameraView(cameraView){
        M2Object *m2Object = new M2Object(m_api);
        m2Object->setLoadParams(0, {}, {});
        m2Object->setModelFileName(m_m2Model);
        m2Object->createPlacementMatrix(mathfu::vec3(0,0,0), 0, mathfu::vec3(1,1,1), nullptr);
        m2Object->setModelAsScene(cameraView != -1);
        m2Object->calcWorldPosition();

        m_m2Object = m2Object;
    };


    M2Scene(ApiContainer *api, int fileDataId, int cameraView = - 1) : m_api(api), m_cameraView(cameraView){
        M2Object *m2Object = new M2Object(m_api);
        m2Object->setLoadParams(0, {}, {});
        m2Object->setModelFileId(fileDataId);
        m2Object->createPlacementMatrix(mathfu::vec3(0,0,0), 0, mathfu::vec3(1,1,1), nullptr);
        m2Object->setModelAsScene(cameraView != -1);
        m2Object->calcWorldPosition();

        m_m2Object = m2Object;
    };
    ~M2Scene() override {
        delete m_m2Object;
    }

    void setAnimationId(int animationId) override {
        m_m2Object->setAnimationId(animationId);
    };

    M2Object * getM2Object() { return m_m2Object; };

    void setReplaceTextureArray(std::vector<int> &replaceTextureArray) override;
    void checkCulling(HCullStage cullStage) override;


    void doPostLoad(HCullStage cullStage) override;
    void update(HUpdateStage updateStage) override;
    void updateBuffers(HCullStage cullStage) override;

    void produceDrawStage(HDrawStage resultDrawStage, HUpdateStage updateStage) override;
};


#endif //WEBWOWVIEWERCPP_M2SCENE_H
