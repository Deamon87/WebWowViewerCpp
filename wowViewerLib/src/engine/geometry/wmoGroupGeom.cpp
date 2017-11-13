//
// Created by deamon on 03.07.17.
//

#include "wmoGroupGeom.h"
#include "../persistance/header/wmoFileHeader.h"
#include "../shaderDefinitions.h"
#include <iostream>
/*
//0
MapObjDiffuse {
    VertexShader(MapObjDiffuse_T1);
    PixelShader(MapObjDiffuse);
}

//1
MapObjSpecular {
    VertexShader(MapObjSpecular_T1);
    PixelShader(MapObjSpecular);
}

//2
MapObjMetal {
    VertexShader(MapObjSpecular_T1);
    PixelShader(MapObjMetal);
}

//3
MapObjEnv {
    VertexShader(MapObjDiffuse_T1_Refl);
    PixelShader(MapObjEnv);
}

//4
MapObjOpaque {
    VertexShader(MapObjDiffuse_T1);
    PixelShader(MapObjOpaque);
}

//5
Effect(MapObjEnvMetal {
    VertexShader(MapObjDiffuse_T1_Refl);
    PixelShader(MapObjEnvMetal);
}

//6
Effect(MapObjComposite) //aka MapObjTwoLayerDiffuse
{
    VertexShader(MapObjDiffuse_Comp);
    PixelShader(MapObjComposite); //aka MapObjTwoLayerDiffuse
}

//7
Effect(MapObjTwoLayerEnvMetal)
{
    VertexShader(MapObjDiffuse_Comp_Refl);
    PixelShader(MapObjTwoLayerEnvMetal);
}

//8
Effect(TwoLayerTerrain)
{
    VertexShader(MapObjDiffuse_Comp_Terrain);
    PixelShader(MapObjTwoLayerTerrain);
}

//9
Effect(MapObjDiffuseEmissive)
{
    VertexShader(MapObjDiffuse_Comp);
    PixelShader(MapObjDiffuseEmissive);
}

//10
Effect(waterWindow)
{
e    //unk
}

//11
Effect(MapObjMaskedEnvMetal)
{
    VertexShader(MapObjDiffuse_T1_Env_T2);
    PixelShader(MapObjMaskedEnvMetal);
}

//12
Effect(MapObjEnvMetalEmissive)
{
    VertexShader(MapObjDiffuse_T1_Env_T2);
    PixelShader(MapObjEnvMetalEmissive);
}

//13
Effect(TwoLayerDiffuseOpaque)
{
    VertexShader(MapObjDiffuse_Comp);
    PixelShader(MapObjTwoLayerDiffuseOpaque);
}

//14
Effect(submarineWindow)
{
    //unk
}

//15
Effect(TwoLayerDiffuseEmissive)
{
    VertexShader(MapObjDiffuse_Comp);
    PixelShader(MapObjTwoLayerDiffuseEmissive);
}

//16
Effect(MapObjDiffuseTerrain)
{
    VertexShader(MapObjDiffuse_T1);
    PixelShader(MapObjDiffuse);
}

*/

enum class WmoVertexShader : int {
    None = -1,
    MapObjDiffuse_T1 = 0,
    MapObjDiffuse_T1_Refl = 1,
    MapObjDiffuse_T1_Env_T2 = 2,
    MapObjSpecular_T1 = 3,
    MapObjDiffuse_Comp = 4,
    MapObjDiffuse_Comp_Refl = 5,
    MapObjDiffuse_Comp_Terrain = 6
};

enum class WmoPixelShader : int {
    None = -1,
    MapObjDiffuse = 0,
    MapObjSpecular = 1,
    MapObjMetal = 2,
    MapObjEnv = 3,
    MapObjOpaque = 4,
    MapObjEnvMetal = 5,
    MapObjTwoLayerDiffuse = 6, //MapObjComposite
    MapObjTwoLayerEnvMetal = 7,
    MapObjTwoLayerTerrain = 8,
    MapObjDiffuseEmissive = 9,
    MapObjMaskedEnvMetal = 10,
    MapObjEnvMetalEmissive = 11,
    MapObjTwoLayerDiffuseOpaque = 12,
    MapObjTwoLayerDiffuseEmissive = 13
};

inline constexpr const int operator+ (WmoPixelShader const val) { return static_cast<const int>(val); };
inline constexpr const int operator+ (WmoVertexShader const val) { return static_cast<const int>(val); };

const int MAX_WMO_SHADERS = 17;
static const struct {
    int vertexShader;
    int pixelShader;
} wmoMaterialShader[MAX_WMO_SHADERS] = {
    //MapObjDiffuse = 0
    {
        vertexShader: +WmoVertexShader::MapObjDiffuse_T1,
        pixelShader: +WmoPixelShader::MapObjDiffuse,
    },
    //MapObjSpecular = 1
    {
        vertexShader: +WmoVertexShader::MapObjSpecular_T1,
        pixelShader: +WmoPixelShader::MapObjSpecular,
    },
    //MapObjMetal = 2
    {
        vertexShader: +WmoVertexShader::MapObjSpecular_T1,
        pixelShader: +WmoPixelShader::MapObjMetal,
    },
    //MapObjEnv = 3
    {
        vertexShader: +WmoVertexShader::MapObjDiffuse_T1_Refl,
        pixelShader: +WmoPixelShader::MapObjEnv,
    },
    //MapObjOpaque = 4
    {
        vertexShader: +WmoVertexShader::MapObjDiffuse_T1,
        pixelShader: +WmoPixelShader::MapObjOpaque,
    },
    //MapObjEnvMetal = 5
    {
        vertexShader: +WmoVertexShader::MapObjDiffuse_T1_Refl,
        pixelShader: +WmoPixelShader::MapObjEnvMetal,
    },
    //MapObjTwoLayerDiffuse = 6
    {
        vertexShader: +WmoVertexShader::MapObjDiffuse_Comp,
        pixelShader: +WmoPixelShader::MapObjTwoLayerDiffuse,
    },
    //MapObjTwoLayerEnvMetal = 7
    {
        vertexShader: +WmoVertexShader::MapObjDiffuse_T1,
        pixelShader: +  WmoPixelShader::MapObjDiffuse,
    },
    //TwoLayerTerrain = 8
    {
        vertexShader: +WmoVertexShader::MapObjDiffuse_Comp_Terrain,
        pixelShader: +WmoPixelShader::MapObjTwoLayerTerrain,
    },
    //MapObjDiffuseEmissive = 9
    {
        vertexShader: +WmoVertexShader::MapObjDiffuse_Comp,
        pixelShader: +WmoPixelShader::MapObjDiffuseEmissive,
    },
    //waterWindow = 10
    {
        vertexShader: +WmoVertexShader::None,
        pixelShader: +WmoPixelShader::None,
    },
    //MapObjMaskedEnvMetal = 11
    {
        vertexShader: +WmoVertexShader::MapObjDiffuse_T1_Env_T2,
        pixelShader: +WmoPixelShader::MapObjMaskedEnvMetal,
    },
    //MapObjEnvMetalEmissive = 12
    {
        vertexShader: +WmoVertexShader::MapObjDiffuse_T1_Env_T2,
        pixelShader: +WmoPixelShader::MapObjEnvMetalEmissive,
    },
    //TwoLayerDiffuseOpaque = 13
    {
        vertexShader: +WmoVertexShader::MapObjDiffuse_Comp,
        pixelShader: +WmoPixelShader::MapObjTwoLayerDiffuseOpaque,
    },
    //submarineWindow = 14
    {
        vertexShader: +WmoVertexShader::None,
        pixelShader: +WmoPixelShader::None,
    },
    //TwoLayerDiffuseEmissive = 15
    {
        vertexShader: +WmoVertexShader::MapObjDiffuse_Comp,
        pixelShader: +WmoPixelShader::MapObjTwoLayerDiffuseEmissive,
    },
    //MapObjDiffuseTerrain = 16
    {
        vertexShader: +WmoVertexShader::MapObjDiffuse_T1,
        pixelShader: +WmoPixelShader::MapObjDiffuse,
    },
};

chunkDef<WmoGroupGeom> WmoGroupGeom::wmoGroupTable = {
    handler : [](WmoGroupGeom& object, ChunkData& chunkData){},
    subChunks : {
        {
            'MVER',
            {
                handler: [](WmoGroupGeom& object, ChunkData& chunkData){
                    debuglog("Entered MVER");
                }
            }
        },
        {
            'MOGP',
            {
                handler: [](WmoGroupGeom& object, ChunkData& chunkData){
                    chunkData.readValue(object.mogp);
                    debuglog("Entered MOGP");
                },
                subChunks: {
                    {
                        'MOPY', {
                            handler: [](WmoGroupGeom& object, ChunkData& chunkData){
                                debuglog("Entered MOPY");
                            },
                        }
                    },
                    {
                        'MOVI', {
                            handler: [](WmoGroupGeom& object, ChunkData& chunkData){
                                object.indicesLen = chunkData.chunkLen / 2;
                                chunkData.readValues(object.indicies, object.indicesLen);
                                debuglog("Entered MOVI");
                            },
                        }
                    },
                    {
                        'MOVT', {
                            handler: [](WmoGroupGeom& object, ChunkData& chunkData){

                                object.verticesLen = chunkData.chunkLen / sizeof(C3Vector);
                                chunkData.readValues(object.verticles, object.verticesLen);
                                debuglog("Entered MOVT");
                            },
                        }
                    },
                    {
                        'MONR', {
                            handler: [](WmoGroupGeom& object, ChunkData& chunkData){
                                object.normalsLen = chunkData.chunkLen / sizeof(C3Vector);
                                chunkData.readValues(object.normals, object.normalsLen);
                                debuglog("Entered MONR");
                            },
                        }
                    },
                    {
                        'MOTV', {
                            handler: [](WmoGroupGeom& object, ChunkData& chunkData){
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
                            handler: [](WmoGroupGeom& object, ChunkData& chunkData){
                                debuglog("Entered MOCV");
                                if (object.mocvRead == 0) {
                                    object.cvLen = chunkData.chunkLen / 4;
                                    chunkData.readValues(object.colorArray, object.cvLen);
                                } else if (object.mocvRead == 1) {
                                    object.cvLen2 = chunkData.chunkLen / 4;
                                    chunkData.readValues(object.colorArray2, object.cvLen2);
                                }

                                object.mocvRead++;
                            },
                        }
                    },
                    {
                        'MODR', {
                            handler: [](WmoGroupGeom& object, ChunkData& chunkData){
                                object.doodadRefsLen = chunkData.chunkLen / 2;
                                chunkData.readValues(object.doodadRefs, object.doodadRefsLen);
                                debuglog("Entered MODR");
                            },
                        }
                    },
                    {
                        'MOLR', {
                            handler: [](WmoGroupGeom& object, ChunkData& chunkData){
                                debuglog("Entered MOLR");
                                object.lightRefListLen = chunkData.chunkLen / 2;
                                chunkData.readValues(object.lightRefList, object.lightRefListLen);
                            },
                        }
                    },
                    {
                        'MOBA', {
                            handler: [](WmoGroupGeom& object, ChunkData& chunkData){
                                object.batchesLen = chunkData.chunkLen / sizeof(SMOBatch);
                                chunkData.readValues(object.batches, object.batchesLen);
                                debuglog("Entered MOBA");
                            },
                        }
                    },
                    {
                        'MOBN', {
                            handler: [](WmoGroupGeom& object, ChunkData& chunkData){
                                object.nodesLen = chunkData.chunkLen / sizeof(t_BSP_NODE);
                                chunkData.readValues(object.bsp_nodes, object.nodesLen);
                                debuglog("Entered MOBN");
                            },
                        }
                    },
                    {
                        'MOBR', {
                            handler: [](WmoGroupGeom& object, ChunkData& chunkData){
                                object.bpsIndiciesLen = chunkData.chunkLen / sizeof(uint16_t);
                                chunkData.readValues(object.bpsIndicies, object.bpsIndiciesLen);
                                debuglog("Entered MOBR");
                            },
                        }
                    },
                    {
                        'MDAL', {
                            handler: [](WmoGroupGeom& object, ChunkData& chunkData){
                                debuglog("Entered MDAL");
                                object.use_replacement_for_header_color = 1;
                                chunkData.readValue(object.replacement_for_header_color);

                            },
                        }
                    },
                    {
                        'MOLP', {
                            handler: [](WmoGroupGeom& object, ChunkData& chunkData){
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



void WmoGroupGeom::process(std::vector<unsigned char> &wmoGroupFile) {
    m_wmoGroupFile = wmoGroupFile;

    CChunkFileReader reader(m_wmoGroupFile);
    reader.processFile(*this, &WmoGroupGeom::wmoGroupTable);

    fixColorVertexAlpha(mohd);
    createVBO();
    createIndexVBO();



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
            colorArray[i].a = mogp->flags & 0x8 ? 0xFF : 0x00;
        }
    } else {
        if (!mohd->flags.flag_skip_base_color) {
            v35 = 0;
            v36 = 0;
            v37 = 0;
        } else {
            if (use_replacement_for_header_color) {
                v35 = replacement_for_header_color.b;
                v37 = replacement_for_header_color.g;
                v36 = replacement_for_header_color.r;
            } else {
                v35 = mohd->ambColor.b;
                v37 = mohd->ambColor.g;
                v36 = mohd->ambColor.r;
            }
        }

        for (int mocv_index(0); mocv_index < begin_second_fixup; ++mocv_index) {
            colorArray[mocv_index].r -= v36;
            colorArray[mocv_index].g -= v37;
            colorArray[mocv_index].b -= v35;

            float v38 = colorArray[mocv_index].a / 255.0f;

            float v11 = colorArray[mocv_index].r - v38 * colorArray[mocv_index].r;
            assert (v11 > -0.5f);
            assert (v11 < 255.5f);
            colorArray[mocv_index].r = v11 / 2;
            float v13 = colorArray[mocv_index].g - v38 * colorArray[mocv_index].g;
            assert (v13 > -0.5f);
            assert (v13 < 255.5f);
            colorArray[mocv_index].g = v13 / 2;
            float v14 = colorArray[mocv_index].b - v38 * colorArray[mocv_index].b;
            assert (v14 > -0.5f);
            assert (v14 < 255.5f);
            colorArray[mocv_index++].b = v14 / 2;
        }

        for (int i(begin_second_fixup); i < cvLen; ++i) {
            float v19 = (colorArray[i].r * colorArray[i].a) / 64 + colorArray[i].r - v36;
            colorArray[i].r = std::min(255.0f, std::max(v19 / 2.0f, 0.0f));

            float v30 = (colorArray[i].g * colorArray[i].a) / 64 + colorArray[i].g - v37;
            colorArray[i].g = std::min(255.0f, std::max(v30 / 2.0f, 0.0f));

            float v33 = (colorArray[i].a * colorArray[i].b) / 64 + colorArray[i].b - v35;
            colorArray[i].b = std::min(255.0f, std::max(v33 / 2.0f, 0.0f));

            colorArray[i].a = mogp->flags & 0x8 ? 0xFF : 0x00;


        }
    }

//        float red = mohd->ambColor.r;
//        float green = mohd->ambColor.g;
//        float blue = mohd->ambColor.b;
//
//        for (int i(0); i < cvLen; ++i) {
//            colorArray[i].r = std::min(255.0f, red + colorArray[i].r) ;
//            colorArray[i].g = std::min(255.0f, (green + colorArray[i].g));
//            colorArray[i].b = std::min(255.0f, (blue + colorArray[i].b));
//        }

}


#define size_of_pvar(x) sizeof(std::remove_pointer<decltype(x)>::type)
#define makePtr(x) (unsigned char *) x
void WmoGroupGeom::createVBO() {

    int vboSizeInBytes =
            size_of_pvar(verticles) * verticesLen+
            size_of_pvar(normals) * normalsLen+
            size_of_pvar(textCoords) * textureCoordsLen+
            size_of_pvar(textCoords2) * textureCoordsLen2+
            size_of_pvar(textCoords3) * textureCoordsLen3+
            size_of_pvar(colorArray) * cvLen +
            size_of_pvar(colorArray2) * cvLen2;

    std::vector<unsigned char> buffer (vboSizeInBytes);

    unsigned char *nextOffset = &buffer[0];
    nextOffset = std::copy(makePtr(verticles),  makePtr(&verticles[verticesLen]), nextOffset);
    normalOffset = nextOffset-&buffer[0]; nextOffset = std::copy(makePtr(normals),    makePtr(&normals[normalsLen]), nextOffset);
    textOffset = nextOffset-&buffer[0];   nextOffset = std::copy(makePtr(textCoords), makePtr(&textCoords[textureCoordsLen]), nextOffset);
    if (textureCoordsLen2 > 0) {
        textOffset2 = nextOffset - &buffer[0];
        nextOffset = std::copy(makePtr(textCoords2), makePtr(&textCoords2[textureCoordsLen2]), nextOffset);
    }
    if (textureCoordsLen3 > 0) {
        textOffset3 = nextOffset - &buffer[0];
        nextOffset = std::copy(makePtr(textCoords3), makePtr(&textCoords3[textureCoordsLen3]), nextOffset);
    }
    if (cvLen > 0) {
        colorOffset = nextOffset - &buffer[0];
        nextOffset = std::copy(makePtr(colorArray), makePtr(&colorArray[cvLen]), nextOffset);
    }
    if (cvLen2 > 0) {
        colorOffset2 = nextOffset - &buffer[0];
        nextOffset = std::copy(makePtr(colorArray2), makePtr(&colorArray2[cvLen2]), nextOffset);
    }

    glGenBuffers(1, &combinedVBO);
    glBindBuffer( GL_ARRAY_BUFFER, combinedVBO);
    glBufferData( GL_ARRAY_BUFFER, vboSizeInBytes, &buffer[0], GL_STATIC_DRAW );
}

void WmoGroupGeom::createIndexVBO() {
    glGenBuffers(1, &indexVBO);
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, indexVBO);
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, indicesLen*2, indicies, GL_STATIC_DRAW );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0);
}

void WmoGroupGeom::draw(IWoWInnerApi *api, SMOMaterial *materials, std::function <BlpTexture* (int materialId, bool isSpec)> getTextureFunc) {

//    var shaderUniforms = this.sceneApi.shaders.getShaderUniforms();
//    var shaderAttributes = this.sceneApi.shaders.getShaderAttributes();

    GLuint blackPixelText = api->getBlackPixelTexture();


    bool isIndoor = (mogp->flags & 0x2000) > 0;

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->indexVBO);
    glBindBuffer(GL_ARRAY_BUFFER, this->combinedVBO);

    glEnableVertexAttribArray(+wmoShader::Attribute::aPosition);
    glVertexAttribPointer(+wmoShader::Attribute::aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0); // position
//    if (shaderAttributes.aNormal != null) {
        glVertexAttribPointer(+wmoShader::Attribute::aNormal, 3, GL_FLOAT, GL_FALSE, 0, (void *)normalOffset); // normal
//    }
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

    if (isIndoor && (cvLen > 0)) {
        glEnableVertexAttribArray(+wmoShader::Attribute::aColor);
        glVertexAttribPointer(+wmoShader::Attribute::aColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, (void *)colorOffset); // color
    } else {
        glDisableVertexAttribArray(+wmoShader::Attribute::aColor);
        glVertexAttrib4f(+wmoShader::Attribute::aColor, 0.5, 0.5, 0.5, 0.5);
    }


//    Color2 array
    if (isIndoor && (cvLen2 > 0)) {
        glEnableVertexAttribArray(+wmoShader::Attribute::aColor2);
        glVertexAttribPointer(+wmoShader::Attribute::aColor2, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, (void *)colorOffset2); // color
    } else {
        glDisableVertexAttribArray(+wmoShader::Attribute::aColor2);
        glVertexAttrib4f(+wmoShader::Attribute::aColor2, 1.0, 1.0, 1.0, 1.0);
    }

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

        SMOMaterial &material = materials[texIndex];
        assert(material.shader < MAX_WMO_SHADERS && material.shader >= 0);
//        uint32_t color = material.diffColor;
        int pixelShader = wmoMaterialShader[material.shader].pixelShader;
        int vertexShader = wmoMaterialShader[material.shader].vertexShader;
//        var colorVector = [color&0xff, (color>> 8)&0xff,
//                (color>>16)&0xff, (color>> 24)&0xff];
//        colorVector[0] /= 255.0; colorVector[1] /= 255.0;
//        colorVector[2] /= 255.0; colorVector[3] /= 255.0;

//        ambientColor = [1,1,1,1];
        glUniform1i(wmoShader->getUnf("uVertexShader"), vertexShader);
        glUniform1i(wmoShader->getUnf("uPixelShader"), pixelShader);

        bool isInteriorBatch = j > mogp->transBatchCount && j < (mogp->transBatchCount + mogp->intBatchCount);

        if ((isInteriorBatch)) {
            float modifier = 0.3;


            mathfu::vec4 diffColor = mathfu::vec4(
                    ((float)material.diffColor.r) / 255.0f,
                    ((float)material.diffColor.g / 255.0f),
                    ((float)material.diffColor.b / 255.0f),
                    ((float)material.diffColor.a / 255.0f)
            );

            mathfu::vec4 ambColor = mathfu::vec4(
                    ((float)mohd->ambColor.r) / 255.0f,
                    ((float)mohd->ambColor.g / 255.0f),
                    ((float)mohd->ambColor.b / 255.0f),
                    ((float)mohd->ambColor.a / 255.0f)
            );

            mathfu::vec4 resultColor = 1.0f * (diffColor * ambColor);

            glUniform4fv(wmoShader->getUnf("uAmbientLight"),
                1,
                &resultColor[0]
            );

            ;
        } else {
            glUniform4f(wmoShader->getUnf("uAmbientLight"),
                        0,
                        0,
                        0,
                        0);
        }

//        glUniform4f(wmoShader->getUnf("uMeshColor1"), 1.0f, 1.0f, 1.0f, 1.0f);

        glUniform1i(wmoShader->getUnf("uUseLitColor"), 1);

//        if ( !renderBatch.flag_unknown_1/*materials[texIndex].flags.F_UNLIT */) {
//            glUniform1i(wmoShader->getUnf("uUseLitColor"), 1);
//        }

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
            glUniform1f(wmoShader->getUnf("uAlphaTest"), -1.0f);
        }

        switch (materials[texIndex].blendMode) {
            case 0 : //Blend_Opaque
                glDisable(GL_BLEND);
                glUniform1f(wmoShader->getUnf("uAlphaTest"), -1.0);
                break;
            case 1 : //Blend_AlphaKey
                glDisable(GL_BLEND);
                //GL_uniform1f(m2Shader->getUnf("uAlphaTest, 2.9);
                glUniform1f(wmoShader->getUnf("uAlphaTest"), 0.903921569);
                //GL_uniform1f(m2Shader->getUnf("uAlphaTest, meshColor[4]*transparency*(252/255));
                break;
            default :
                glUniform1f(wmoShader->getUnf("uAlphaTest"), -1);
                glEnable(GL_BLEND);
                glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);

                break;
        }

        if (materials[texIndex].flags.F_UNCULLED) {
            glDisable(GL_CULL_FACE);
        } else {
            glEnable(GL_CULL_FACE);
        }



        //var textureObject = this.textureArray[j];
        bool isSecondTextSpec = material.shader == 8;

        BlpTexture *texture1 = getTextureFunc(material.diffuseNameIndex, false);
        BlpTexture *texture2 = getTextureFunc(material.envNameIndex, isSecondTextSpec);
        BlpTexture *texture3 = getTextureFunc(material.texture_2, false);

        glActiveTexture(GL_TEXTURE0);
        if (texture1 != nullptr && texture1->getIsLoaded()) {
            glBindTexture(GL_TEXTURE_2D, texture1->getGlTexture());
        }  else {
            glBindTexture(GL_TEXTURE_2D, blackPixelText);
        }

        if (texture2 != nullptr && texture2->getIsLoaded()) {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, texture2->getGlTexture());
        } else {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, blackPixelText);
        }

        if (texture3 != nullptr && texture3->getIsLoaded()) {
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, texture3->getGlTexture());
        } else {
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, blackPixelText);
        }

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

}
