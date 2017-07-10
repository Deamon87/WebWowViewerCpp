//
// Created by deamon on 03.07.17.
//

#ifndef WEBWOWVIEWERCPP_WMOOBJECT_H
#define WEBWOWVIEWERCPP_WMOOBJECT_H

class WmoGroupObject;
#include <string>
#include "m2WmoObject.h"
#include "../persistance/adtFile.h"
#include "../../../3rdparty/mathfu/include/mathfu/glsl_mappings.h"
#include "wmoGroupObject.h"

class WmoObject {
    WmoObject(IWoWInnerApi *api) : m_api(api) {

    }

    IWoWInnerApi *m_api;

    WmoMainGeom *mainGeom = nullptr;
    bool m_loading = false;
    bool m_loaded = false;

    mathfu::mat4 m_placementMatrix;
    mathfu::mat4 m_placementInvertMatrix;
    std::string m_modelName;

    std::vector<WmoGroupObject> groupObjects;

#define toRadian(x) (float) x*M_PI/180

    void createPlacementMatrix(SMMapObjDef &mapObjDef){
        float TILESIZE = 533.333333333;

        float posx = 32*TILESIZE - mapObjDef.position.x;
        float posy = mapObjDef.position.y;
        float posz = 32*TILESIZE - mapObjDef.position.z;


        mathfu::mat4 placementMatrix = mathfu::mat4::Identity();
        placementMatrix *= mathfu::mat4::RotationX(toRadian(90));
        placementMatrix *= mathfu::mat4::RotationY(toRadian(90));


        placementMatrix *= mathfu::mat4::FromTranslationVector(mathfu::vec3(posx, posy, posz));

        placementMatrix *= RotationY(toRadian(mapObjDef.rotation.y-270));
        placementMatrix *= RotationZ(toRadian(-mapObjDef.rotation.x));
        placementMatrix *= RotationX(toRadian(mapObjDef.rotation.z-90));


        mathfu::mat4 placementInvertMatrix = placementMatrix.Inverse();

        m_placementInvertMatrix = placementInvertMatrix;
        m_placementMatrix = placementMatrix;
    }
public:
    std::string getTextureName(int index);

    M2WmoObject& getDoodad(int index);
    void setLoadingParam(std::string modelName, SMMapObjDef &mapObjDef);
    void startLoading();

};


#endif //WEBWOWVIEWERCPP_WMOOBJECT_H
