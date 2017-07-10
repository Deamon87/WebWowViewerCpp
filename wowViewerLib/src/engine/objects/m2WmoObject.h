//
// Created by Deamon on 7/10/2017.
//

#ifndef WEBWOWVIEWERCPP_M2WMOOBJECT_H
#define WEBWOWVIEWERCPP_M2WMOOBJECT_H

#include "m2Object.h"
#include <cstdint>
#include <mathfu/glsl_mappings.h>

class M2WmoObject : public M2Object {
private:
    mathfu::mat4 m_placementMatrix;
    mathfu::mat4 m_inversePlacementMatrix;
    mathfu::vec3 m_worldPosition;
public:
    explicit M2WmoObject(IWoWInnerApi *api): M2Object::M2Object(api){
    }

    void createPlacementMatrix(SMODoodadDef &def, mathfu::mat4 &wmoPlacementMat) {
        mathfu::mat4 placementMatrix = mathfu::mat4::Identity();
        placementMatrix = placementMatrix * wmoPlacementMat;
        placementMatrix = placementMatrix * mathfu::mat4::FromTranslationVector(mathfu::vec3(def.position));

        mathfu::quat quat4(def.orientation.w, def.orientation.x, def.orientation.y, def.orientation.z);
        placementMatrix = placementMatrix * quat4.ToMatrix4();

        float scale = def.scale;
        placementMatrix = placementMatrix * mathfu::mat4::FromScaleVector(mathfu::vec3(scale,scale,scale));

        mathfu::mat4 invertPlacementMatrix = placementMatrix.Inverse();

        m_placementMatrix = placementMatrix;
        m_inversePlacementMatrix = invertPlacementMatrix;
    }

    void calcWorldPosition(){
        m_worldPosition = (m_placementMatrix * mathfu::vec4(0,0,0,1)).xyz();
    }

    void load(std::string modelName, SMODoodadDef &doodadDef, mathfu::mat4 &wmoPlacementMat){
        createPlacementMatrix(doodadDef, wmoPlacementMat);
        calcWorldPosition();


        setLoadParams(modelName, 0, {}, {});
    }
};


#endif //WEBWOWVIEWERCPP_M2WMOOBJECT_H
