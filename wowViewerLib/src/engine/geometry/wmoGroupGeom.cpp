//
// Created by deamon on 03.07.17.
//

#include "wmoGroupGeom.h"
#include "../persistance/header/wmoFileHeader.h"
#include "../shader/ShaderDefinitions.h"
#include <iostream>

chunkDef<WmoGroupGeom> WmoGroupGeom::wmoGroupTable = {
    [](WmoGroupGeom& object, ChunkData& chunkData){},
    {
        {
            'MVER',
            {
                [](WmoGroupGeom& object, ChunkData& chunkData){
                    debuglog("Entered MVER");
                }
            }
        },
        {
            'MOGP',
            {
                [](WmoGroupGeom& object, ChunkData& chunkData){
                    chunkData.readValue(object.mogp);
                    debuglog("Entered MOGP");
                },
                {
                    {
                        'MOPY', {
                            [](WmoGroupGeom& object, ChunkData& chunkData){
                                debuglog("Entered MOPY");
                                object.mopyLen = chunkData.chunkLen / sizeof(SMOPoly);
                                chunkData.readValues(object.mopy, object.mopyLen);
                            },
                        }
                    },
                    {
                        'MOVI', {
                            [](WmoGroupGeom& object, ChunkData& chunkData){
                                debuglog("Entered MOVI");
                                object.indicesLen = chunkData.chunkLen / 2;
                                chunkData.readValues(object.indicies, object.indicesLen);
                            },
                        }
                    },
                    {
                        'MOVT', {
                            [](WmoGroupGeom& object, ChunkData& chunkData){

                                object.verticesLen = chunkData.chunkLen / sizeof(C3Vector);
                                chunkData.readValues(object.verticles, object.verticesLen);
                                debuglog("Entered MOVT");
                            },
                        }
                    },
                    {
                        'MONR', {
                            [](WmoGroupGeom& object, ChunkData& chunkData){
                                object.normalsLen = chunkData.chunkLen / sizeof(C3Vector);
                                chunkData.readValues(object.normals, object.normalsLen);
                                debuglog("Entered MONR");
                            },
                        }
                    },
                    {
                        'MOTV', {
                            [](WmoGroupGeom& object, ChunkData& chunkData){
                                debuglog("Entered MOTV");

                                if (object.textureCoordsRead == 0) {
                                    object.textureCoordsLen = chunkData.chunkLen / sizeof(C2Vector);
                                    chunkData.readValues(object.textCoords,
                                                         object.textureCoordsLen);
                                } else if (object.textureCoordsRead == 1) {
                                    object.textureCoordsLen2 = chunkData.chunkLen / sizeof(C2Vector);
                                    chunkData.readValues(object.textCoords2,
                                                         object.textureCoordsLen2);
                                } else if (object.textureCoordsRead == 2) {
                                    object.textureCoordsLen3 = chunkData.chunkLen / sizeof(C2Vector);
                                    chunkData.readValues(object.textCoords3,
                                                         object.textureCoordsLen3);
                                }
                                object.textureCoordsRead++;
                            },
                        }
                    },
                    {
                        'MOCV', {
                            [](WmoGroupGeom& object, ChunkData& chunkData){
                                debuglog("Entered MOCV");
                                if (object.mocvRead == 0 && object.mogp->flags.hasVertexColors) {
                                    if (object.mogp->flags.hasVertexColors) {
                                        object.cvLen = chunkData.chunkLen / 4;
                                        chunkData.readValues(object.colorArray, object.cvLen);
                                    } else {
                                        object.mocvRead++;
                                    }
                                }

                                if (object.mocvRead == 1) {
                                    if (object.mogp->flags.CVERTS2) {
                                        object.cvLen2 = chunkData.chunkLen / 4;
                                        chunkData.readValues(object.colorArray2, object.cvLen2);
                                    }
                                }

                                object.mocvRead++;
                            },
                        }
                    },
                    {
                        'MODR', {
                            [](WmoGroupGeom& object, ChunkData& chunkData){
                                object.doodadRefsLen = chunkData.chunkLen / 2;
                                chunkData.readValues(object.doodadRefs, object.doodadRefsLen);
                                debuglog("Entered MODR");
                            },
                        }
                    },
                    {
                        'MOLR', {
                            [](WmoGroupGeom& object, ChunkData& chunkData){
                                debuglog("Entered MOLR");
                                object.lightRefListLen = chunkData.chunkLen / 2;
                                chunkData.readValues(object.lightRefList, object.lightRefListLen);
                            },
                        }
                    },
                    {
                        'MOBA', {
                            [](WmoGroupGeom& object, ChunkData& chunkData){
                                object.batchesLen = chunkData.chunkLen / sizeof(SMOBatch);
                                chunkData.readValues(object.batches, object.batchesLen);
                                debuglog("Entered MOBA");
                            },
                        }
                    },
                    {
                        'MOPB', {
                            [](WmoGroupGeom& object, ChunkData& chunkData){
                                debuglog("Entered MOPB");
                                object.prePassBatchesLen = chunkData.chunkLen / sizeof(SMOBatch);
                                chunkData.readValues(object.prePassbatches, object.prePassBatchesLen);
                            },
                        }
                    },
                    {
                        'MOBN', {
                            [](WmoGroupGeom& object, ChunkData& chunkData){
                                object.nodesLen = chunkData.chunkLen / sizeof(t_BSP_NODE);
                                chunkData.readValues(object.bsp_nodes, object.nodesLen);
                                debuglog("Entered MOBN");
                            },
                        }
                    },
                    {
                        'MOBR', {
                            [](WmoGroupGeom& object, ChunkData& chunkData){
                                object.bpsIndiciesLen = chunkData.chunkLen / sizeof(uint16_t);
                                chunkData.readValues(object.bpsIndicies, object.bpsIndiciesLen);
                                debuglog("Entered MOBR");
                            },
                        }
                    },
                    {
                        'MDAL', {
                            [](WmoGroupGeom& object, ChunkData& chunkData){
                                debuglog("Entered MDAL");
                                object.use_replacement_for_header_color = 1;
                                chunkData.readValue(object.replacement_for_header_color);

                            },
                        }
                    },
                    {
                        'MOLP', {
                            [](WmoGroupGeom& object, ChunkData& chunkData){
                                debuglog("Entered MOLP");
                                object.molpCnt = chunkData.chunkLen / sizeof(MOLP);
                                chunkData.readValues(object.molp, object.molpCnt);
                            },
                        }
                    },
                }
            }
        }
    }
};

void WmoGroupGeom::process(std::vector<unsigned char> &wmoGroupFile, std::string &fileName) {
    m_wmoGroupFile = wmoGroupFile;

    CChunkFileReader reader(m_wmoGroupFile);
    reader.processFile(*this, &WmoGroupGeom::wmoGroupTable);

    fixColorVertexAlpha(mohd);
    if (!mohd->flags.flag_attenuate_vertices_based_on_distance_to_portal) {
        this->m_attenuateFunc(*this);
    }

    m_loaded = true;
}


void WmoGroupGeom::fixColorVertexAlpha(SMOHeader *mohd) {
    int begin_second_fixup = 0;
    if (mogp->transBatchCount) {
        begin_second_fixup =
            *((uint16_t *) &batches[(uint16_t) mogp->transBatchCount] - 2) + 1;
    }

    unsigned char v35;
    unsigned char v36;
    unsigned char v37;

    if (mohd->flags.flag_lighten_interiors) {
        for (int i(begin_second_fixup); i < cvLen; ++i) {
            colorArray[i].a = (unsigned char) ((mogp->flags.EXTERIOR > 0) ? 0xFF : 0x00);
        }
    } else {
        if (!mohd->flags.flag_skip_base_color) {
            v35 = 0;
            v36 = 0;
            v37 = 0;
        } else {
//            if (use_replacement_for_header_color) {
//                v35 = replacement_for_header_color.b;
//                v37 = replacement_for_header_color.g;
//                v36 = replacement_for_header_color.r;
//            } else {
                v36 = mohd->ambColor.r;
                v37 = mohd->ambColor.g;
                v35 = mohd->ambColor.b;
//            }
        }

        for (int mocv_index(0); mocv_index < begin_second_fixup; ++mocv_index) {
            colorArray[mocv_index].r -= v36;
            colorArray[mocv_index].g -= v37;
            colorArray[mocv_index].b -= v35;

            float v38 = colorArray[mocv_index].a / 255.0f;

            float v11 = colorArray[mocv_index].r - v38 * colorArray[mocv_index].r;
            assert (v11 > -0.5f);
            assert (v11 < 255.5f);
            colorArray[mocv_index].r = (unsigned char) std::max(0.0f, floorf(v11 / 2.0f));
            float v13 = colorArray[mocv_index].g - v38 * colorArray[mocv_index].g;
            assert (v13 > -0.5f);
            assert (v13 < 255.5f);
            colorArray[mocv_index].g = (unsigned char) std::max(0.0f, floorf(v13 / 2.0f));
            float v14 = colorArray[mocv_index].b - v38 * colorArray[mocv_index].b;
            assert (v14 > -0.5f);
            assert (v14 < 255.5f);
            colorArray[mocv_index++].b = (unsigned char) std::max(0.0f, floorf(v14 / 2.0f));
        }

        for (int i(begin_second_fixup); i < cvLen; ++i) {
            float v19 = (colorArray[i].r * colorArray[i].a) / 64 + colorArray[i].r - v36;
            colorArray[i].r = (unsigned char) std::min(255.0f, std::max(v19 / 2.0f, 0.0f));

            float v30 = (colorArray[i].g * colorArray[i].a) / 64 + colorArray[i].g - v37;
            colorArray[i].g = (unsigned char) std::min(255.0f, std::max(v30 / 2.0f, 0.0f));

            float v33 = (colorArray[i].a * colorArray[i].b) / 64 + colorArray[i].b - v35;
            colorArray[i].b = (unsigned char) std::min(255.0f, std::max(v33 / 2.0f, 0.0f));

            colorArray[i].a = (unsigned char) ((mogp->flags.EXTERIOR ) > 0 ? 0xFF : 0x00);
        }
    }
}


HGVertexBuffer WmoGroupGeom::getVBO(GDevice &device) {
    if (combinedVBO == nullptr) {
        combinedVBO = device.createVertexBuffer();

        struct VboFormat {
            C3Vector pos;
            C3Vector normal;
            C2Vector textCoordinate;
            C2Vector textCoordinate2;
            C2Vector textCoordinate3;
            CImVector color;
            CImVector color2;
        };

        static const C2Vector c2ones = C2Vector(mathfu::vec2(1.0, 1.0));
        static const C3Vector c3zeros = C3Vector(mathfu::vec3(0, 0, 0));

        std::vector<VboFormat> buffer (verticesLen);
        for (int i = 0; i < verticesLen; i++) {
            VboFormat &format = buffer[i];
            format.pos = verticles[i];
            if (normalsLen > 0) {
                format.normal = normals[i];
            } else {
                format.normal = c3zeros;
            }
            if (textureCoordsLen > 0) {
                format.textCoordinate = textCoords[i];
            } else {
                format.textCoordinate = c2ones;
            }
            if (textureCoordsLen2 > 0) {
                format.textCoordinate2 = textCoords2[i];
            } else {
                format.textCoordinate2 = c2ones;
            }
            if (textureCoordsLen3 > 0) {
                format.textCoordinate3 = textCoords3[i];
            } else {
                format.textCoordinate3 = c2ones;
            }
            if (cvLen > 0) {
                format.color = colorArray[i];
            } else {
                *(int *) &format.color = 0;
            }
            if (cvLen2 > 0) {
                format.color2 = colorArray2[i];
            } else {
                *(int *) &format.color2 = 0x000000FF;
            }
        }

        combinedVBO->uploadData(&buffer[0], (int)(buffer.size() * sizeof(VboFormat)));
    }

    return combinedVBO;
}

HGIndexBuffer WmoGroupGeom::getIBO(GDevice &device) {
    if (indexVBO == nullptr) {
        indexVBO = device.createIndexBuffer();
        indexVBO->uploadData(
            &indicies[0],
            indicesLen * sizeof(uint16_t));
    }

    return indexVBO;
}

static GBufferBinding staticWMOBindings[7] = {
    {+wmoShader::Attribute::aPosition, 3, GL_FLOAT, false, 56, 0 },
    {+wmoShader::Attribute::aNormal, 3, GL_FLOAT, false, 56, 12},
    {+wmoShader::Attribute::aTexCoord, 2, GL_FLOAT, false, 56, 24},
    {+wmoShader::Attribute::aTexCoord2, 2, GL_FLOAT, false, 56, 32},
    {+wmoShader::Attribute::aTexCoord3, 2, GL_FLOAT, false, 56, 40},
    {+wmoShader::Attribute::aColor, 4, GL_UNSIGNED_BYTE, true, 56, 48},
    {+wmoShader::Attribute::aColor2, 4, GL_UNSIGNED_BYTE, true, 56, 52}
};

HGVertexBufferBindings WmoGroupGeom::getVertexBindings(GDevice &device) {
    if (vertexBufferBindings == nullptr) {
        vertexBufferBindings = device.createVertexBufferBindings();
        vertexBufferBindings->setIndexBuffer(getIBO(device));

        GVertexBufferBinding vertexBinding;
        vertexBinding.vertexBuffer = getVBO(device);

        vertexBinding.bindings = std::vector<GBufferBinding>(&staticWMOBindings[0], &staticWMOBindings[6]);

        vertexBufferBindings->addVertexBufferBinding(vertexBinding);
        vertexBufferBindings->save();
    }

    return vertexBufferBindings;
}

void WmoGroupGeom::draw(IWoWInnerApi *api, SMOMaterial const *materials, mathfu::vec4 &ambColor, std::function <HBlpTexture (int materialId, bool isSpec)> getTextureFunc) {
    /*
    GLuint blackPixelText = api->getBlackPixelTexture();
    Config * config = api->getConfig();

    bool isIndoor = (mogp->flags.INTERIOR) > 0;

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->indexVBO);
    glBindBuffer(GL_ARRAY_BUFFER, this->combinedVBO);

    glEnableVertexAttribArray(+wmoShader::Attribute::aPosition);
    glVertexAttribPointer(+wmoShader::Attribute::aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0); // position
    glVertexAttribPointer(+wmoShader::Attribute::aNormal, 3, GL_FLOAT, GL_FALSE, 0, (void *)normalOffset); // normal
    glVertexAttribPointer(+wmoShader::Attribute::aTexCoord, 2, GL_FLOAT, GL_FALSE, 0, (void *)textOffset); // texcoord


    if ((textureCoordsLen2 > 0)) {
        glEnableVertexAttribArray(+wmoShader::Attribute::aTexCoord2);
        glVertexAttribPointer(+wmoShader::Attribute::aTexCoord2, 2, GL_FLOAT, GL_FALSE, 0, (void *)textOffset2); // texcoord
    } else {
        glDisableVertexAttribArray(+wmoShader::Attribute::aTexCoord2);
        glVertexAttrib2f(+wmoShader::Attribute::aTexCoord2, 1.0, 1.0);
    }
    if ((textureCoordsLen3 > 0)) {
        glEnableVertexAttribArray(+wmoShader::Attribute::aTexCoord3);
        glVertexAttribPointer(+wmoShader::Attribute::aTexCoord3, 2, GL_FLOAT, GL_FALSE, 0, (void *)textOffset3); // texcoord
    } else {
        glDisableVertexAttribArray(+wmoShader::Attribute::aTexCoord3);
        glVertexAttrib2f(+wmoShader::Attribute::aTexCoord3, 1.0, 1.0);
    }

    auto wmoShader = api->getWmoShader();

    glActiveTexture(GL_TEXTURE0);
    glUniform1f(wmoShader->getUnf("uAlphaTest"), -1.0);

    int minBatch = config->getMinBatch();
    int maxBatch = std::min(config->getMaxBatch(), batchesLen);

    for (int j = minBatch; j < maxBatch; j++) {
        SMOBatch &renderBatch = batches[j];

        int texIndex;
        if (renderBatch.flag_use_material_id_large) {
            texIndex = renderBatch.postLegion.material_id_large;
        } else {
            texIndex = renderBatch.material_id;
        }

        const SMOMaterial &material = materials[texIndex];
        assert(material.shader < MAX_WMO_SHADERS && material.shader >= 0);
        auto shaderId = material.shader;
        if (shaderId >= MAX_WMO_SHADERS) {
            shaderId = 0;
        }
        int pixelShader = wmoMaterialShader[shaderId].pixelShader;
        int vertexShader = wmoMaterialShader[shaderId].vertexShader;

        glUniform1i(wmoShader->getUnf("uVertexShader"), vertexShader);
        glUniform1i(wmoShader->getUnf("uPixelShader"), pixelShader);

        bool isAffectedByMOCV = j < (mogp->transBatchCount + mogp->intBatchCount);
        isAffectedByMOCV = true;
        bool isAffectedByAmbient = (j >= 0 && j < (mogp->transBatchCount)) ||
                (j >= (mogp->transBatchCount + mogp->intBatchCount));
//        isAffectedByAmbient = true;

        mathfu::vec4 ambientColor = ambColor;
        if (isAffectedByAmbient) {
            ambientColor = api->getGlobalAmbientColor();
        }

        if (isAffectedByMOCV && (cvLen > 0)) {
            glEnableVertexAttribArray(+wmoShader::Attribute::aColor);
            glVertexAttribPointer(+wmoShader::Attribute::aColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, (void *)colorOffset); // color
        } else {
            glDisableVertexAttribArray(+wmoShader::Attribute::aColor);
            glVertexAttrib4f(+wmoShader::Attribute::aColor, 0.0, 0.0, 0.0, 0.0);
        }


//    Color2 array
        if (isAffectedByMOCV && (cvLen2 > 0)) {
            glEnableVertexAttribArray(+wmoShader::Attribute::aColor2);
            glVertexAttribPointer(+wmoShader::Attribute::aColor2, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, (void *)colorOffset2); // color
        } else {
            glDisableVertexAttribArray(+wmoShader::Attribute::aColor2);
            //Without 1.0 new Undercity has wrong color on the walls
            glVertexAttrib4f(+wmoShader::Attribute::aColor2, 0.0, 0.0, 0.0, 1.0);
        }

        glUniform4fv(wmoShader->getUnf("uAmbientLight"),
            1,
            &ambientColor[0]
        );

        glUniform1i(wmoShader->getUnf("uUseLitColor"), 1);

        if (materials[texIndex].flags.F_UNLIT) {
            glUniform1i(wmoShader->getUnf("uUseLitColor"), 0);
        }

        if (material.blendMode != 0) {
            float alphaTestVal = 0.878431f;
//            if ((materials[texIndex].flags. & 0x80) > 0) {
//                //alphaTestVal = 0.2999999;
//            }
            glUniform1f(wmoShader->getUnf("uAlphaTest"), alphaTestVal);
        } else {
            glUniform1f(wmoShader->getUnf("uAlphaTest"), -1.0f);
        }

        auto blendMode = material.blendMode;
        switch (material.blendMode) {
            case 0 : //Blend_Opaque
                glDisable(GL_BLEND);
                glUniform1f(wmoShader->getUnf("uAlphaTest"), -1.0);
                glUniform1i(wmoShader->getUnf("uEnableAlpha"), 0);
                break;
            case 1 : //Blend_AlphaKey
                glDisable(GL_BLEND);
                glUniform1f(wmoShader->getUnf("uAlphaTest"), 70.0/255.0);
                glUniform1i(wmoShader->getUnf("uEnableAlpha"), 1);
                break;
            case 2 : //Blend_Alpha
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glUniform1f(wmoShader->getUnf("uAlphaTest"), 70.0/255.0);
                glUniform1i(wmoShader->getUnf("uEnableAlpha"), 1);
                break;

            case 3 : //GxBlend_Add
                glUniform1f(wmoShader->getUnf("uAlphaTest"), 0.00392157);
                glUniform1i(wmoShader->getUnf("uEnableAlpha"), 1);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE);

                break;

            default :
//                glUniform1f(wmoShader->getUnf("uAlphaTest"), -1);
                glDisable(GL_BLEND);
                glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);

                glUniform1f(wmoShader->getUnf("uAlphaTest"), 244.0/255.0);
                glUniform1i(wmoShader->getUnf("uEnableAlpha"), 1);

                std::cout << "material.blendMode = " << material.blendMode;

                break;
        }

        if (material.flags.F_UNCULLED) {
            glDisable(GL_CULL_FACE);
        } else {
            glEnable(GL_CULL_FACE);
        }



        //var textureObject = this.textureArray[j];
        bool isSecondTextSpec = material.shader == 8;

        HBlpTexture texture1 = getTextureFunc(material.diffuseNameIndex, false);
        HBlpTexture texture2 = getTextureFunc(material.envNameIndex, isSecondTextSpec);
        HBlpTexture texture3 = getTextureFunc(material.texture_2, false);

        glActiveTexture(GL_TEXTURE0);
        if (texture1 != nullptr && texture1->getIsLoaded()) {
            glBindTexture(GL_TEXTURE_2D, texture1->getGlTexture());
        }  else {
            glBindTexture(GL_TEXTURE_2D, blackPixelText);
        }
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glActiveTexture(GL_TEXTURE1);
        if (texture2 != nullptr && texture2->getIsLoaded()) {
            glBindTexture(GL_TEXTURE_2D, texture2->getGlTexture());
        } else {
            glBindTexture(GL_TEXTURE_2D, blackPixelText);
        }
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glActiveTexture(GL_TEXTURE2);
        if (texture3 != nullptr && texture3->getIsLoaded()) {
            glBindTexture(GL_TEXTURE_2D, texture3->getGlTexture());
        } else {
            glBindTexture(GL_TEXTURE_2D, blackPixelText);
        }
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

//        // Hack to skip verticles from node
//        if (bspNodeList) {
//            var currentTriangle = renderBatch.startIndex / 3;
//            var triangleCount = renderBatch.count / 3;
//            var finalTriangle = currentTriangle+triangleCount;
//
//            var mobrPiece = [];
//            for (var k = 0; k < bspNodeList.length; k++) {
//                mobrPiece = mobrPiece.concat(this.wmoGroupFile.mobr.slice(bspNodeList[k].firstFace, bspNodeList[k].firstFace+bspNodeList[k].numFaces-1));
//            }
//            mobrPiece = mobrPiece.sort(function (a,b) {return a < b ? -1 : 1}).filter(function(item, pos, ary) {
//                return !pos || item != ary[pos - 1];
//            });
//
//            var mobrIndex = 0;
//            while (currentTriangle < finalTriangle) {
//                while (mobrPiece[mobrIndex] < currentTriangle) mobrIndex++;
//                while (currentTriangle == mobrPiece[mobrIndex]) {currentTriangle++; mobrIndex++};
//
//                triangleCount = finalTriangle - currentTriangle;
//                if (currentTriangle < mobrPiece[mobrIndex] && (currentTriangle + triangleCount > mobrPiece[mobrIndex])) {
//                    triangleCount = mobrPiece[mobrIndex] - currentTriangle;
//                }
//
//                GL_drawElements(GL_TRIANGLES, triangleCount*3, GL_UNSIGNED_SHORT, currentTriangle*3 * 2);
//                currentTriangle = currentTriangle + triangleCount;
//            }
//        } else {
        glDrawElements(GL_TRIANGLES, renderBatch.num_indices, GL_UNSIGNED_SHORT, (void*)(renderBatch.first_index * 2));
//        }

//        if (textureObject && textureObject[1]) {
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
//        }
        glDisable(GL_BLEND);
    }
    glUniform1f(wmoShader->getUnf("uAlphaTest"), -1.0f);
    */
}
