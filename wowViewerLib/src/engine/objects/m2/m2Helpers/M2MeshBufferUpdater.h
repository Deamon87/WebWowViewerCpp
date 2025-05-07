//
// Created by deamon on 09.07.18.
//

#ifndef AWEBWOWVIEWERCPP_M2MESHBUFFERUPDATER_H
#define AWEBWOWVIEWERCPP_M2MESHBUFFERUPDATER_H
#define _USE_MATH_DEFINES

#define NOMINMAX 1

#include <mathfu/glsl_mappings.h>
#include "./../../../../gapi/UniformBufferStructures.h"
#include "M2MaterialInst.h"
#include "../../../persistance/header/skinFileHeader.h"
#include "../../../persistance/header/M2FileHeader.h"
#include "../m2Object.h"



class M2MeshBufferUpdater {
public:
    static float calcFinalTransparency(const M2Object &m2Object, int batchIndex, M2SkinProfile * m2SkinProfile);

    static void updateMaterialData(const std::shared_ptr<IM2Material> &m2Material, M2Object *m2Object, M2Data * m2Data, M2SkinProfile * m2SkinProfile);
    static void updateProjectiveMaterialData(int batchIndex, uint8_t blendMode, uint8_t pixelShader,
        const std::shared_ptr<IM2ProjectiveMaterial> &m2Material, M2Object *m2Object, M2Data * m2Data, M2SkinProfile * m2SkinProfile);

    static mathfu::mat4 getTextureMatrix(const M2Object &m2Object, int textureMatIndex,  M2Data *m2Data);

    static void getTextureMatrixIndexes(const M2Object &m2Object, int batchIndex, M2Data *m2Data,
                                            const M2SkinProfile *m2SkinProfile, std::array<int, 2> &o_textureMatIndexes);

    static mathfu::vec4 getCombinedColor(M2SkinProfile *skinData, int batchIndex,  const std::vector<mathfu::vec4, tbb::cache_aligned_allocator<mathfu::vec4>> &subMeshColors) ;
    static float getTextureWeight(M2SkinProfile *skinData, M2Data *m2data, int batchIndex, int textureIndex, const std::vector<float> &transparencies) ;

    static void fillTextureMatrices(const M2Object &m2Object, int batchIndex, M2Data *m2Data,
                             M2SkinProfile *m2SkinProfile, mathfu::mat4 *uTextMat);

    static inline mathfu::vec3 &getFogColor(EGxBlendEnum blendMode, mathfu::vec3 &originalFogColor);

    static void updateSortData(HGM2Mesh &hmesh, const M2Object &m2Object, int batchIndex,
                               const M2Data * m2File, const M2SkinProfile *m2SkinProfile, const mathfu::mat4 &modelViewMat);

    static inline int getTextureWeightIndex(const M2SkinProfile *skinData, const M2Data *m2Data,
                                     int batchIndex, int textureIndex);
};



#endif //AWEBWOWVIEWERCPP_M2MESHBUFFERUPDATER_H
