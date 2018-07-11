//
// Created by deamon on 19.06.17.
//

#include "m2Geom.h"
#include "skinGeom.h"
#include "../shader/ShaderDefinitions.h"
#include "../opengl/header.h"

chunkDef<M2Geom> M2Geom::m2FileTable = {
    [](M2Geom& file, ChunkData& chunkData){},
    {
        {
            '12DM',
                {
                    [](M2Geom &file, ChunkData &chunkData) {
                        debuglog("Entered MD21");

//                                    chunkData.currentOffset = chunkData.currentOffset - 8;
                        chunkData.readValue(file.m_m2Data);

                        chunkData.bytesRead = chunkData.chunkLen;
                    }
                }
        },
        {
                'DIFS',
                {[](M2Geom &file, ChunkData &chunkData) {
                        debuglog("Entered SFID");
                        file.skinFileDataIDs =
                                std::vector<uint32_t>(
                                        file.m_m2Data->num_skin_profiles);

                        for (int i = 0; i < file.skinFileDataIDs.size(); i++) {
                            chunkData.readValue(file.skinFileDataIDs[i]);
                        }
                    }
                }
        },
        {
                'DIXT',
                {
                        [](M2Geom &file, ChunkData &chunkData) {
                            debuglog("Entered DIXT");
                            file.textureFileDataIDs =
                                    std::vector<uint32_t>(
                                            (unsigned long) (chunkData.chunkLen / 4));

                            for (int i = 0; i < file.textureFileDataIDs.size(); i++) {
                                chunkData.readValue(file.textureFileDataIDs[i]);
                            }
                        }
                }
        }
    }
};

void initM2Textures(M2Data *m2Header, void *m2File){
    int32_t texturesSize = m2Header->textures.size;
    for (int i = 0; i < texturesSize; i++) {
        M2Texture *texture = m2Header->textures.getElement(i);
        texture->filename.initM2Array(m2Header);
    }
}
void initCompBones(M2Data *m2Header, void *m2File){
    int32_t bonesSize = m2Header->bones.size;
    for (int i = 0; i < bonesSize; i++) {
        M2CompBone *compBone = m2Header->bones.getElement(i);
        compBone->translation.initTrack(m2Header);
        compBone->rotation.initTrack(m2Header);
        compBone->scaling.initTrack(m2Header);
    }
}
void initM2Color(M2Data *m2Header, void *m2File) {
    int32_t m2ColorSize = m2Header->colors.size;
    for (int i = 0; i < m2ColorSize; i++) {
        M2Color * m2Color = m2Header->colors.getElement(i);
        m2Color->alpha.initTrack(m2Header);
        m2Color->color.initTrack(m2Header);
    }
}
void initM2TextureWeight(M2Data *m2Header, void *m2File) {
    int32_t textureWeightSize = m2Header->texture_weights.size;
    for (int i = 0; i < textureWeightSize; i++) {
        M2TextureWeight * textureWeight = m2Header->texture_weights.getElement(i);
        textureWeight->weight.initTrack(m2Header);
    }
}
void initM2TextureTransform(M2Data *m2Header, void *m2File) {
    int32_t textureTransformSize = m2Header->texture_transforms.size;
    for (int i = 0; i < textureTransformSize; i++) {
        M2TextureTransform * textureTransform = m2Header->texture_transforms.getElement(i);
        textureTransform->translation.initTrack(m2Header);
        textureTransform->rotation.initTrack(m2Header);
        textureTransform->scaling.initTrack(m2Header);
    }
}
void initM2Attachment(M2Data *m2Header, void *m2File) {
    int32_t attachmentCount = m2Header->attachments.size;
    for (int i = 0; i < attachmentCount; i++) {
        M2Attachment *attachment = m2Header->attachments.getElement(i);
        attachment->animate_attached.initTrack(m2Header);
    }
}
void initM2Event(M2Data *m2Header, void *m2File) {
    int32_t eventCount = m2Header->events.size;
    for (int i = 0; i < eventCount; i++) {
        M2Event *event = m2Header->events.getElement(i);
        event->enabled.initTrackBase(m2Header);
    }
}
void initM2Light(M2Data *m2Header, void *m2File) {
    int32_t lightCount = m2Header->lights.size;
    for (int i = 0; i < lightCount; i++) {
        M2Light *light = m2Header->lights.getElement(i);
        light->ambient_color.initTrack(m2Header);
        light->ambient_intensity.initTrack(m2Header);
        light->diffuse_color.initTrack(m2Header);
        light->diffuse_intensity.initTrack(m2Header);
        light->attenuation_start.initTrack(m2Header);
        light->attenuation_end.initTrack(m2Header);
        light->visibility.initTrack(m2Header);
    }
}

void initM2Particle(M2Data *m2Header, void *m2File) {
    assert(sizeof(M2Particle) == 492);
    int32_t particleEmitterCount = m2Header->particle_emitters.size;
    for (int i = 0; i < particleEmitterCount; i++) {
        M2Particle *particleEmitter = m2Header->particle_emitters.getElement(i);

        particleEmitter->old.geometry_model_filename.initM2Array(m2Header);
        particleEmitter->old.recursion_model_filename.initM2Array(m2Header);

        particleEmitter->old.emissionSpeed.initTrack(m2Header);
        particleEmitter->old.speedVariation.initTrack(m2Header);
        particleEmitter->old.verticalRange.initTrack(m2Header);
        particleEmitter->old.horizontalRange.initTrack(m2Header);
        particleEmitter->old.gravity.initTrack(m2Header);
        particleEmitter->old.lifespan.initTrack(m2Header);
        particleEmitter->old.emissionRate.initTrack(m2Header);
        particleEmitter->old.emissionAreaLength.initTrack(m2Header);
        particleEmitter->old.emissionAreaWidth.initTrack(m2Header);
        particleEmitter->old.zSource.initTrack(m2Header);
        particleEmitter->old.alphaTrack.initPartTrack(m2Header);
        particleEmitter->old.colorTrack.initPartTrack(m2Header);
        particleEmitter->old.scaleTrack.initPartTrack(m2Header);
        particleEmitter->old.headCellTrack.initPartTrack(m2Header);
        particleEmitter->old.tailCellTrack.initPartTrack(m2Header);

        particleEmitter->old.splinePoints.initM2Array(m2Header);

        particleEmitter->old.enabledIn.initTrack(m2Header);
    }
}

void initM2Camera(M2Data *m2Header, void *m2File) {
    int32_t cameraCount = m2Header->cameras.size;
    for (int i = 0; i < cameraCount; i++) {
        M2Camera *camera = m2Header->cameras.getElement(i);
        camera->positions.initTrack(m2Header);
        camera->target_position.initTrack(m2Header);
        camera->roll.initTrack(m2Header);
        camera->FoV.initTrack(m2Header);
    }
}

void M2Geom::process(std::vector<unsigned char> &m2File, std::string &fileName) {
    this->m2File = m2File;

    if (
        m2File[0] == 'M' &&
        m2File[1] == 'D' &&
        m2File[2] == '2' &&
        m2File[3] == '1'

            ) {
        CChunkFileReader reader(this->m2File);
        reader.processFile(*this, &M2Geom::m2FileTable);
    } else {
        M2Data *m2Header = (M2Data *) &this->m2File[0];
        this->m_m2Data = m2Header;
    }
    M2Data *m2Header = this->m_m2Data;
    void *m2FileP = &this->m2File[0];

    //Step 1: Init all m2Arrays
    m2Header->global_loops.initM2Array(m2Header);
    m2Header->sequences.initM2Array(m2Header);
    m2Header->sequence_lookups.initM2Array(m2Header);
    m2Header->bones.initM2Array(m2Header);
    m2Header->key_bone_lookup.initM2Array(m2Header);
    m2Header->vertices.initM2Array(m2Header);
    m2Header->colors.initM2Array(m2Header);
    m2Header->textures.initM2Array(m2Header);
    m2Header->texture_weights.initM2Array(m2Header);
    m2Header->texture_transforms.initM2Array(m2Header);
    m2Header->replacable_texture_lookup.initM2Array(m2Header);
    m2Header->materials.initM2Array(m2Header);
    m2Header->bone_lookup_table.initM2Array(m2Header);
    m2Header->texture_lookup_table.initM2Array(m2Header);
    m2Header->tex_unit_lookup_table.initM2Array(m2Header);
    m2Header->transparency_lookup_table.initM2Array(m2Header);
    m2Header->texture_transforms_lookup_table.initM2Array(m2Header);
    m2Header->collision_triangles.initM2Array(m2Header);
    m2Header->collision_vertices.initM2Array(m2Header);
    m2Header->collision_normals.initM2Array(m2Header);
    m2Header->attachments.initM2Array(m2Header);
    m2Header->attachment_lookup_table.initM2Array(m2Header);
    m2Header->events.initM2Array(m2Header);
    m2Header->lights.initM2Array(m2Header);
    m2Header->cameras.initM2Array(m2Header);
    m2Header->camera_lookup_table.initM2Array(m2Header);
    m2Header->ribbon_emitters.initM2Array(m2Header);
    m2Header->particle_emitters.initM2Array(m2Header);

    if (m2Header->global_flags.flag_has_blend_maps) {
        m2Header->blend_map_overrides.initM2Array(m2FileP);
    }

    //Step 2: init tracks
    initCompBones(m2Header, m2FileP);
    initM2Color(m2Header, m2FileP);
    initM2TextureWeight(m2Header, m2FileP);
    initM2TextureTransform(m2Header, m2FileP);
    initM2Attachment(m2Header, m2FileP);
    initM2Event(m2Header, m2FileP);
    initM2Light(m2Header, m2FileP);
    initM2Particle(m2Header, m2FileP);
    initM2Camera(m2Header, m2FileP); //TODO: off for now

    initM2Textures(m2Header, m2FileP);

    m_loaded = true;
}

void
M2Geom::setupPlacementAttribute(GLuint placementVBO) {
        glBindBuffer(GL_ARRAY_BUFFER, placementVBO);

        //"Official" way to pass mat4 to shader as attribute
        glVertexAttribPointer(+m2Shader::Attribute::aPlacementMat + 0, 4, GL_FLOAT, GL_FALSE, 20 * 4, 0);  // position
        glVertexAttribPointer(+m2Shader::Attribute::aPlacementMat + 1, 4, GL_FLOAT, GL_FALSE, 20 * 4, (void*)16);  // position
        glVertexAttribPointer(+m2Shader::Attribute::aPlacementMat + 2, 4, GL_FLOAT, GL_FALSE, 20 * 4, (void*)32);  // position
        glVertexAttribPointer(+m2Shader::Attribute::aPlacementMat + 3, 4, GL_FLOAT, GL_FALSE, 20 * 4, (void*)48);  // position
//        glVertexAttribPointer(+m2Shader::Attribute::aAmbientColor, 4, GL_FLOAT, GL_FALSE, 20 * 4, (void*)64); //Diffuse color

}

void
M2Geom::setupUniforms(
        IWoWInnerApi *api,
        mathfu::mat4 &placementMatrix,
        std::vector<mathfu::mat4> &boneMatrices,
        mathfu::vec4 &diffuseColor,
        mathfu::vec4 &ambientColor,
        bool drawTransparent,
        std::vector<M2LightResult> &lights,
        bool instanced) {
    /*
    ShaderRuntimeData *m2Shader;
    if (!instanced) {
        m2Shader = api->getM2Shader();
    } else {
        m2Shader = api->getM2InstancingShader();
    }

    if (!instanced) {
        glUniformMatrix4fv(m2Shader->getUnf("uPlacementMat"), 1, GL_FALSE, &placementMatrix[0]);
    }
    glUniformMatrix4fv(m2Shader->getUnf("uBoneMatrixes[0]"), boneMatrices.size(), GL_FALSE, &boneMatrices[0][0]);


    //Set proper color
//    if (diffuseColor) {
    if (!instanced) {
        glUniform3fv(m2Shader->getUnf("uSunColor"), 1, &diffuseColor[0]);
        glUniform4fv(m2Shader->getUnf("uAmbientLight"), 1, &ambientColor[0]);
    }


    //Setup lights

    mathfu::mat4 viewModelMat = api->getViewMat()*placementMatrix;

    int index = 0;
    float indet[4] = {0,0,0,0};
    static const size_t pcLightNames[4][3] = {
            {
                CalculateFNV("pc_lights[0].color"),
                CalculateFNV("pc_lights[0].attenuation"),
                CalculateFNV("pc_lights[0].position")
            },
            {
                CalculateFNV("pc_lights[1].color"),
                CalculateFNV("pc_lights[1].attenuation"),
                CalculateFNV("pc_lights[1].position")
            },
            {
                CalculateFNV("pc_lights[2].color"),
                CalculateFNV("pc_lights[2].attenuation"),
                CalculateFNV("pc_lights[2].position")
            },
            {
                CalculateFNV("pc_lights[3].color"),
                CalculateFNV("pc_lights[3].attenuation"),
                CalculateFNV("pc_lights[3].position")
            }
    };

    bool BCLoginScreenHack = api->getConfig()->getBCLightHack();
    for (int i = 0; i < (int)lights.size() && index < 4; i++) {
        std::string uniformName;
        mathfu::vec4 attenVec;
        if (BCLoginScreenHack) {
            attenVec = mathfu::vec4(lights[i].attenuation_start, 1.0, lights[i].attenuation_end, lights.size());
        } else {
//            if ((lights[i].attenuation_end - lights[i].attenuation_start < 0.1)) continue;
//            attenVec = mathfu::vec4(lights[i].attenuation_start, 1.0, lights[i].attenuation_end, lights.size());
            attenVec = mathfu::vec4(lights[i].attenuation_start, lights[i].diffuse_intensity, lights[i].attenuation_end, lights.size());
        }

        glUniform4fv(m2Shader->getUnfHash(pcLightNames[index][0]), 1, &lights[i].diffuse_color.data_[0]);
        glUniform4fv(m2Shader->getUnfHash(pcLightNames[index][1]), 1, &attenVec.data_[0]);

        mathfu::vec4 viewPos = viewModelMat * lights[i].position;
        glUniform4fv(m2Shader->getUnfHash(pcLightNames[index][2]), 1, &viewPos.data_[0]);
        index++;
    }
    glUniform1i(m2Shader->getUnf("uLightCount"), index);
    for (int i = index; i < 4; i++) {
        std::string uniformName;
        glUniform4fv(m2Shader->getUnfHash(pcLightNames[index][0]), 1, indet);
        glUniform4fv(m2Shader->getUnfHash(pcLightNames[index][1]), 1, indet);
        glUniform4fv(m2Shader->getUnfHash(pcLightNames[index][2]), 1, indet);
        index++;
    }
     */
}

void
M2Geom::drawMesh(
        IWoWInnerApi *api,
        M2MaterialInst &materialData, M2SkinProfile &skinData, mathfu::vec4 &meshColor, float transparency,
        mathfu::mat4 &textureMatrix1, mathfu::mat4 &textureMatrix2,
        int vertexShaderIndex,
        int pixelShaderIndex,
        mathfu::vec4 &originalFogColor, int instanceCount) {

   /* ShaderRuntimeData *m2Shader;
    if (instanceCount == -1) {
        m2Shader = api->getM2Shader();
    } else {
        m2Shader = api->getM2InstancingShader();
    }
    GLuint blackPixelText = api->getBlackPixelTexture();
    bool fogChanged = false;

    glUniformMatrix4fv(m2Shader->getUnf("uTextMat1"), 1, GL_FALSE, &textureMatrix1[0]);
    glUniformMatrix4fv(m2Shader->getUnf("uTextMat2"), 1, GL_FALSE, &textureMatrix2[0]);

    glUniform4fv(m2Shader->getUnf("uColor"), 1, &meshColor[0]);
    glUniform1i(m2Shader->getUnf("uVertexShader"), vertexShaderIndex);
    glUniform1i(m2Shader->getUnf("uPixelShader"), pixelShaderIndex);

    static float fog_zero[3] = {0,0,0};
    static float fog_half[3] = {0.5,0.5,0.5};
    static float fog_one[3] = {1.0,1.0,1.0};

    if (materialData.isRendered) {
        if (materialData.texUnit1Texture != nullptr && materialData.texUnit1Texture->getIsLoaded()) {
            //try {
            glDepthMask(true);

            auto textMaterial = skinData.batches[materialData.texUnit1TexIndex];
            int renderFlagIndex = textMaterial->materialIndex;
            auto renderFlag = m_m2Data->materials[renderFlagIndex];

            float finalTransparency = meshColor.w;
            if ( textMaterial->textureCount && !(textMaterial->flags & 0x40)) {
                finalTransparency *= transparency;
            }
            glUniform1f(m2Shader->getUnf("uTransparency"), finalTransparency);

//            glUniform1i(m2Shader->getUnf("uBlendMode"), renderFlag->blending_mode);
            auto blendMode = renderFlag->blending_mode;
//            blendMode = 0;
            if (blendMode >= (uint16_t)EGxBlendEnum::GxBlend_Alpha) {
                glEnable(GL_BLEND);
            } else {
                glDisable(GL_BLEND);
            }

            if (blendMode == (uint16_t)EGxBlendEnum::GxBlend_AlphaKey) {
                glUniform1f(m2Shader->getUnf("uAlphaTest"), 128.0f/255.0f * finalTransparency);
            } else {
                glUniform1f(m2Shader->getUnf("uAlphaTest"), 1.0f/255.0f);
            }

            BlendModeDesc &selectedBlendMode = blendModes[M2BlendingModeToEGxBlendEnum[blendMode]];
            glBlendFuncSeparate(
                selectedBlendMode.SrcColor,
                selectedBlendMode.DestColor,
                selectedBlendMode.SrcAlpha,
                selectedBlendMode.DestAlpha
            );


            switch (blendMode) {
                case 0 : //Blend_Opaque
                    glUniform1f(m2Shader->getUnf("uAlphaTest"), -1.0f);
                    break;
                case 1 : //Blend_AlphaKey
                    break;
                case 2 : //Blend_Alpha
                    break;
                case 3 : //Blend_NoAlphaAdd
                    //Override fog
                    glUniform3fv(m2Shader->getUnf("uFogColor"), 1, fog_zero);
                    fogChanged = true;

                    break;
                case 4 : //Blend_Add
                    glUniform3fv(m2Shader->getUnf("uFogColor"),  1, fog_zero);
                    fogChanged = true;
                    break;

                case 5: //Blend_Mod
                    glUniform3fv(m2Shader->getUnf("uFogColor"), 1, fog_one);
                    fogChanged = true;
                    break;

                case 6: //Blend_Mod2x
                    glUniform3fv(m2Shader->getUnf("uFogColor"), 1, fog_half);
                    fogChanged = true;
                    break;

                case 7: //Blend_Mod2x
                    glUniform3fv(m2Shader->getUnf("uFogColor"), 1, fog_half);
                    fogChanged = true;
                    break;
                default :
                    debuglog("Unknown blending mode in M2 file")
                    break;
            }

            if (((renderFlag->flags & 0x1) > 0)) {
                glUniform1i(m2Shader->getUnf("uIsAffectedByLight"), 0);
            } else {
                glUniform1i(m2Shader->getUnf("uIsAffectedByLight"), 1);
            }
            if ((renderFlag->flags & 0x2) > 0 ) {
                glUniform1i(m2Shader->getUnf("uUnFogged"), 1);
            } else {
                glUniform1i(m2Shader->getUnf("uUnFogged"), 0);
            }

            if ((renderFlag->flags & 0x4) > 0) {
                glDisable(GL_CULL_FACE);
            } else {
                glEnable(GL_CULL_FACE);
            }

            if ((renderFlag->flags & 0x8) > 0) {
                glDisable(GL_DEPTH_TEST);
            } else {
                glEnable(GL_DEPTH_TEST);
            }


            if ((renderFlag->flags & 0x10) > 0) {
                //GL_disable(GL_DEPTH_TEST);
                glDepthMask(GL_FALSE);
            } else {
                // GL_enable(GL_DEPTH_TEST);
                glDepthMask(GL_TRUE);
            }

//            if (materialData.isEnviromentMapping) {
//                glUniform1i(m2Shader->getUnf("isEnviroment"), 1);
//            } else {
//                glUniform1i(m2Shader->getUnf("isEnviroment"), 0);
//            }

            //Set up texture animation

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, materialData.texUnit1Texture->getGlTexture());
            if (materialData.xWrapTex1) {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            } else {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            }
            if (materialData.yWrapTex1) {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            } else {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            }
            if (materialData.texUnit2Texture != nullptr) {
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, materialData.texUnit2Texture->getGlTexture());

                if (materialData.xWrapTex2) {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                } else {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                }
                if (materialData.yWrapTex2) {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                } else {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                }
            } else {
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, blackPixelText);
            }

            auto meshIndex = materialData.meshIndex;
            auto mesh = skinData.submeshes[meshIndex];
            if (instanceCount == -1) {
                //var error = gl.getError(); // Drop error flag
                glDrawElements(GL_TRIANGLES, mesh->indexCount,
                               GL_UNSIGNED_SHORT,
                               (const void *) (intptr_t)((mesh->indexStart + (mesh->Level << 16)) * 2));
            } else {
#ifndef WITH_GLESv2
                glDrawElementsInstanced(GL_TRIANGLES, skinData.submeshes[meshIndex]->indexCount,
                                        GL_UNSIGNED_SHORT,
                                        (const void *) (intptr_t)((mesh->indexStart + (mesh->Level << 16)) * 2),
                                        instanceCount);
#endif
            }
            if (materialData.texUnit2Texture != nullptr) {
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, 0);
                glActiveTexture(GL_TEXTURE0);
            }

            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND);

            if (fogChanged) {
                glUniform3fv(m2Shader->getUnf("uFogColor"), 1, &originalFogColor[0]);
            }
        }
    }
    */
}

HGVertexBuffer M2Geom::getVBO(GDevice &device) {
    if (vertexVbo.get() == nullptr) {
        vertexVbo = device.createVertexBuffer();
        vertexVbo->uploadData(
            m_m2Data->vertices.getElement(0),
            m_m2Data->vertices.size*sizeof(M2Vertex));
    }

    return vertexVbo;
}
