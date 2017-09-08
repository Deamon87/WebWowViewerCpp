//
// Created by deamon on 08.09.17.
//

#ifndef WEBWOWVIEWERCPP_M2SCENE_H
#define WEBWOWVIEWERCPP_M2SCENE_H


#include "iInnerSceneApi.h"
#include "m2Object.h"

class M2Scene : public iInnerSceneApi {
private:
    IWoWInnerApi *m_api;
    std::string m_m2Model;

    M2Object *m_m2Object;
    bool m_drawModel = false;

public:
    M2Scene(IWoWInnerApi *api, std::string m2Model) : m_api(api), m_m2Model(m2Model){
        M2Object *m2Object = new M2Object(m_api);
        m2Object->setLoadParams(m_m2Model, 0, {},{});
        m2Object->createPlacementMatrix(mathfu::vec3(0,0,0), 0, mathfu::vec3(1,1,1), nullptr);
        m2Object->calcWorldPosition();

        m_m2Object = m2Object;
    };

    void checkCulling(mathfu::mat4 &frustumMat, mathfu::mat4 &lookAtMat4, mathfu::vec4 &cameraPos);
    void draw();

    void update(double deltaTime, mathfu::vec3 cameraVec3, mathfu::mat4 lookAtMat);

};


#endif //WEBWOWVIEWERCPP_M2SCENE_H
