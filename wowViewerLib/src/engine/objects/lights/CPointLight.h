//
// Created by Deamon on 4/1/2024.
//

#ifndef AWEBWOWVIEWERCPP_CPOINTLIGHT_H
#define AWEBWOWVIEWERCPP_CPOINTLIGHT_H

#include "../../persistance/wdtLightFile.h"
#include "../../persistance/header/wmoFileHeader.h"
#include "../../../gapi/UniformBufferStructures.h"

class CPointLight {
public:
    CPointLight();
    CPointLight(const WdtLightFile::MapPointLight3 &lightRecord);
    CPointLight(const mathfu::mat4 &modelMat, const map_object_point_light &lightRecord);
    CPointLight(const mathfu::mat4 &modelMat, const map_object_pointlight_anim &lightRecord);

    const LocalLight& getLightRec() const {
        return m_localLight;
    }
private:
    LocalLight m_localLight;
};


#endif //AWEBWOWVIEWERCPP_CPOINTLIGHT_H
