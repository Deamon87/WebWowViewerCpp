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

void M2MeshBufferUpdater::updateMaterialData(const std::shared_ptr<IM2Material> &m2Material, M2Object *m2Object, M2SkinProfile * m2SkinProfile){
    auto &m2Geom = m2Object->m_m2Geom;;
    const M2Data * m2Data = m2Geom->getM2Data();
    const auto &txacVals = m2Geom->txacMesh;


    int batchIndex = m2Material->batchIndex;
    auto batch = m2SkinProfile->batches[batchIndex];
    int renderFlagIndex = batch->materialIndex;
    auto renderFlag = m2Data->materials[renderFlagIndex];

    int txacVal1 = 0, txacVal2 = 0;
    if (renderFlagIndex >= 0 && renderFlagIndex < txacVals.size()) {
        txacVal1 = txacVals[renderFlagIndex].perByte[0];
        txacVal2 = txacVals[renderFlagIndex].perByte[1];
    }

    std::array<int,2> textureMatrixIndexes = {-1, -1};
    getTextureMatrixIndexes(*m2Object, batchIndex, m2Material->vertexShader, m2Data, m2SkinProfile, textureMatrixIndexes);



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
    meshblockVSPS.txac1 = txacVal1;
    meshblockVSPS.txac2 = txacVal2;

    m2Material->m_vertexFragmentData->save();
}
void M2MeshBufferUpdater::updateProjectiveMaterialData(int batchIndex, uint8_t blendMode, uint8_t vertexIndex, uint8_t pixelShader,
                                                       const std::shared_ptr<IM2ProjectiveMaterial> &m2Material, M2Object *m2Object,
                                                       const M2Data * m2Data, M2SkinProfile * m2SkinProfile) {
    if (!m2Material) return;

    auto batch = m2SkinProfile->batches[batchIndex];
    auto skinSection = m2SkinProfile->skinSections[batch->skinSectionIndex];
    int renderFlagIndex = batch->materialIndex;
    auto renderFlag = m2Data->materials[renderFlagIndex];

    std::array<int,2> textureMatrixIndexes = {-1, -1};
    getTextureMatrixIndexes(*m2Object, batchIndex, vertexIndex, m2Data, m2SkinProfile, textureMatrixIndexes);

    //2. Update VSPS buffer
    {
        auto &meshblockVSPS = m2Material->m_vertexFragmentData->getObject();
        meshblockVSPS.VertexShader = 0;
        meshblockVSPS.IsAffectedByLight = ((renderFlag->flags & 0x1) > 0) ? 0 : 1;
        meshblockVSPS.textureMatIndex1 = textureMatrixIndexes[0];
        meshblockVSPS.textureMatIndex2 = textureMatrixIndexes[1];
        meshblockVSPS.UnFogged = ((renderFlag->flags & 0x2) > 0) ? 1 : 0;
        meshblockVSPS.BlendMode = blendMode;
        meshblockVSPS.applyWeight = batch->textureCount && !(batch->flags & 0x40);
        meshblockVSPS.colorIndex =
            ((batch->colorIndex >= 0) && (batch->colorIndex < m2Data->colors.size))
            ? batch->colorIndex
            : -1;
        meshblockVSPS.textureWeightIndexes[0] = getTextureWeightIndex(m2SkinProfile, m2Data, batchIndex, 0);
        meshblockVSPS.textureWeightIndexes[1] = getTextureWeightIndex(m2SkinProfile, m2Data, batchIndex, 1);
        meshblockVSPS.textureWeightIndexes[2] = getTextureWeightIndex(m2SkinProfile, m2Data, batchIndex, 2);
        meshblockVSPS.textureWeightIndexes[3] = getTextureWeightIndex(m2SkinProfile, m2Data, batchIndex, 3);

        meshblockVSPS.PixelShader = pixelShader;
        meshblockVSPS.IsAffectedByLight = ((renderFlag->flags & 0x1) > 0) ? 0 : 1;

        m2Material->m_vertexFragmentData->save();
    }

    {
        //Iterate over vertexes of projective mesh to get the min max, and to pick three
        //affinely-independent vertices (by position) to build the local-position -> UV
        //transform from. We feed their *actual* UV values into createProjectionalTexture
        //rather than assuming any vertex sits exactly at UV (0,0)/(1,0)/(0,1) — some models'
        //decal UVs are offset by a non-trivial amount from those corners

        //assert(skinSection->vertexCount == 4);

        auto min = mathfu::vec3(9999, 9999, 9999);
        auto max = mathfu::vec3(-9999, -9999, -9999);

        mathfu::vec2 pos0, pos1, pos2;
        mathfu::vec2 uv0, uv1, uv2;
        int cornersFound = 0;

        for (int vertIndex = skinSection->vertexStart; vertIndex < skinSection->vertexStart + skinSection->vertexCount; ++vertIndex) {
            auto const &vertex = *m2Data->vertices[vertIndex];

            mathfu::vec2 pos = mathfu::vec3(vertex.pos).xy();
            mathfu::vec2 uv = mathfu::vec2(vertex.tex_coords[0].x, vertex.tex_coords[0].y);

            if (cornersFound == 0) {
                pos0 = pos; uv0 = uv;
                cornersFound = 1;
            } else if (cornersFound == 1) {
                if (!feq(pos.x, pos0.x) || !feq(pos.y, pos0.y)) {
                    pos1 = pos; uv1 = uv;
                    cornersFound = 2;
                }
            } else if (cornersFound == 2) {
                float cross = (pos1.x - pos0.x) * (pos.y - pos0.y) - (pos1.y - pos0.y) * (pos.x - pos0.x);
                if (!feq(cross, 0, 0.0001f)) {
                    pos2 = pos; uv2 = uv;
                    cornersFound = 3;
                }
            }

            min = mathfu::vec3(
               mathfu::vec3(
                   std::min(min.x, vertex.pos.x),
                   std::min(min.y, vertex.pos.y),
                   std::min(min.z, vertex.pos.z)
               )
           );

           max = mathfu::vec3(
               mathfu::vec3(
                   std::max(max.x, vertex.pos.x),
                   std::max(max.y, vertex.pos.y),
                   std::max(max.z, vertex.pos.z)
               )
           );
        }

        auto &projectiveData = m2Material->m_projectiveTextData->getObject();
        projectiveData.localMin = mathfu::vec4(min.x, min.y, min.z, 0);
        projectiveData.localMax = mathfu::vec4(max.x, max.y, max.z, 0);
        projectiveData.localToUVMat = MathHelper::createProjectionalTexture(pos0, uv0, pos1, uv1, pos2, uv2);
        m2Material->m_projectiveTextData->save();
    }

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

#ifdef DEBUG_MESH_NAMES
    // Keep the mesh's renderdoc label in sync with the latest sort data
    hmesh->setDebugName(std::string("M2,") +
                        " FileDataId = " + std::to_string(m2Object.m_modelFileId) +
                        " batchFlags = " + std::to_string(textMaterial->flags) +
                        " priorityPlane = " + std::to_string(textMaterial->priorityPlane) +
                        " sortDistance = " + std::to_string(value) +
                        " isTransparent = " + std::to_string(hmesh->getIsTransparent()));
#endif
}

mathfu::mat4 M2MeshBufferUpdater::getTextureMatrix(const M2Object &m2Object, int textureMatIndex, const M2Data *m2Data) {
    if (textureMatIndex < 0)
        return mathfu::mat4::Identity();

    if (textureMatIndex >= m2Object.textAnimMatrices.size())
        return mathfu::mat4::Identity();

    return m2Object.textAnimMatrices[textureMatIndex];
}

void M2MeshBufferUpdater::getTextureMatrixIndexes(const M2Object &m2Object, int batchIndex, uint8_t vertexIndex, const M2Data *m2Data,
                                        const M2SkinProfile *m2SkinProfile, std::array<int, 2> &o_textureMatIndexes) {
    auto const & batch = *m2SkinProfile->batches[batchIndex];
    auto textureAnim = batch.textureTransformComboIndex;
    auto textureCount = batch.textureCount;

    std::array<uint8_t, 2> textureSlots = {0, 1};

    if (m2Object.m_m2Geom->m_wfv1 != nullptr) {
        textureAnim = 1; // hack for fdid 2445860
    }

    if (vertexIndex == 11) { //Diffuse_T1_Env_T2
        textureSlots = {0, 2};
    }
    for (int i = 0; i < std::min<int>(textureCount, 2); i++) {
        int16_t textureMatIndex = -1;

        int textureAnimIndex = textureAnim + textureSlots[i];
        if (textureAnimIndex < m2Data->texture_transforms_lookup_table.size)
            textureMatIndex = *m2Data->texture_transforms_lookup_table[textureAnimIndex];

        o_textureMatIndexes[i] = textureMatIndex;
    }
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
    const M2SkinProfile *skinData,
    const M2Data * m2Data,
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

void M2MeshBufferUpdater::fillTextureMatrices(const M2Object &m2Object, int batchIndex, uint8_t vertexIndex,
                                M2Data *m2Data, M2SkinProfile *m2SkinProfile, mathfu::mat4 *uTextMat) {

    std::array<int,2> textureMatrixIndexes;
    getTextureMatrixIndexes(m2Object, batchIndex, vertexIndex, m2Data, m2SkinProfile, textureMatrixIndexes);

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