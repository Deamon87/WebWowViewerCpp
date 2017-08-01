//
// Created by deamon on 03.07.17.
//

#ifndef WEBWOWVIEWERCPP_WMOOBJECT_H
#define WEBWOWVIEWERCPP_WMOOBJECT_H

class WmoGroupObject;
#include <string>
#include "../persistance/header/adtFileHeader.h"
#include "../../../3rdparty/mathfu/include/mathfu/glsl_mappings.h"
#include "wmoGroupObject.h"
#include "m2Object.h"

class WmoObject {

public:
    WmoObject(IWoWInnerApi *api) : m_api(api) {
        m_getTextureFunc = std::bind(&WmoObject::getTexture, this, std::placeholders::_1);
    }
private:
    IWoWInnerApi *m_api;

    WmoMainGeom *mainGeom = nullptr;
    bool m_loading = false;
    bool m_loaded = false;

    std::function <BlpTexture&(int materialId)> m_getTextureFunc;

    mathfu::mat4 m_placementMatrix;
    mathfu::mat4 m_placementInvertMatrix;
    std::string m_modelName;

    std::vector<WmoGroupObject*> groupObjects = std::vector<WmoGroupObject*>(0);
    std::vector<BlpTexture> blpTextures;

#define toRadian(x) (float) x*M_PI/180

    inline mathfu::mat4 RotationX(float angle) {
        return mathfu::quat::FromAngleAxis(angle, mathfu::vec3(1,0,0)).ToMatrix4();
    };

    inline mathfu::mat4 RotationY(float angle) {
        return mathfu::quat::FromAngleAxis(angle, mathfu::vec3(1,0,0)).ToMatrix4();
    };

    inline mathfu::mat4 RotationZ(float angle) {
        return mathfu::quat::FromAngleAxis(angle, mathfu::vec3(1,0,0)).ToMatrix4();
    };

    void createPlacementMatrix(SMMapObjDef &mapObjDef){
        float TILESIZE = 533.333333333;

        float posx = 32*TILESIZE - mapObjDef.position.x;
        float posy = mapObjDef.position.y;
        float posz = 32*TILESIZE - mapObjDef.position.z;


        mathfu::mat4 placementMatrix = mathfu::mat4::Identity();

        placementMatrix *= RotationX(toRadian(90));
        placementMatrix *= RotationY(toRadian(90));


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

//    M2Object& getDoodad(int index);
    BlpTexture& getTexture(int materialId);
    void setLoadingParam(std::string modelName, SMMapObjDef &mapObjDef);
    void startLoading();

    void draw();

    void createGroupObjects();
};


#endif //WEBWOWVIEWERCPP_WMOOBJECT_H
