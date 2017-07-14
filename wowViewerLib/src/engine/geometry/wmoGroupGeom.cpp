//
// Created by deamon on 03.07.17.
//

#include "wmoGroupGeom.h"
#include "../persistance/wmoFile.h"
#include "../shaderDefinitions.h"
#include <iostream>

chunkDef<WmoGroupGeom> WmoGroupGeom::wmoGroupTable = {
        handler : [](WmoGroupGeom& object, ChunkData& chunkData){},
        subChunks : {
                {
                        'MVER',
                        {
                                handler: [](WmoGroupGeom& object, ChunkData& chunkData){
                                    std::cout<<"Entered MVER"<<std::endl;
                                }
                        }
                },
                {
                        'MOGP',
                        {
                                handler: [](WmoGroupGeom& object, ChunkData& chunkData){
                                    chunkData.readValue(object.mogp);
                                    std::cout<<"Entered MOGP"<<std::endl;
                                },
                                subChunks: {
                                        {
                                            'MOPY', {
                                                handler: [](WmoGroupGeom& object, ChunkData& chunkData){
                                                    std::cout<<"Entered MOPY"<<std::endl;
                                                },
                                            }
                                        },
                                        {
                                            'MOVI', {
                                                handler: [](WmoGroupGeom& object, ChunkData& chunkData){
                                                    object.indicesLen = chunkData.chunkLen / 2;
                                                    chunkData.readValues(object.indicies, object.indicesLen);
                                                    std::cout<<"Entered MOVI"<<std::endl;
                                                },
                                            }
                                        },
                                        {
                                            'MOVT', {
                                                handler: [](WmoGroupGeom& object, ChunkData& chunkData){

                                                    object.verticesLen = chunkData.chunkLen / sizeof(C3Vector);
                                                    chunkData.readValues(object.verticles, object.verticesLen);
                                                    std::cout<<"Entered MOVT"<<std::endl;
                                                },
                                            }
                                        },
                                        {
                                            'MONR', {
                                                handler: [](WmoGroupGeom& object, ChunkData& chunkData){
                                                    object.normalsLen = chunkData.chunkLen / sizeof(C3Vector);
                                                    chunkData.readValues(object.normals, object.normalsLen);
                                                    std::cout<<"Entered MONR"<<std::endl;
                                                },
                                            }
                                        },
                                        {
                                            'MOTV', {
                                                handler: [](WmoGroupGeom& object, ChunkData& chunkData){
                                                    object.textureCoordsLen = chunkData.chunkLen / sizeof(C2Vector);
                                                    chunkData.readValues(object.textCoords, object.textureCoordsLen);

                                                    std::cout<<"Entered MOTV"<<std::endl;
                                                },
                                            }
                                        },
                                        {
                                            'MOCV', {
                                                handler: [](WmoGroupGeom& object, ChunkData& chunkData){
                                                    object.cvLen = chunkData.chunkLen / 4;
                                                    chunkData.readValues(object.colorArray, object.cvLen);
                                                    std::cout<<"Entered MOCV"<<std::endl;
                                                },
                                            }
                                        },
                                        {
                                            'MODR', {
                                                handler: [](WmoGroupGeom& object, ChunkData& chunkData){
                                                    object.doodadRefsLen = chunkData.chunkLen / 2;
                                                    chunkData.readValues(object.doodadRefs, object.doodadRefsLen);
                                                    std::cout<<"Entered MODR"<<std::endl;
                                                },
                                            }
                                        },
                                        {
                                            'MOBA', {
                                                handler: [](WmoGroupGeom& object, ChunkData& chunkData){
                                                    object.batchesLen = chunkData.chunkLen / sizeof(SMOBatch);
                                                    chunkData.readValues(object.batches, object.batchesLen);
                                                    std::cout<<"Entered MOBA"<<std::endl;
                                                },
                                            }
                                        },
                                        {
                                            'MOBN', {
                                                handler: [](WmoGroupGeom& object, ChunkData& chunkData){
                                                    object.nodesLen = chunkData.chunkLen / sizeof(t_BSP_NODE);
                                                    chunkData.readValues(object.bsp_nodes, object.nodesLen);
                                                    std::cout<<"Entered MOBN"<<std::endl;
                                                },
                                            }
                                        },
                                        {
                                            'MOBR', {
                                                handler: [](WmoGroupGeom& object, ChunkData& chunkData){
                                                    object.bpsIndiciesLen = chunkData.chunkLen / sizeof(uint16_t);
                                                    chunkData.readValues(object.bpsIndicies, object.bpsIndiciesLen);
                                                    std::cout<<"Entered MOBR"<<std::endl;
                                                },
                                            }
                                        },
                                }
                        }
                }
        }
};



void WmoGroupGeom::process(std::vector<unsigned char> &wmoGroupFile) {
    CChunkFileReader<WmoGroupGeom> reader(wmoGroupFile, &WmoGroupGeom::wmoGroupTable);
    reader.processFile(*this);

    createVBO();
    createIndexVBO();

    m_loaded = true;
}

#define size_of_pvar(x) sizeof(std::remove_pointer<decltype(x)>::type)
#define makePtr(x) (unsigned char *) x
void WmoGroupGeom::createVBO() {

    int vboSizeInBytes =
            size_of_pvar(verticles) * verticesLen+
            size_of_pvar(normals) * normalsLen+
            size_of_pvar(textCoords) * textureCoordsLen+
            size_of_pvar(colorArray) * cvLen;

    std::vector<unsigned char> buffer (vboSizeInBytes);

    unsigned char *nextOffset = &buffer[0];
    nextOffset = std::copy(makePtr(verticles),  makePtr(verticles+verticesLen), nextOffset);
    normalOffset = nextOffset-&buffer[0]; nextOffset = std::copy(makePtr(normals),    makePtr(normals+normalsLen), nextOffset);
    textOffset = nextOffset-&buffer[0];   nextOffset = std::copy(makePtr(textCoords), makePtr(textCoords+textureCoordsLen), nextOffset);
    colorOffset = nextOffset-&buffer[0];  nextOffset = std::copy(makePtr(colorArray), makePtr(colorArray+cvLen), nextOffset);

    glGenBuffers(1, &combinedVBO);
    glBindBuffer( GL_ARRAY_BUFFER, combinedVBO);
    glBufferData( GL_ARRAY_BUFFER, vboSizeInBytes, &buffer[0], GL_STATIC_DRAW );
}

void WmoGroupGeom::createIndexVBO() {
    glGenBuffers(1, &indexVBO);
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, indexVBO);
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, indicesLen, indicies, GL_STATIC_DRAW );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0);
}

void WmoGroupGeom::draw(IWoWInnerApi *api, SMOMaterial *materials, std::function <BlpTexture&(int materialId)> getTextureFunc) {

//    var shaderUniforms = this.sceneApi.shaders.getShaderUniforms();
//    var shaderAttributes = this.sceneApi.shaders.getShaderAttributes();

    GLuint blackPixelText = api->getBlackPixelTexture();


    bool isIndoor = (mogp->flags & 0x2000) > 0;

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->indexVBO);
    glBindBuffer(GL_ARRAY_BUFFER, this->combinedVBO);

    glEnableVertexAttribArray(+wmoShader::Attribute::aPosition);
    glVertexAttribPointer(+wmoShader::Attribute::aPosition, 3, GL_FLOAT, false, 0, 0); // position
//    if (shaderAttributes.aNormal != null) {
        glVertexAttribPointer(+wmoShader::Attribute::aNormal, 3, GL_FLOAT, false, 0, (void *)normalOffset); // normal
//    }
    glVertexAttribPointer(+wmoShader::Attribute::aTexCoord, 2, GL_FLOAT, false, 0, (void *)textOffset); // texcoord


//    if (shaderAttributes.aTexCoord2 != null) {
//    if ((textCoords2) && (wmoGroupObject.textCoords2.length > 0)) {
//        glEnableVertexAttribArray(shaderAttributes.aTexCoord2);
//        glVertexAttribPointer(shaderAttributes.aTexCoord2, 2, GL_FLOAT, false, 0, this.textOffset2 * 4); // texcoord
//    } else {
        glDisableVertexAttribArray(+wmoShader::Attribute::aTexCoord2);
        glVertexAttrib2f(+wmoShader::Attribute::aTexCoord2, 1.0, 1.0);
//    }
//    }

    if (isIndoor && (cvLen > 0)) {
        glEnableVertexAttribArray(+wmoShader::Attribute::aColor);
        glVertexAttribPointer(+wmoShader::Attribute::aColor, 4, GL_UNSIGNED_BYTE, true, 0, (void *)colorOffset); // color
    } else {
        glDisableVertexAttribArray(+wmoShader::Attribute::aColor);
        glVertexAttrib4f(+wmoShader::Attribute::aColor, 1.0, 1.0, 1.0, 1.0);
    }


    //Color2 array
//    if (isIndoor && (wmoGroupObject.colorVerticles2) &&(wmoGroupObject.colorVerticles2.length > 0)) {
//        glEnableVertexAttribArray(shaderAttributes.aColor2);
//        glVertexAttribPointer(shaderAttributes.aColor2, 4, GL_UNSIGNED_BYTE, true, 0, this.colorOffset2 * 4); // color
//    } else {
        glDisableVertexAttribArray(+wmoShader::Attribute::aColor2);
        glVertexAttrib4f(+wmoShader::Attribute::aColor2, 1.0, 1.0, 1.0, 1.0);
//    }

    auto wmoShader = api->getWmoShader();

    glActiveTexture(GL_TEXTURE0);
    glUniform1f(wmoShader->getUnf("uAlphaTest"), -1.0);

    //for (var j = 0; j < wmoGroupObject.mogp.numBatchesA; j++) {
    for (int j = 0; j < batchesLen; j++) {
        SMOBatch &renderBatch = batches[j];

        int texIndex;
        if (renderBatch.flag_use_material_id_large) {
            texIndex = renderBatch.postLegion.material_id_large;
        } else {
            texIndex = renderBatch.material_id;
        }

        uint32_t color = materials[texIndex].diffColor;
//        var colorVector = [color&0xff, (color>> 8)&0xff,
//                (color>>16)&0xff, (color>> 24)&0xff];
//        colorVector[0] /= 255.0; colorVector[1] /= 255.0;
//        colorVector[2] /= 255.0; colorVector[3] /= 255.0;

//        ambientColor = [1,1,1,1];
        glUniform4f(wmoShader->getUnf("uMeshColor1"), 1.0f, 1.0f, 1.0f, 1.0f);


        if (materials[texIndex].blendMode != 0) {
            float alphaTestVal = 0.878431f;
//            if ((materials[texIndex].flags. & 0x80) > 0) {
//                //alphaTestVal = 0.2999999;
//            }
            if (materials[texIndex].flags.F_UNLIT) {
                alphaTestVal = 0.1f; //TODO: confirm this
            }

            glUniform1f(wmoShader->getUnf("uAlphaTest"), alphaTestVal);
        } else {
            glUniform1f(wmoShader->getUnf("uAlphaTest"), -1.0d);
        }

        if (materials[texIndex].flags.F_UNCULLED) {
            glDisable(GL_CULL_FACE);
        } else {
            glEnable(GL_CULL_FACE);
        }

        //var textureObject = this.textureArray[j];

        BlpTexture texture1 = getTextureFunc(materials[texIndex].diffuseNameIndex);
//        BlpTexture texture2 = getTextureFunc(materials[texIndex].texture_2);

        glActiveTexture(GL_TEXTURE0);
        if (texture1.getIsLoaded()) {
            glBindTexture(GL_TEXTURE_2D, texture1.getGlTexture());
        }  else {
            glBindTexture(GL_TEXTURE_2D, blackPixelText);
        }
//        if (textureObject && textureObject[1]) {
//            glActiveTexture(GL_TEXTURE1);
//            glBindTexture(GL_TEXTURE_2D, textureObject[1].texture);
//        } else {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, blackPixelText);
//        }

//        /* Hack to skip verticles from node */
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
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, 0);
//        }
    }
    glUniform1f(wmoShader->getUnf("uAlphaTest"), -1.0d);

}
