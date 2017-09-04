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
#include "../geometry/wmoMainGeom.h"

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
    CAaBox m_bbox;

    std::function <BlpTexture&(int materialId)> m_getTextureFunc;

    mathfu::mat4 m_placementMatrix;
    mathfu::mat4 m_placementInvertMatrix;
    std::string m_modelName;

    std::vector<WmoGroupObject*> groupObjects = std::vector<WmoGroupObject*>(0);
    std::vector<BlpTexture> blpTextures;

    std::vector<bool> drawGroupWMO;

    void createPlacementMatrix(SMMapObjDef &mapObjDef);
    void createBB(CAaBox bbox);
public:
    std::string getTextureName(int index);

//    M2Object& getDoodad(int index);
    BlpTexture& getTexture(int materialId);
    void setLoadingParam(std::string modelName, SMMapObjDef &mapObjDef);
    void startLoading();
    bool isLoaded(){ return m_loaded;}
    bool hasPortals() {
        return mainGeom->header->nPortals != 0;
    }

    void draw();

    void createGroupObjects();


    bool checkFrustumCulling(mathfu::vec4 &cameraPos, std::vector<mathfu::vec4> &frustumPlanes,
                             std::vector<mathfu::vec3> &frustumPoints, std::set<M2Object *> &m2RenderedThisFrame);

    void update();
};


#endif //WEBWOWVIEWERCPP_WMOOBJECT_H
