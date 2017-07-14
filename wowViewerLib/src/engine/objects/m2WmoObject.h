//
// Created by Deamon on 7/10/2017.
//

#ifndef WEBWOWVIEWERCPP_M2WMOOBJECT_H
#define WEBWOWVIEWERCPP_M2WMOOBJECT_H

#include <cstdint>
#include <mathfu/glsl_mappings.h>
#include "m2Object.h"

class M2WmoObject : public M2Object {
private:

public:
    explicit M2WmoObject(IWoWInnerApi *api): M2Object::M2Object(api){
    }



    void calcWorldPosition(){
        m_worldPosition = (m_placementMatrix * mathfu::vec4(0,0,0,1)).xyz();
    }

    void load(std::string modelName, SMODoodadDef &doodadDef, mathfu::mat4 &wmoPlacementMat){
        createPlacementMatrix(doodadDef, wmoPlacementMat);
        calcWorldPosition();


        this->setLoadParams(modelName, 0, {}, {});
    }
};


#endif //WEBWOWVIEWERCPP_M2WMOOBJECT_H
