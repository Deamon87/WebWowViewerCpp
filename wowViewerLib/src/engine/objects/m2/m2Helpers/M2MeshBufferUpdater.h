//
// Created by deamon on 09.07.18.
//

#ifndef AWEBWOWVIEWERCPP_M2MESHBUFFERUPDATER_H
#define AWEBWOWVIEWERCPP_M2MESHBUFFERUPDATER_H


#include <mathfu/glsl_mappings.h>
#include "./../../../../gapi/UniformBufferStructures.h"
#include "M2MaterialInst.h"
#include "../../../persistance/header/skinFileHeader.h"
#include "../../../persistance/header/M2FileHeader.h"
#include "../m2Object.h"

class M2MeshBufferUpdater {
public:
    static float calcFinalTransparency(const M2Object &m2Object, int batchIndex, M2SkinProfile * m2SkinProfile);
    static void assignUpdateEvents(HGM2Mesh &hmesh, M2Object *m2Object, M2MaterialInst &materialData, M2Data * m2Data, M2SkinProfile * m2SkinProfile);

    static void fillLights(const M2Object &m2Object, M2::modelWideBlockPS &modelBlockPS);

    static mathfu::mat4 getTextureMatrix(const M2Object &m2Object, int textureMatIndex,  M2Data *m2Data);
    static void fillTextureMatrices(const M2Object &m2Object, int batchIndex, M2Data *m2Data,
                             M2SkinProfile *m2SkinProfile, mathfu::mat4 *uTextMat);

    static inline mathfu::vec3 &getFogColor(EGxBlendEnum blendMode, mathfu::vec3 &originalFogColor);

    static void updateSortData(HGM2Mesh &hmesh, const M2Object &m2Object, M2MaterialInst &materialData,
                               const M2Data * m2File, const M2SkinProfile *m2SkinProfile, mathfu::mat4 &modelViewMat);
};



#endif //AWEBWOWVIEWERCPP_M2MESHBUFFERUPDATER_H
