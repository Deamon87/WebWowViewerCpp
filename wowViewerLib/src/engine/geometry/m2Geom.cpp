//
// Created by deamon on 19.06.17.
//

#include "m2Geom.h"
#include "skinGeom.h"
#include "../shaderDefinitions.h"
#include "../opengl/header.h"

void M2Geom::loadTextures() {

}

void M2Geom::createVBO() {
    glGenBuffers(1, &this->vertexVbo);
    glBindBuffer(GL_ARRAY_BUFFER, this->vertexVbo);
    glBufferData(GL_ARRAY_BUFFER, m_m2Data->vertices.size*sizeof(M2Vertex), m_m2Data->vertices.getElement(0), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void M2Geom::setupAttributes() {
    glBindBuffer(GL_ARRAY_BUFFER, this->vertexVbo);
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skinObject.indexVBO);
    //gl.vertexAttrib4f(shaderAttributes.aColor, 0.5, 0.5, 0.5, 0.5);

    /*
     {name: "pos",           type : "vector3f"},           0+12 = 12
     {name: "bonesWeight",   type : "uint8Array", len: 4}, 12+4 = 16
     {name: "bones",         type : "uint8Array", len: 4}, 16+4 = 20
     {name: "normal",        type : "vector3f"},           20+12 = 32
     {name: "textureX",      type : "float32"},            32+4 = 36
     {name: "textureY",      type : "float32"},            36+4 = 40
     {name : "textureX2",    type : "float32"},            40+4 = 44
     {name : "textureY2",    type : "float32"}             44+4 = 48
     */

    glVertexAttribPointer(+m2Shader::Attribute::aPosition, 3, GL_FLOAT, false, 48, (void *)0);  // position
    glVertexAttribPointer(+m2Shader::Attribute::boneWeights, 4, GL_UNSIGNED_BYTE, true, 48, (void *)12);  // bonesWeight
    glVertexAttribPointer(+m2Shader::Attribute::bones, 4, GL_UNSIGNED_BYTE, false, 48, (void *)16);  // bones
    glVertexAttribPointer(+m2Shader::Attribute::aNormal, 3, GL_FLOAT, false, 48, (void *)20); // normal
    glVertexAttribPointer(+m2Shader::Attribute::aTexCoord, 2, GL_FLOAT, false, 48, (void *)32); // texcoord
    glVertexAttribPointer(+m2Shader::Attribute::aTexCoord2, 2, GL_FLOAT, false, 48, (void *)40); // texcoord

}


void initM2Textures(M2Data *m2Header){
    int32_t texturesSize = m2Header->textures.size;
    for (int i = 0; i < texturesSize; i++) {
        M2Texture *texture = m2Header->textures.getElement(i);
        texture->filename.initM2Array(m2Header);
    }
}
void initCompBones(M2Data *m2Header){
    int32_t bonesSize = m2Header->bones.size;
    for (int i = 0; i < bonesSize; i++) {
        M2CompBone *compBone = m2Header->bones.getElement(i);
        compBone->translation.initTrack(m2Header);
        compBone->rotation.initTrack(m2Header);
        compBone->scaling.initTrack(m2Header);
    }
}
void initM2Color(M2Data *m2Header) {
    int32_t m2ColorSize = m2Header->colors.size;
    for (int i = 0; i < m2ColorSize; i++) {
        M2Color * m2Color = m2Header->colors.getElement(i);
        m2Color->alpha.initTrack(m2Header);
        m2Color->color.initTrack(m2Header);
    }
}
void initM2TextureWeight(M2Data *m2Header) {
    int32_t textureWeightSize = m2Header->texture_weights.size;
    for (int i = 0; i < textureWeightSize; i++) {
        M2TextureWeight * textureWeight = m2Header->texture_weights.getElement(i);
        textureWeight->weight.initTrack(m2Header);
    }
}
void initM2TextureTransform(M2Data *m2Header) {
    int32_t textureTransformSize = m2Header->texture_transforms.size;
    for (int i = 0; i < textureTransformSize; i++) {
        M2TextureTransform * textureTransform = m2Header->texture_transforms.getElement(i);
        textureTransform->translation.initTrack(m2Header);
        textureTransform->rotation.initTrack(m2Header);
        textureTransform->scaling.initTrack(m2Header);
    }
}
void initM2Attachment(M2Data *m2Header) {
    int32_t attachmentCount = m2Header->attachments.size;
    for (int i = 0; i < attachmentCount; i++) {
        M2Attachment *attachment = m2Header->attachments.getElement(i);
        attachment->animate_attached.initTrack(m2Header);
    }
}
void initM2Event(M2Data *m2Header) {
    int32_t eventCount = m2Header->events.size;
    for (int i = 0; i < eventCount; i++) {
        M2Event *event = m2Header->events.getElement(i);
        event->enabled.initTrack(m2Header);
    }
}
void initM2Light(M2Data *m2Header) {
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
}void initM2Camera(M2Data *m2Header) {
    int32_t cameraCount = m2Header->cameras.size;
    for (int i = 0; i < cameraCount; i++) {
        M2Camera *light = m2Header->cameras.getElement(i);
        light->positions.initTrack(m2Header);
        light->target_position.initTrack(m2Header);
        light->roll.initTrack(m2Header);
    }
}

void M2Geom::process(std::vector<unsigned char> &m2File) {
    this->m2File = m2File;

    M2Data *m2Header = (M2Data *) &this->m2File[0];
    this->m_m2Data = m2Header;

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
        m2Header->blend_map_overrides.initM2Array(m2Header);
    }

    //Step 2: init tracks
    initCompBones(m2Header);
    initM2Color(m2Header);
    initM2TextureWeight(m2Header);
    initM2TextureTransform(m2Header);
    initM2Attachment(m2Header);
    initM2Event(m2Header);
    initM2Light(m2Header);
    initM2Camera(m2Header);

    initM2Textures(m2Header);

    //Step 3: create VBO
    this->createVBO();

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
        glVertexAttribPointer(+m2Shader::Attribute::aDiffuseColor, 4, GL_FLOAT, GL_FALSE, 20 * 4, (void*)64); //Diffuse color

}

void
M2Geom::setupUniforms(
        IWoWInnerApi *api,
        mathfu::mat4 &placementMatrix,
        std::vector<mathfu::mat4> &boneMatrices,
        mathfu::vec4 &diffuseColor,
        bool drawTransparent,
        bool instanced) {
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
        glUniform4fv(m2Shader->getUnf("uDiffuseColor"), 1, &diffuseColor[0]);
    }
//    }
//    if (drawTransparent) {
//        glUniform1i(m2Shader->getUnf("isTransparent"), 1);
//    } else {
//        glUniform1i(m2Shader->getUnf("isTransparent"), 0);
//    }

    //Setup lights

//    ?var activeLights = (lights) ? (lights.length | 0) : 0;
    /*
    for (var i = 0; i < lights.length; i++) {
        if (lights[i].attenuation_end - lights[i].attenuation_start > 0.01) {
            activeLights++;
        }
    }*/
//    gl.uniform1i(m2Shader->getUnf("uLightCount, activeLights);
    glUniform1i(m2Shader->getUnf("uLightCount"), 0);
    int index = 0;
//    for (var i = 0; i < lights.length; i++) {
//        //if (lights[i].attenuation_end - lights[i].attenuation_start <= 0.01) continue;
//        gl.uniform4fv(uniforms["pc_lights["+index+"].color"], new Float32Array(lights[i].diffuse_color));
//        gl.uniform4fv(uniforms["pc_lights["+index+"].attenuation"], new Float32Array([lights[i].attenuation_start, lights[i].diffuse_intensity, lights[i].attenuation_end, activeLights]));
//        gl.uniform4fv(uniforms["pc_lights["+index+"].position"], new Float32Array(lights[i].position));
//        index++;
//    }
    float indet[4] = {0,0,0,0};
    for (int i = index; i < 3; i++) {
        std::string uniformName;
        uniformName = std::string("pc_lights[")+std::to_string(index)+std::string("].color");
        glUniform4fv(m2Shader->getUnf(uniformName), 1, indet);
        uniformName = std::string("pc_lights[")+std::to_string(index)+std::string("].attenuation");
        glUniform4fv(m2Shader->getUnf(uniformName), 1, indet);
        uniformName = std::string("pc_lights[")+std::to_string(index)+std::string("].position");
        glUniform4fv(m2Shader->getUnf(uniformName), 1, indet);
        index++;
    }

    bool diffuseFound = false;
//    for (int i = 0; i < lights.length; i++) {
//        if (lights[i].ambient_color) {
//            if (lights[i].ambient_color[0] != 0 && lights[i].ambient_color[1] != 0 && lights[i].ambient_color[2] != 0) {
//                gl.uniform4fv(m2Shader->getUnf("uPcColor, new Float32Array(lights[i].ambient_color));
//                diffuseFound = true;
//                break;
//            }
//        }
//    }

//    if (!diffuseFound) {

    float pcColor[4];
//    glUniform4fv(m2Shader->getUnf("uPcColor"), 1, pcColor);
//    }
}

void
M2Geom::drawMesh(
        IWoWInnerApi *api,
        M2MaterialInst &materialData, M2SkinProfile &skinData, mathfu::vec4 &meshColor, float transparency,
        mathfu::mat4 &textureMatrix1, mathfu::mat4 &textureMatrix2, int pixelShaderIndex,
        mathfu::vec4 &originalFogColor, int instanceCount) {

    ShaderRuntimeData *m2Shader;
    if (instanceCount == -1) {
        m2Shader = api->getM2Shader();
    } else {
        m2Shader = api->getM2InstancingShader();
    }
    GLuint blackPixelText = api->getBlackPixelTexture();
    //var blackPixelText = this.sceneApi.getBlackPixelTexture();

    //materialData = new M2Material();
    bool fogChanged = false;

    glUniformMatrix4fv(m2Shader->getUnf("uTextMat1"), 1, GL_FALSE, &textureMatrix1[0]);
    glUniformMatrix4fv(m2Shader->getUnf("uTextMat2"), 1, GL_FALSE, &textureMatrix2[0]);

    glUniform4fv(m2Shader->getUnf("uColor"), 1, &meshColor[0]);
    glUniform1f(m2Shader->getUnf("uTransparency"), transparency);
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

//            glUniform1i(m2Shader->getUnf("uBlendMode"), renderFlag->blending_mode);
            switch (renderFlag->blending_mode) {
                case 0 : //Blend_Opaque
                    glDisable(GL_BLEND);
                    glUniform1f(m2Shader->getUnf("uAlphaTest"), -1.0);
                    break;
                case 1 : //Blend_AlphaKey
                    glDisable(GL_BLEND);
                    //GL_uniform1f(m2Shader->getUnf("uAlphaTest, 2.9);
                    glUniform1f(m2Shader->getUnf("uAlphaTest"), 0.903921569);
                    //GL_uniform1f(m2Shader->getUnf("uAlphaTest, meshColor[4]*transparency*(252/255));
                    break;
                case 2 : //Blend_Alpha
                    glUniform1f(m2Shader->getUnf("uAlphaTest"), -1);
                    glEnable(GL_BLEND);
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // default blend func
                    break;
                case 3 : //Blend_NoAlphaAdd
                    glUniform1f(m2Shader->getUnf("uAlphaTest"), -1);
                    glEnable(GL_BLEND);
                    glBlendFunc(GL_ONE, GL_ONE);

                    //Override fog
                    glUniform3fv(m2Shader->getUnf("uFogColor"), 1, fog_zero);
                    fogChanged = true;

                    break;
                case 4 : //Blend_Add
                    glUniform1f(m2Shader->getUnf("uAlphaTest"), 0.00392157);
                    glEnable(GL_BLEND);
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

                    glUniform3fv(m2Shader->getUnf("uFogColor"),  1, fog_zero);
                    fogChanged = true;
                    break;

                case 5: //Blend_Mod
                    glUniform1f(m2Shader->getUnf("uAlphaTest"), 0.00392157);
                    glEnable(GL_BLEND);
                    glBlendFunc(GL_DST_COLOR, GL_ZERO);

                    glUniform3fv(m2Shader->getUnf("uFogColor"), 1, fog_one);
                    fogChanged = true;
                    break;

                case 6: //Blend_Mod2x
                    glUniform1f(m2Shader->getUnf("uAlphaTest"), 0.00392157);
                    glEnable(GL_BLEND);
                    glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);

                    glUniform3fv(m2Shader->getUnf("uFogColor"), 1, fog_half);
                    fogChanged = true;
                    break;
                default :
                    glUniform1f(m2Shader->getUnf("uAlphaTest"), -1);
                    glEnable(GL_BLEND);
                    glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);

                    break;
            }

            if (((renderFlag->flags & 0x1) > 0)|| (renderFlag->blending_mode == 5) || (renderFlag->blending_mode == 6)) {
                glUniform1i(m2Shader->getUnf("uUseDiffuseColor"), 0);
            } else {
                glUniform1i(m2Shader->getUnf("uUseDiffuseColor"), 1);
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
                //GL_uniform1i(m2Shader->getUnf("isBillboard, 1);
            }


            if ((renderFlag->flags & 0x10) > 0) {
                //GL_disable(GL_DEPTH_TEST);
                glDepthMask(GL_FALSE);
            } else {
                // GL_enable(GL_DEPTH_TEST);
                glDepthMask(GL_TRUE);
            }

            if (materialData.isEnviromentMapping) {
                glUniform1i(m2Shader->getUnf("isEnviroment"), 1);
            } else {
                glUniform1i(m2Shader->getUnf("isEnviroment"), 0);
            }

            /* Set up texture animation */
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
            if (instanceCount == -1) {
                //var error = gl.getError(); // Drop error flag
                glDrawElements(GL_TRIANGLES, skinData.submeshes[meshIndex]->indexCount,
                               GL_UNSIGNED_SHORT,
                               (const void *) (skinData.submeshes[meshIndex]->indexStart * 2));
            } else {
//                instExt.drawElementsInstancedANGLE(gl.TRIANGLES, skinData.subMeshes[meshIndex].nTriangles, gl.UNSIGNED_SHORT, skinData.subMeshes[meshIndex].StartTriangle * 2, instanceCount);
                glDrawElementsInstanced(GL_TRIANGLES, skinData.submeshes[meshIndex]->indexCount,
                                        GL_UNSIGNED_SHORT,
                                        (const void *)(skinData.submeshes[meshIndex]->indexStart * 2),
                                        instanceCount);
            }
            if (materialData.texUnit2Texture != nullptr) {
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, 0);
                glActiveTexture(GL_TEXTURE0);
            }

            /*
             if ((renderFlag.flags & 0x8) > 0) {
             gl.uniform1i(m2Shader->getUnf("isBillboard, 0);
             }
             */

            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND);

            if (fogChanged) {
                glUniform3fv(m2Shader->getUnf("uFogColor"), 1, &originalFogColor[0]);
            }
        }
    }
}



