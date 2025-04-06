//
// Created by deamon on 09.07.18.
//

#include "M2MeshBufferUpdater.h"
#include "../../../../gapi/interface/meshes/IM2Mesh.h"
#include "../../../persistance/header/M2FileHeader.h"

float M2MeshBufferUpdater::calcFinalTransparency(const M2Object &m2Object, int batchIndex, M2SkinProfile * m2SkinProfile){
    auto textMaterial = m2SkinProfile->batches[batchIndex];

    mathfu::vec4 meshColor = getCombinedColor(m2SkinProfile, batchIndex, m2Object.subMeshColors);
    float transparency = getTextureWeight(m2SkinProfile, m2Object.m_m2Geom->getM2Data(), batchIndex, 0, m2Object.transparencies);
    float finalTransparency = meshColor.w;
    if ( textMaterial->textureCount && !(textMaterial->flags & 0x40)) {
        finalTransparency *= transparency;
    }

    finalTransparency *= m2Object.m_alpha;

	return finalTransparency;
}

void M2MeshBufferUpdater::updateMaterialData(const std::shared_ptr<IM2Material> &m2Material, M2Object *m2Object, M2Data * m2Data, M2SkinProfile * m2SkinProfile){
    int batchIndex = m2Material->batchIndex;
    auto batch = m2SkinProfile->batches[batchIndex];
    int renderFlagIndex = batch->materialIndex;
    auto renderFlag = m2Data->materials[renderFlagIndex];

    std::array<int,2> textureMatrixIndexes = {-1, -1};
    getTextureMatrixIndexes(*m2Object, batchIndex, m2Data, m2SkinProfile, textureMatrixIndexes);

    //2. Update VSPS buffer
    auto &meshblockVSPS = m2Material->m_vertexFragmentData->getObject();
    meshblockVSPS.VertexShader = m2Material->vertexShader;
    meshblockVSPS.IsAffectedByLight = ((renderFlag->flags & 0x1) > 0) ? 0 : 1;
    meshblockVSPS.textureMatIndex1 = textureMatrixIndexes[0];
    meshblockVSPS.textureMatIndex2 = textureMatrixIndexes[1];
    meshblockVSPS.PixelShader = m2Material->pixelShader;
    meshblockVSPS.UnFogged = ((renderFlag->flags & 0x2) > 0) ? 1 : 0;
    meshblockVSPS.BlendMode = static_cast<int>(m2Material->blendMode);
    meshblockVSPS.applyWeight = batch->textureCount && !(batch->flags & 0x40);
    meshblockVSPS.colorIndex =
        ((batch->colorIndex >= 0) && (batch->colorIndex < m2Data->colors.size))
        ? batch->colorIndex
        : -1;
    meshblockVSPS.textureWeightIndexes[0] = getTextureWeightIndex(m2SkinProfile, m2Data, batchIndex, 0);
    meshblockVSPS.textureWeightIndexes[1] = getTextureWeightIndex(m2SkinProfile, m2Data, batchIndex, 1);
    meshblockVSPS.textureWeightIndexes[2] = getTextureWeightIndex(m2SkinProfile, m2Data, batchIndex, 2);
    meshblockVSPS.textureWeightIndexes[3] = getTextureWeightIndex(m2SkinProfile, m2Data, batchIndex, 3);

    meshblockVSPS.PixelShader = m2Material->pixelShader;
    meshblockVSPS.IsAffectedByLight = ((renderFlag->flags & 0x1) > 0) ? 0 : 1;

    m2Material->m_vertexFragmentData->save();
}

void M2MeshBufferUpdater::updateSortData(HGM2Mesh &hmesh, const M2Object &m2Object, int batchIndex,
                                         const M2Data * m2File, const M2SkinProfile *m2SkinProfile, const mathfu::mat4 &modelViewMat) {

    M2Batch *textMaterial = m2SkinProfile->batches.getElement(batchIndex);
    M2SkinSection *submesh = m2SkinProfile->skinSections.getElement(textMaterial->skinSectionIndex);

    mathfu::vec4 sortCenterPosition = mathfu::vec4(mathfu::vec3(submesh->sortCenterPosition), 1.0);

    const mathfu::mat4 &boneMat = m2Object.bonesMatrices[submesh->centerBoneIndex];
    sortCenterPosition = modelViewMat * (boneMat * sortCenterPosition);

    float value = sortCenterPosition.xyz().Length();

    if (textMaterial->flags & 3) {
        mathfu::vec4 resultPoint;

        if ( value > 0.00000023841858 ) {
            resultPoint = sortCenterPosition * (1.0f / value);
        } else {
            resultPoint = sortCenterPosition;
        }

        mathfu::mat4 mat4 = modelViewMat * boneMat;
        float scale = mat4.GetColumn(0).xyz().Length();
        float sortDist = scale * submesh->sortRadius;

        resultPoint *= sortDist;

        if (textMaterial->flags & 1) {
            value = (sortCenterPosition - resultPoint).xyz().Length();
        } else {
            value = (sortCenterPosition + resultPoint).xyz().Length();
        }
    }

    hmesh->setSortDistance(value);
}

mathfu::mat4 M2MeshBufferUpdater::getTextureMatrix(const M2Object &m2Object, int textureMatIndex,  M2Data *m2Data) {
    if (textureMatIndex < 0)
        return mathfu::mat4::Identity();

    if (textureMatIndex >= m2Object.textAnimMatrices.size())
        return mathfu::mat4::Identity();

    return m2Object.textAnimMatrices[textureMatIndex];
}

void M2MeshBufferUpdater::getTextureMatrixIndexes(const M2Object &m2Object, int batchIndex, M2Data *m2Data,
                                        const M2SkinProfile *m2SkinProfile, std::array<int, 2> &o_textureMatIndexes) {
    auto textureAnim = m2SkinProfile->batches[batchIndex]->textureTransformComboIndex;

    if (m2Object.m_m2Geom->m_wfv1 != nullptr) {
        textureAnim = 1; // hack for fdid 2445860
    }

    int16_t textureMatIndex = -1;
    if (textureAnim < m2Data->texture_transforms_lookup_table.size)
        textureMatIndex = *m2Data->texture_transforms_lookup_table[textureAnim];

    o_textureMatIndexes[0] = textureMatIndex;

    textureMatIndex = -1;
    if (textureAnim+1 < m2Data->texture_transforms_lookup_table.size)
        textureMatIndex = *m2Data->texture_transforms_lookup_table[textureAnim+1];

    o_textureMatIndexes[1] = textureMatIndex;
}

mathfu::vec4 M2MeshBufferUpdater::getCombinedColor(
    M2SkinProfile *skinData,
    int batchIndex,
    const std::vector<mathfu::vec4, tbb::cache_aligned_allocator<mathfu::vec4>> &subMeshColors
) {
    int colorIndex = skinData->batches[batchIndex]->colorIndex;
    mathfu::vec4 submeshColor = mathfu::vec4(1,1,1,1);

    if ((colorIndex >= 0) && (colorIndex < subMeshColors.size())) {
        const mathfu::vec4 &color = subMeshColors[colorIndex];
        submeshColor = color;
    }

    return submeshColor;
}

float M2MeshBufferUpdater::getTextureWeight(
    M2SkinProfile *skinData,
    M2Data * m2Data,
    int batchIndex,
    int textureIndex,
    const std::vector<float> &transparencies) {
    float transparency = 1.0;

    int transpIndex = getTextureWeightIndex(skinData, m2Data, batchIndex, textureIndex);

    if ((transpIndex >= 0) && (transparencies.size() > transpIndex)) {
        transparency = transparencies[transpIndex];
    }

    return transparency;
}

int
M2MeshBufferUpdater::getTextureWeightIndex(const M2SkinProfile *skinData, const M2Data *m2Data, int batchIndex,
                                           int textureIndex) {
    if (textureIndex >= skinData->batches[batchIndex]->textureCount) {
        return -1;
    }

    int transpLookupIndex = skinData->batches[batchIndex]->textureWeightComboIndex + textureIndex;
    int transpIndex = -1;
    if ((transpLookupIndex >= 0) && (transpLookupIndex < m2Data->transparency_lookup_table.size)) {
        transpIndex = *m2Data->transparency_lookup_table[transpLookupIndex];
    }
    return transpIndex;
}

void M2MeshBufferUpdater::fillTextureMatrices(const M2Object &m2Object, int batchIndex, M2Data *m2Data,
                                M2SkinProfile *m2SkinProfile, mathfu::mat4 *uTextMat) {

    std::array<int,2> textureMatrixIndexes;
    getTextureMatrixIndexes(m2Object, batchIndex, m2Data, m2SkinProfile, textureMatrixIndexes);

    uTextMat[0] = M2MeshBufferUpdater::getTextureMatrix(m2Object, textureMatrixIndexes[0], m2Data);
    uTextMat[1] = M2MeshBufferUpdater::getTextureMatrix(m2Object, textureMatrixIndexes[1], m2Data);
}

mathfu::vec3 &M2MeshBufferUpdater::getFogColor(EGxBlendEnum blendMode, mathfu::vec3 &originalFogColor) {

    static mathfu::vec3 fog_zero = mathfu::vec3(0,0,0);
    static mathfu::vec3 fog_half = mathfu::vec3(0.5,0.5,0.5);
    static mathfu::vec3 fog_one = mathfu::vec3(1.0,1.0,1.0);

    switch (blendMode) {
        case EGxBlendEnum::GxBlend_Opaque: //Blend_Opaque
        case EGxBlendEnum::GxBlend_AlphaKey : //Blend_AlphaKey
        case EGxBlendEnum::GxBlend_Alpha : //Blend_Alpha
            return originalFogColor;

        case EGxBlendEnum::GxBlend_NoAlphaAdd  : //Blend_NoAlphaAdd
        case EGxBlendEnum::GxBlend_Add : //Blend_Add
            return fog_zero;

        case EGxBlendEnum::GxBlend_Mod: //Blend_Mod
            return fog_one;

        case EGxBlendEnum::GxBlend_Mod2x:
        case EGxBlendEnum::GxBlend_BlendAdd:
            return fog_half;

        default :
            debuglog("Unknown blending mode in M2 file")
            break;
    }

    return originalFogColor;
}