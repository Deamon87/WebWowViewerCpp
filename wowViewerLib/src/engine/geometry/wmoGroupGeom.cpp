//
// Created by deamon on 03.07.17.
//

#include "wmoGroupGeom.h"
#include "../persistance/header/wmoFileHeader.h"
#include "../shader/ShaderDefinitions.h"
#include "../../gapi/interface/IDevice.h"
#include "../algorithms/mathHelper.h"
#include <iostream>

chunkDef<WmoGroupGeom> WmoGroupGeom::wmoGroupTable = {
    [](WmoGroupGeom& object, ChunkData& chunkData){},
    {
        {
            'MVER',
            {
                [](WmoGroupGeom& object, ChunkData& chunkData){
                    uint32_t  version;
                    debuglog("Entered MVER");
                    chunkData.readValue(version);
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
                        'MLIQ', {
                            [](WmoGroupGeom& object, ChunkData& chunkData){
                                debuglog("Entered MLIQ");
                                chunkData.readValue(object.m_mliq);

                                static_assert(sizeof(MLIQ) == 30, "MLIQ structure has garbage align bytes");

                                object.m_liquidVerticles_len = object.m_mliq->xverts * object.m_mliq->yverts;
                                chunkData.readValues(object.m_liquidVerticles, object.m_liquidVerticles_len);

                                object.m_liquidTiles_len = object.m_mliq->xtiles*object.m_mliq->ytiles;
                                chunkData.readValues(object.m_liquidTiles, object.m_liquidTiles_len);
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
                                } else if (object.textureCoordsRead == 3) {
                                    debuglog("texture coords 4 in file "+ object.getFileName() + " "+std::to_string(object.getFileDataId()));
                                    object.textureCoordsLen4 = chunkData.chunkLen / sizeof(C2Vector);
                                    chunkData.readValues(object.textCoords4,
                                                         object.textureCoordsLen4);
                                } else {
                                    debuglog("object.textureCoordsRead = "+std::to_string(object.textureCoordsRead));
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
                                    object.cvLen = chunkData.chunkLen / 4;
                                    chunkData.readValues(object.colorArray, object.cvLen);
                                    object.mocvRead++;
                               } else if (object.mogp->flags.CVERTS2 && object.mocvRead < 2) {
                                    object.cvLen2 = chunkData.chunkLen / 4;
                                    chunkData.readValues(object.colorArray2, object.cvLen2);
                                    object.mocvRead++;
                               } else {
                                    std::cout << "Spotted incorrect MOCV cout" << std::endl;
                                }
                            },
                        }
                    },
                    {
                        'MOC2', {
                        [](WmoGroupGeom& object, ChunkData& chunkData){
                                debuglog("Entered MOC2");
                                object.cvSecondLen = chunkData.chunkLen / 4;
                                chunkData.readValues(object.colorSecondArray, object.cvSecondLen);
                            }
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

void WmoGroupGeom::process(HFileContent wmoGroupFile, const std::string &fileName) {
    m_wmoGroupFile = wmoGroupFile;

    CChunkFileReader reader(*m_wmoGroupFile.get());
    reader.processFile(*this, &WmoGroupGeom::wmoGroupTable);

    fixColorVertexAlpha(mohd);
    if (!mohd->flags.flag_attenuate_vertices_based_on_distance_to_portal) {
        this->m_attenuateFunc(*this);
    }

    fsStatus = FileStatus::FSLoaded;
}


void WmoGroupGeom::fixColorVertexAlpha(SMOHeader *mohd) {
    int begin_second_fixup = 0;
    if (batches == nullptr) return;

    if (mogp->transBatchCount) {
        begin_second_fixup = batches[(uint16_t) mogp->transBatchCount-1].last_vertex + 1;

    }

    unsigned char v35;
    unsigned char v36;
    unsigned char v37;

    if (mohd->flags.flag_lighten_interiors) {
        for (int i(begin_second_fixup); i < cvLen; ++i) {
            colorArray[i].a = (unsigned char) ((mogp->flags.EXTERIOR > 0) ? 0xFF : 0x00);
        }
    } else {
        if (mohd->flags.flag_skip_base_color) {
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


HGVertexBuffer WmoGroupGeom::getVBO(const HMapSceneBufferCreate &sceneRenderer) {
    if (combinedVBO == nullptr) {
        combinedVBO = sceneRenderer->createWMOVertexBuffer(verticesLen * sizeof(WMOVertex));

        WMOVertex * buffer = static_cast<WMOVertex *>(combinedVBO->getPointer());

        static const C2Vector c2ones = C2Vector(mathfu::vec2(1.0, 1.0));
        static const C3Vector c3zeros = C3Vector(mathfu::vec3(0, 0, 0));

        for (int i = 0; i < verticesLen; i++) {
            WMOVertex &format = buffer[i];
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
            if (textureCoordsLen4 > 0) {
                format.textCoordinate4 = textCoords4[i];
            } else {
                format.textCoordinate4 = c2ones;
            }
            if (cvLen > 0) {
                format.color = colorArray[i];
            } else {
                format.color.r = 0;
                format.color.g = 0;
                format.color.b = 0;
                format.color.a = 0;
            }
            if (cvLen2 > 0) {
                format.color2 = colorArray2[i];
            } else {
                format.color2.r = 0;
                format.color2.g = 0;
                format.color2.b = 0;
                format.color2.a = 0xFF;
            }
            if (cvSecondLen > 0) {
                format.colorSecond = colorSecondArray[i];
            } else {
                format.colorSecond.r = 0;
                format.colorSecond.g = 0;
                format.colorSecond.b = 0;
                format.colorSecond.a = 0xFF;
            }
        }

        combinedVBO->uploadData(&buffer[0], (int)(verticesLen * sizeof(WMOVertex)));
    }

    return combinedVBO;
}

HGIndexBuffer WmoGroupGeom::getIBO(const HMapSceneBufferCreate &sceneRenderer) {
    if (indexVBO == nullptr) {
        indexVBO = sceneRenderer->createWMOIndexBuffer(indicesLen * sizeof(uint16_t));
        indexVBO->uploadData(
            &indicies[0],
            indicesLen * sizeof(uint16_t));
    }

    return indexVBO;
}

HGVertexBufferBindings WmoGroupGeom::getVertexBindings(const HMapSceneBufferCreate &sceneRenderer, mathfu::vec4 localAmbient) {
    if (vertexBufferBindings == nullptr) {

        vertexBufferBindings = sceneRenderer->createWmoVAO(getVBO(sceneRenderer), getIBO(sceneRenderer), localAmbient);
    }

    return vertexBufferBindings;
}


int WmoGroupGeom::getLegacyWaterType(int a) {
    a = a + 1;

    if ( (a - 1) <= 0x13 )
    {
        int newwater = (a - 1) & 3;
        if ( newwater == 1 ) {
            a = 14;
            return a;
        }
            
        if ( newwater >= 1 )
        {
            if ( newwater == 2 )
            {
                a = 19;
            }
            else if ( newwater == 3 )
            {
                a = 20;
            }
            return a;
        }
        a = 13;
    }
    return a;
}

HGVertexBufferBindings WmoGroupGeom::getWaterVertexBindings(const HMapSceneBufferCreate &sceneRenderer) {

    if (vertexWaterBufferBindings == nullptr) {
        if (this->m_mliq == nullptr) return nullptr;

        std::vector<LiquidVertexFormat> lVertices;
//        lVertices.reserve((m_mliq->xverts)*(m_mliq->yverts)*3);

        mathfu::vec3 pos(m_mliq->basePos.x, m_mliq->basePos.y, m_mliq->basePos.z);

        for (int j = 0; j < m_mliq->yverts; j++) {
            for (int i = 0; i < m_mliq->xverts; i++) {
                int p = j*m_mliq->xverts + i;

                LiquidVertexFormat lvfVertex;
                lvfVertex.pos_transp = mathfu::vec4_packed(mathfu::vec4(
                    pos.x + (MathHelper::UNITSIZE * i),
                    pos.y + (MathHelper::UNITSIZE * j),
                    m_liquidVerticles[p].waterVert.height,
                    1.0
                ));
                lvfVertex.uv = mathfu::vec2(0,0);

                lVertices.push_back(lvfVertex);
            }
        }


        std::vector<uint16_t> iboBuffer;

        for (int j = 0; j < m_mliq->ytiles; j++) {
            for (int i = 0; i < m_mliq->xtiles; i++) {
                int tileIndex = j*m_mliq->xtiles + i;
                assert(tileIndex < m_liquidTiles_len);
                SMOLTile tile = m_liquidTiles[tileIndex];

                if (tile.legacyLiquidType == 15) continue;

                if (liquidType == -1) {
                    liquidType = getLegacyWaterType(tile.legacyLiquidType);
                }

                int16_t vertindexes[4] = {
                    (int16_t) (j * (m_mliq->xverts) + i),
                    (int16_t) (j * (m_mliq->xverts) + i + 1),
                    (int16_t) ((j + 1) * (m_mliq->xverts) + i + 1),
                    (int16_t) ((j + 1) * (m_mliq->xverts) + i)
                };

                iboBuffer.push_back (vertindexes[0]);
                iboBuffer.push_back (vertindexes[1]);
                iboBuffer.push_back (vertindexes[2]);

                iboBuffer.push_back (vertindexes[0]);
                iboBuffer.push_back (vertindexes[2]);
                iboBuffer.push_back (vertindexes[3]);

            }
        }

        waterIBO = sceneRenderer->createWaterIndexBuffer(iboBuffer.size() * sizeof(uint16_t));
        waterIBO->uploadData(
            &iboBuffer[0],
            iboBuffer.size() * sizeof(uint16_t));

        waterVBO = sceneRenderer->createWaterVertexBuffer(lVertices.size() * sizeof(LiquidVertexFormat));
        waterVBO->uploadData(
            &lVertices[0],
            lVertices.size() * sizeof(LiquidVertexFormat)
        );

        vertexWaterBufferBindings = sceneRenderer->createWaterVAO(waterVBO, waterIBO);

        waterIndexSize = iboBuffer.size();
    }

    return vertexWaterBufferBindings;

}
