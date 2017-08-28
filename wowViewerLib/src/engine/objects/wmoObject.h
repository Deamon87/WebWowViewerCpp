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

    void createPlacementMatrix(SMMapObjDef &mapObjDef);
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
