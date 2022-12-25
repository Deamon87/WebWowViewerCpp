//
// Created by deamon on 22.06.17.
//

#include "skinGeom.h"
#include "../persistance/header/M2FileHeader.h"
#include "../../gapi/interface/IDevice.h"

void SkinGeom::process(HFileContent skinFile, const std::string &fileName) {
    this->m2Skin = skinFile;

    M2SkinProfile *skinHeader = (M2SkinProfile *) &(*this->m2Skin.get())[0];
    this->m_skinData = skinHeader;

    //Step 1: Init all m2Arrays
    skinHeader->vertices.initM2Array(skinHeader);
    skinHeader->indices.initM2Array(skinHeader);
    skinHeader->bones.initM2Array(skinHeader);
    skinHeader->skinSections.initM2Array(skinHeader);
    skinHeader->batches.initM2Array(skinHeader);

    fsStatus = FileStatus::FSLoaded;
}
void SkinGeom::fixShaderIdBasedOnBlendOverride(M2Data *m2File) {
    M2SkinProfile* skinFileData = this->m_skinData;

    for (int i = 0; i < skinFileData->batches.size; i++){
        M2Batch *skinTextureDefinition = skinFileData->batches[i];
        if ((skinTextureDefinition->shader_id & 0x8000) > 0) continue;

        bool hasBlendingOverrides = m2File->global_flags.flag_has_blend_maps > 0;
        M2Material* m2Material = m2File->materials[skinTextureDefinition->materialIndex];
        uint16_t blendingMode = m2Material->blending_mode;

        if (!hasBlendingOverrides) {
            uint16_t texUnit = *m2File->tex_unit_lookup_table[skinTextureDefinition->textureCoordComboIndex];

            uint16_t newShaderId = (blendingMode != 0) ? 1 : 0;
            if (texUnit > 2) {
                newShaderId |= 8;
            }
            newShaderId = newShaderId << 4;
            if (texUnit == 1) {
                newShaderId = newShaderId | 0x4000;
            }

            skinTextureDefinition->shader_id = newShaderId;
        } else {
            uint16_t op_count = skinTextureDefinition->textureCount;
            uint16_t currShaderId = skinTextureDefinition->shader_id;
            uint32_t runtimeShaderVals[2] = {0,0};

            uint16_t newShaderId = 0;
            for (int j = 0; j < op_count; j++) {

                uint16_t blendMapVal = *m2File->blend_map_overrides[currShaderId + j];
                if (j == 0 && blendingMode == 0) {
                    blendMapVal = 0;
                }

                uint16_t texUnit = *m2File->tex_unit_lookup_table[skinTextureDefinition->textureCoordComboIndex + j];

                runtimeShaderVals[j] = blendMapVal;
                if (texUnit > 2) {
                    runtimeShaderVals[j] = blendMapVal | 8;
                }

                if ( (texUnit == 1) && (j+1 == op_count) )
                    newShaderId |= 0x4000;
            }

            newShaderId |= (runtimeShaderVals[1] & 0xFFFF) | ((runtimeShaderVals[0] << 4) & 0xFFFF);
            skinTextureDefinition->shader_id = newShaderId;
        }
    }
}

void SkinGeom::fixData(M2Data *m2File) {
    //TODO: enable this for WOTLK
//    if (!this->m_fixed) {
//        this->fixShaderIdBasedOnBlendOverride(m2File);
//        this->fixShaderIdBasedOnLayer(m2File);
//
//        this->m_fixed = true;
//    }
}

void SkinGeom::fixShaderIdBasedOnLayer(M2Data *m2File) {
/*
    M2SkinProfile* skinFileData = this->m_skinData;

    bool reducingIsNeeded = false;
    int prevRenderFlagIndex = -1;

    M2Batch* lowerLayerSkin = nullptr;
    int someFlags = 0;
    for (int i = 0; i < skinFileData->batches.size; i++) {
        M2Batch *texDef = skinFileData->batches[i];
        uint16_t currRenderFlagIndex = texDef->materialIndex;
        if (currRenderFlagIndex == prevRenderFlagIndex) {
            reducingIsNeeded = true;
            continue;
        }
        prevRenderFlagIndex = currRenderFlagIndex;
        M2Material *renderFlag = m2File->materials[currRenderFlagIndex];

        int lowerBits = texDef->shader_id & 7;
        if (texDef->materialLayer == 0) {

            if ((texDef->textureCount>= 1) && (renderFlag->blending_mode == 0)) {
                texDef->shader_id &= 0xFF8F;
            }
            lowerLayerSkin = texDef;
        }
        //Line 84
        if ((someFlags & 0xFF) == 1) {
            uint16_t blendingMode = renderFlag->blending_mode;
            if ((blendingMode == 2 || blendingMode == 1)
                && (texDef->textureCount == 1)
                && ((((renderFlag->flags & 0xff) ^ (m2File->materials[lowerLayerSkin->materialIndex]->flags & 0xff)) & 1) == 0)
                && texDef->textureComboIndex == lowerLayerSkin->textureComboIndex)
            {
                if (*m2File->transparency_lookup_table[lowerLayerSkin->textureWeightComboIndex] ==
                        *m2File->transparency_lookup_table[texDef->textureWeightComboIndex]) {
                    texDef->shader_id = 0x8000;
                    lowerLayerSkin->shader_id = 0x8001;
                    someFlags = (someFlags&0xFF00) | 3;
                    continue;
                }
            }
            someFlags = (someFlags&0xFF00);
        }
        //Line 105
        if ((someFlags & 0xFF) < 2){
            if ((renderFlag->blending_mode == 0) && (texDef->textureCount == 2) && ((lowerBits == 4) || (lowerBits == 6))){
                if ( (*m2File->tex_unit_lookup_table[texDef->textureCoordComboIndex] == 0) &&  (*m2File->tex_unit_lookup_table[texDef->textureCoordComboIndex+1] > 2)) {
                    someFlags = (someFlags&0xFF00) | 1;
                }
            }
        }
        //Line 114
        if ((someFlags >> 8) != 0) {
            if ((someFlags >> 8) == 1) {
                //Line 119
                uint16_t blend = renderFlag->blending_mode;
                if ((blend != 4) && (blend != 6) || (texDef->textureCount != 1) || (*m2File->tex_unit_lookup_table[texDef->textureCoordComboIndex] <= 2)) {

                } else if (m2File->transparency_lookup_table[lowerLayerSkin->textureWeightComboIndex] == m2File->transparency_lookup_table[texDef->textureWeightComboIndex]) {
                    //Line 124
                    texDef->shader_id = 0x8000;
                    lowerLayerSkin->shader_id = renderFlag->blending_mode != 4 ? 14 : 0x8002;
                    //lowerLayerSkin.op_count = 2;
                    //TODO: Implement packing of textures

                    someFlags = (someFlags & 0xFF) | (2 << 8);
                    continue;
                }
            } else {
                //Line 140
                if ((someFlags >> 8) != 2) {
                    continue;
                }
                uint16_t blend = renderFlag->blending_mode;

                if ((blend != 2) && (blend != 1)
                    || (texDef->textureCount != 1)
                    || ((((renderFlag->flags & 0xff) ^ (m2File->materials[lowerLayerSkin->materialIndex]->flags & 0xff)) & 1) == 0)
                    || ((texDef->textureComboIndex & 0xff) != (lowerLayerSkin->textureComboIndex & 0xff))) {

                } else  if (m2File->transparency_lookup_table[lowerLayerSkin->textureWeightComboIndex] == m2File->transparency_lookup_table[texDef->textureWeightComboIndex]) {
                    texDef->shader_id = 0x8000;
                    lowerLayerSkin->shader_id = ((lowerLayerSkin->shader_id == 0x8002? 2 : 0) - 0x7FFF) & 0xFFFF;
                    someFlags = (someFlags & 0xFF) | (3 << 8);
                    continue;
                }
            }
            someFlags = (someFlags & 0xFF);
        }
        if ( (renderFlag->blending_mode == 0) && (texDef->textureCount == 1) && (m2File->transparency_lookup_table[texDef->textureCoordComboIndex] == 0)) {
            someFlags = (someFlags & 0xFF) | (1 << 8);
        }
    }

    if (reducingIsNeeded) {
        int prevRenderFlagIndex = -1;
        for (int i = 0; i < skinFileData->batches.size; i++) {
            M2Batch *texDef = skinFileData->batches[i];
            int renderFlagIndex = texDef->materialIndex;
            if (renderFlagIndex == prevRenderFlagIndex) {
                texDef->shader_id =                 skinFileData->batches[i-1]->shader_id;
                texDef->textureCount =              skinFileData->batches[i-1]->textureCount;
                texDef->textureComboIndex =         skinFileData->batches[i-1]->textureComboIndex;
                texDef->textureCoordComboIndex =    skinFileData->batches[i-1]->textureCoordComboIndex;
            } else {
                prevRenderFlagIndex = renderFlagIndex;
            }
        }
    }
    */
}

void SkinGeom::generateIndexBuffer(std::vector<uint16_t> &buffer) {
    int indiciesLength = this->m_skinData->indices.size;

    buffer = std::vector<uint16_t>(indiciesLength);

    for (int i = 0; i < indiciesLength; i++) {
        buffer[i] = *this->m_skinData->vertices.getElement(*this->m_skinData->indices.getElement(i));
    }
}

#ifndef WOWLIB_EXCLUDE_RENDERER
HGIndexBuffer SkinGeom::getIBO(const HMapSceneBufferCreate &renderer) {
    if (m_IBO == nullptr) {
        std::vector<uint16_t> indicies;
        generateIndexBuffer(indicies);

        m_IBO = renderer->createM2IndexBuffer(indicies.size() * sizeof(uint16_t));
        m_IBO->uploadData(
            &indicies[0],
            indicies.size() * sizeof(uint16_t));
    }

    return m_IBO;
}
#endif