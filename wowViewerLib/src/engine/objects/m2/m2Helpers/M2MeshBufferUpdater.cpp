//
// Created by deamon on 09.07.18.
//

#include "M2MeshBufferUpdater.h"
#include "../../../../gapi/interface/meshes/IM2Mesh.h"
#include "../../../persistance/header/M2FileHeader.h"

float M2MeshBufferUpdater::calcFinalTransparency(const M2Object &m2Object, int batchIndex, M2SkinProfile * m2SkinProfile){
    auto textMaterial = m2SkinProfile->batches[batchIndex];
    int renderFlagIndex = textMaterial->materialIndex;

    mathfu::vec4 meshColor = M2Object::getCombinedColor(m2SkinProfile, batchIndex, m2Object.subMeshColors);
    float transparency = M2Object::getTransparency(m2SkinProfile, batchIndex, m2Object.transparencies);
    float finalTransparency = meshColor.w;
    if ( textMaterial->textureCount && !(textMaterial->flags & 0x40)) {
        finalTransparency *= transparency;
    }

	return finalTransparency;
}

void M2MeshBufferUpdater::assignUpdateEvents(HGM2Mesh &hmesh, M2Object *m2Object, M2MaterialInst &materialData, M2Data * m2Data, M2SkinProfile * m2SkinProfile) {
    auto blendMode = hmesh->getGxBlendMode();
    int batchIndex = materialData.batchIndex;
    auto vertexShader = materialData.vertexShader;

    hmesh->getUniformBuffer(2)->setUpdateHandler([m2Object, m2SkinProfile, blendMode, batchIndex, vertexShader](IUniformBufferChunk *self){
        auto m2Data = m2Object->m_m2Geom->getM2Data();

        auto textMaterial = m2SkinProfile->batches[batchIndex];
        int renderFlagIndex = textMaterial->materialIndex;
        auto renderFlag = m2Data->materials[renderFlagIndex];

        mathfu::vec4 meshColor = M2Object::getCombinedColor(m2SkinProfile, batchIndex, m2Object->subMeshColors);
        float transparency = M2Object::getTransparency(m2SkinProfile, batchIndex, m2Object->transparencies);
        float finalTransparency = meshColor.w;
        if ( textMaterial->textureCount && !(textMaterial->flags & 0x40)) {
            finalTransparency *= transparency;
        }

        auto &meshblockVS = self->getObject<M2::meshWideBlockVS>();
        meshblockVS.Color_Transparency = mathfu::vec4_packed(mathfu::vec4(meshColor.x, meshColor.y, meshColor.z, finalTransparency));
        meshblockVS.isSkyBox = m2Object->m_boolSkybox ? 1 : 0;
        meshblockVS.VertexShader = vertexShader;
        meshblockVS.IsAffectedByLight = ((renderFlag->flags & 0x1) > 0) ? 0 : 1;

        fillTextureMatrices(*m2Object, batchIndex, m2Data, m2SkinProfile, meshblockVS.uTextMat);

    });

    //3. Update individual PS buffer
    auto pixelShader = materialData.pixelShader;
    hmesh->getUniformBuffer(4)->setUpdateHandler([m2Object, m2SkinProfile, blendMode, batchIndex, pixelShader](IUniformBufferChunk *self) {
        auto m2Data = m2Object->m_m2Geom->getM2Data();

        auto textMaterial = m2SkinProfile->batches[batchIndex];
        int renderFlagIndex = textMaterial->materialIndex;
        auto renderFlag = m2Data->materials[renderFlagIndex];

        float finalTransparency = M2MeshBufferUpdater::calcFinalTransparency(*m2Object, batchIndex, m2SkinProfile);

        float uAlphaTest;
        if (blendMode == EGxBlendEnum::GxBlend_AlphaKey) {
            uAlphaTest = 128.0f/255.0f * finalTransparency; //Maybe move this to shader logic?
        } else {
            uAlphaTest = 1.0f/255.0f;
        }

//        mathfu::vec3 uFogColor = getFogColor(blendMode, uGlobalFogColor);

        //Fill values into buffer
        auto &meshblockPS = self->getObject<M2::meshWideBlockPS>();
        meshblockPS.PixelShader = pixelShader;
        meshblockPS.IsAffectedByLight = ((renderFlag->flags & 0x1) > 0) ? 0 : 1;
        meshblockPS.UnFogged = ((renderFlag->flags & 0x2) > 0) ? 1 : 0;
        meshblockPS.BlendMode = static_cast<int>(blendMode);
        meshblockPS.uFogColorAndAlphaTest = mathfu::vec4(mathfu::vec3(0,0,0), uAlphaTest);
    });
}

void M2MeshBufferUpdater::updateSortData(HGM2Mesh &hmesh, const M2Object &m2Object, M2MaterialInst &materialData,
                                         const M2Data * m2File, const M2SkinProfile *m2SkinProfile, mathfu::mat4 &modelViewMat) {

    M2Batch *textMaterial = m2SkinProfile->batches.getElement(materialData.batchIndex);
    M2SkinSection *submesh = m2SkinProfile->skinSections.getElement(textMaterial->skinSectionIndex);

    mathfu::vec4 centerBB = mathfu::vec4(mathfu::vec3(submesh->sortCenterPosition), 1.0);

    const mathfu::mat4 &boneMat = m2Object.bonesMatrices[submesh->centerBoneIndex];
    centerBB = modelViewMat * (boneMat * centerBB);

    float value = centerBB.xyz().Length();

    if (textMaterial->flags & 3) {
        mathfu::vec4 resultPoint;

        if ( value > 0.00000023841858 ) {
            resultPoint = centerBB * (1.0f / value);
        } else {
            resultPoint = centerBB;
        }

        mathfu::mat4 mat4 = modelViewMat * boneMat;
        float dist = mat4.GetColumn(3).xyz().Length();
        float sortDist = dist * submesh->sortRadius;

        resultPoint *= sortDist;

        if (textMaterial->flags & 1) {
            value = (centerBB - resultPoint).xyz().Length();
        } else {
            value = (centerBB + resultPoint).xyz().Length();
        }
    }

    hmesh->setSortDistance(value);
}

void M2MeshBufferUpdater::fillLights(const M2Object &m2Object, M2::modelWideBlockPS &modelBlockPS) {
    bool BCLoginScreenHack = m2Object.m_api->getConfig()->getBCLightHack();
    int lightCount = (int) std::min(m2Object.lights.size(), (size_t) 4);
    for (int j = 0; j < lightCount; j++) {
        std::string uniformName;
        mathfu::vec4 attenVec;

        attenVec = mathfu::vec4(m2Object.lights[j].attenuation_start, m2Object.lights[j].diffuse_intensity, m2Object.lights[j].attenuation_end, m2Object.lights.size());


        modelBlockPS.pc_lights[j].attenuation = attenVec;//;lights[i].diffuse_color);

        if (BCLoginScreenHack) {
            modelBlockPS.pc_lights[j].color = m2Object.lights[j].diffuse_color ;
        } else {
            modelBlockPS.pc_lights[j].color = m2Object.lights[j].diffuse_color * m2Object.lights[j].diffuse_intensity;
        }

//        mathfu::vec4 viewPos = modelView * m2Object.lights[j].position;
        modelBlockPS.pc_lights[j].position = m2Object.lights[j].position;
    }
    modelBlockPS.LightCount = lightCount;
    modelBlockPS.bcHack = BCLoginScreenHack ? 1 : 0;
}

void M2MeshBufferUpdater::fillTextureMatrices(const M2Object &m2Object, int batchIndex, M2Data *m2Data,
                                M2SkinProfile *m2SkinProfile, mathfu::mat4 *uTextMat) {

    const auto textureAnim = m2SkinProfile->batches[batchIndex]->textureTransformComboIndex;
    int16_t textureMatIndex = *m2Data->texture_transforms_lookup_table[textureAnim];
    if (textureMatIndex >= 0 && textureMatIndex < m2Object.textAnimMatrices.size()) {
        uTextMat[0] = m2Object.textAnimMatrices[textureMatIndex];
    } else {
        uTextMat[0] = mathfu::mat4::Identity();
    }
    if (textureAnim+1 < m2Data->texture_transforms_lookup_table.size) {
        int textureMatIndex = *m2Data->texture_transforms_lookup_table[textureAnim+1];
        if (textureMatIndex >= 0 && textureMatIndex < m2Object.textAnimMatrices.size()) {
            uTextMat[1] =m2Object.textAnimMatrices[textureMatIndex];
        } else {
            uTextMat[1] = mathfu::mat4::Identity();
        }
    } else {
        uTextMat[1] = mathfu::mat4::Identity();
    }
};

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