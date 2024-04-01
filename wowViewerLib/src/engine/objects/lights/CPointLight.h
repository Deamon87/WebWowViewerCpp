//
// Created by Deamon on 4/1/2024.
//

#ifndef AWEBWOWVIEWERCPP_CPOINTLIGHT_H
#define AWEBWOWVIEWERCPP_CPOINTLIGHT_H

#include "../../persistance/wdtLightFile.h"
#include "../../../gapi/UniformBufferStructures.h"

class CPointLight {
public:
    CPointLight();
    CPointLight(WdtLightFile::MapPointLight3 &lightRecord);

    LocalLight& getLightRec() {
        return m_localLight;
    }
private:
    LocalLight m_localLight;
};


#endif //AWEBWOWVIEWERCPP_CPOINTLIGHT_H
