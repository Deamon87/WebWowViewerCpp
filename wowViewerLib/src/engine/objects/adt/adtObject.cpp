//
// Created by deamon on 10.07.17.
//

#include "adtObject.h"
#include "../../shader/ShaderDefinitions.h"
#include "../../algorithms/mathHelper.h"
#include "../../persistance/adtFile.h"
#include "../../persistance/wdtFile.h"
#include "../../../gapi/interface/IDevice.h"
#include "../../../gapi/UniformBufferStructures.h"
#include "../../persistance/header/adtFileHeader.h"

static GBufferBinding bufferBinding[5] = {
    {(uint32_t)adtShader::Attribute::aIndex, 1, GBindingType::GFLOAT, false, 60, 0},
    {(uint32_t)adtShader::Attribute::aHeight, 3, GBindingType::GFLOAT, false, 60, 4},
    {(uint32_t)adtShader::Attribute::aNormal, 3, GBindingType::GFLOAT, false, 60, 16 },
    {(uint32_t)adtShader::Attribute::aColor, 4, GBindingType::GFLOAT, false, 60, 28},
    {(uint32_t)adtShader::Attribute::aVertexLighting, 4, GBindingType::GFLOAT, false, 60, 44},
};

static GBufferBinding staticWaterBindings[2] = {
    {+adtWater::Attribute::aPositionTransp, 4, GBindingType::GFLOAT, false, 24, 0},
    {+adtWater::Attribute::aTexCoord, 2, GBindingType::GFLOAT, false, 24, 16},
//    {+waterShader::Attribute::aDepth, 1, GBindingType::GFLOAT, false, 24, 0 },
//    {+waterShader::Attribute::aTexCoord, 2, GBindingType::GFLOAT, false, 24, 4},

};



void AdtObject::loadingFinished() {
//    std::cout << "AdtObject::loadingFinished finished called";

    createVBO();
    loadAlphaTextures();
    createMeshes();
//    createIndexVBO();

    calcBoundingBoxes();

    loadM2s();
    loadWmos();
    loadWater();

    m_loaded = true;
}

void AdtObject::loadM2s() {
    uint32_t offset = 0;
    int32_t length = m_adtFileObj->doodadDef_len;
    //1. Load non-lod
    objectLods[0].m2Objects = std::vector<std::shared_ptr<M2Object>>(length, nullptr);
    for (int j = 0, i = offset; i < offset+length; i++, j++) {
        SMDoodadDef &doodadDef = m_adtFileObj->doodadDef[i];
        if (doodadDef.flags.mddf_entry_is_filedata_id) {
            //2. Get model
            int fileDataId = doodadDef.nameId;
            objectLods[0].m2Objects[i] = m_mapApi->getM2Object(fileDataId, doodadDef);
        } else {
            std::string fileName = &m_adtFileObj->doodadNamesField[m_adtFileObj->mmid[doodadDef.nameId]];
            //2. Get model
            objectLods[0].m2Objects[i] = m_mapApi->getM2Object(fileName, doodadDef);
        }
    }

    //2. Load lod
    bool useLod1Version = m_adtFileObjLod->lod_levels_for_objects != nullptr;

    if (m_adtFileObjLod->lod_levels_for_objects != nullptr) {
        offset = m_adtFileObjLod->lod_levels_for_objects->m2LodOffset[2];
        length = m_adtFileObjLod->lod_levels_for_objects->m2LodLength[2];
    } else {
        length = 0;
    };
    objectLods[1].m2Objects = std::vector<std::shared_ptr<M2Object>>(length, nullptr);
    for (int j = 0, i = offset; i < offset+length; i++, j++) {
        //1. Get filename
        SMDoodadDef &doodadDef = m_adtFileObjLod->doodadDefObj1[i];
        if (doodadDef.flags.mddf_entry_is_filedata_id == 1) {
            //2. Get model
            int fileDataId = doodadDef.nameId;
            objectLods[1].m2Objects[i] = m_mapApi->getM2Object(fileDataId, doodadDef);
        } else {
            std::string fileName = &m_adtFileObj->doodadNamesField[m_adtFileObj->mmid[doodadDef.nameId]];
            //2. Get model
            objectLods[1].m2Objects[i] = m_mapApi->getM2Object(fileName, doodadDef);
        }
    }
}
void AdtObject::loadWmos() {
    uint32_t offset = 0;
    int32_t length = m_adtFileObj->mapObjDef_len;

    //1. Load non lod
    objectLods[0].wmoObjects = std::vector<std::shared_ptr<WmoObject>>(length, nullptr);

    for (int j = 0, i = offset; i < offset + length; i++, j++) {
        //1. Get filename
        auto &mapDef = m_adtFileObj->mapObjDef[i];
        if (!mapDef.flags.modf_entry_is_filedata_id) {
            std::string fileName;
            fileName = &m_adtFileObj->wmoNamesField[m_adtFileObj->mwid[mapDef.nameId]];
            objectLods[0].wmoObjects[j] = m_mapApi->getWmoObject(fileName, mapDef);
        } else {
            uint32_t fileDataId = mapDef.nameId;
            objectLods[0].wmoObjects[j] = m_mapApi->getWmoObject(fileDataId, mapDef);
        }
//        std::cout << "wmo filename = "<< fileName << std::endl;
    }

    //2. Load lod!
    offset = 0;
    length = m_adtFileObjLod->mapObjDefObj1_len;
    bool useLod1Version = m_adtFileObjLod->lod_levels_for_objects != nullptr;
    if (m_adtFileObjLod->lod_levels_for_objects != nullptr) {
        offset = m_adtFileObjLod->lod_levels_for_objects->wmoLodOffset[2];
        length = m_adtFileObjLod->lod_levels_for_objects->wmoLodLength[2];
    } else {
        length = 0;
    }
    objectLods[1].wmoObjects = std::vector<std::shared_ptr<WmoObject>>(length, nullptr);
    for (int j = 0, i = offset; i < offset + length; i++, j++) {
        //Load Lods
        std::string fileName;
        auto &mapDefLod = m_adtFileObjLod->mapObjDefObj1[i];
        if (mapDefLod.flags.modf_entry_is_filedata_id == 0) {
            fileName = &m_adtFileObj->wmoNamesField[m_adtFileObj->mwid[mapDefLod.nameId]];
            objectLods[1].wmoObjects[j] = m_mapApi->getWmoObject(fileName, mapDefLod);
        } else {
            uint32_t fileDataId = mapDefLod.nameId;
            objectLods[1].wmoObjects[j] = m_mapApi->getWmoObject(fileDataId, mapDefLod);
        }
    }
}
void AdtObject::loadWater() {
    if (m_adtFile->mH2OHeader == nullptr) return;

    //Parse the blob
    PACK(
    struct LiquidVertexFormat {
        mathfu::vec4_packed pos_transp;
        mathfu::vec2_packed uv;
    });
    std::vector<LiquidVertexFormat> vertexBuffer;
    std::vector<uint16_t > indexBuffer;

    const float TILESIZE =  533.3433333f ;
    const float CHUNKSIZE = TILESIZE / 16.0f;
    const float UNITSIZE =  CHUNKSIZE / 8.0f;

    mathfu::vec3 adtBasePos = mathfu::vec3(AdtIndexToWorldCoordinate(adt_y), AdtIndexToWorldCoordinate(adt_x), 0);

    int basetextureFDID = 0;
    mathfu::vec3 color = mathfu::vec3(1,1,1);
    for (int y_chunk = 0; y_chunk < 16; y_chunk++) {
        for (int x_chunk = 0; x_chunk < 16; x_chunk++) {
            auto &liquidChunk = m_adtFile->mH2OHeader->chunks[y_chunk*16 + x_chunk];

            if (liquidChunk.layer_count == 0) continue;

            auto *liquidInstPtr =
                ((SMLiquidInstance *)(&m_adtFile->mH2OBlob[liquidChunk.offset_instances - m_adtFile->mH2OblobOffset]));

            mathfu::vec3 liquidBasePos =
                adtBasePos -
                mathfu::vec3(
                    CHUNKSIZE*y_chunk,
                    CHUNKSIZE*x_chunk,
                0);

            for (int layerInd = 0; layerInd < liquidChunk.layer_count; layerInd++) {
                SMLiquidInstance &liquidInstance = liquidInstPtr[layerInd];

                float *heightPtr = nullptr;
                if (liquidInstance.offset_vertex_data != 0) {
                    heightPtr = ((float *) (&m_adtFile->mH2OBlob[liquidInstance.offset_vertex_data - m_adtFile->mH2OblobOffset]));
                }

                //SmallHack
                if (basetextureFDID == 0) {
                    if (liquidInstance.liquid_object_or_lvf > 42) {
                        std::vector<LiquidMat> liqMats;
                        m_api->databaseHandler->getLiquidObjectData(liquidInstance.liquid_object_or_lvf, liqMats);
                        for (auto &liqMat : liqMats) {
                            if (liqMat.FileDataId != 0) {
                                basetextureFDID = liqMat.FileDataId;
                                if (liqMat.color1[0] > 0 || liqMat.color1[1] > 0 || liqMat.color1[2] > 0) {
                                    color = mathfu::vec3(liqMat.color1[0], liqMat.color1[1], liqMat.color1[2]);
                                }
                                break;
                            }
                        }
                    } else {
                        std::vector<int> fileDataIds;
                        m_api->databaseHandler->getLiquidTypeData(liquidInstance.liquid_type, fileDataIds);
                        for (auto fdid: fileDataIds) {
                            if (fdid != 0) {
                                basetextureFDID = fdid;
                                break;
                            }
                        }
                    }
                }


                int baseVertexIndForInst = vertexBuffer.size();

                for (int y = 0; y < liquidInstance.height + 1; y++) {
                    for (int x = 0; x < liquidInstance.width + 1; x++) {
                        mathfu::vec3 pos =
                            liquidBasePos -
                            mathfu::vec3(
                                UNITSIZE*(y+liquidInstance.y_offset),
                                UNITSIZE*(x+liquidInstance.x_offset),
                                -liquidInstance.min_height_level
                            );

                        bool hackBool = !((liquidInstance.liquid_object_or_lvf == 42) && (liquidInstance.liquid_type == 2));
                        if (liquidInstance.liquid_object_or_lvf != 2 && heightPtr!= nullptr && hackBool) {
                            pos.z = heightPtr[y * (liquidInstance.width + 1) + x];
                        }

                        LiquidVertexFormat vertex;
                        vertex.pos_transp = mathfu::vec4(pos, 1.0);
                        vertex.uv = mathfu::vec2(0,0);

                        vertexBuffer.push_back(vertex);
                    }
                }

                for (int y = 0; y < liquidInstance.height; y++) {
                    for (int x = 0; x < liquidInstance.width; x++) {
                        int16_t vertindexes[4] = {
                            (int16_t) (baseVertexIndForInst + y * (liquidInstance.width +1 ) + x),
                            (int16_t) (baseVertexIndForInst + y * (liquidInstance.width + 1) + x + 1),
                            (int16_t) (baseVertexIndForInst + (y + 1) * (liquidInstance.width + 1) + x),
                            (int16_t) (baseVertexIndForInst + (y + 1) * (liquidInstance.width + 1) + x + 1),
                        };

                        indexBuffer.push_back (vertindexes[0]);
                        indexBuffer.push_back (vertindexes[1]);
                        indexBuffer.push_back (vertindexes[2]);

                        indexBuffer.push_back (vertindexes[1]);
                        indexBuffer.push_back (vertindexes[3]);
                        indexBuffer.push_back (vertindexes[2]);
                    }
                }
            }
        }
    }

    std::shared_ptr<IDevice> device = m_api->hDevice;

    waterIBO = device->createIndexBuffer();
    waterIBO->uploadData(
        indexBuffer.data(),
        indexBuffer.size() * sizeof(uint16_t));

    waterVBO = device->createVertexBuffer();
    waterVBO->uploadData(
        vertexBuffer.data(),
        vertexBuffer.size() * sizeof(LiquidVertexFormat)
    );

    vertexWaterBufferBindings = device->createVertexBufferBindings();
    vertexWaterBufferBindings->setIndexBuffer(waterIBO);

    GVertexBufferBinding vertexBinding;
    vertexBinding.vertexBuffer = waterVBO;

    vertexBinding.bindings = std::vector<GBufferBinding>(&staticWaterBindings[0], &staticWaterBindings[2]);

    vertexWaterBufferBindings->addVertexBufferBinding(vertexBinding);
    vertexWaterBufferBindings->save();


//Create mesh(es)
    HGShaderPermutation shaderPermutation = m_api->hDevice->getShader("adtWater", nullptr);

    gMeshTemplate meshTemplate(vertexWaterBufferBindings, shaderPermutation);

    meshTemplate.meshType = MeshType::eWmoMesh;
    meshTemplate.depthWrite = false;
    meshTemplate.depthCulling = true;
    meshTemplate.backFaceCulling = false;

    meshTemplate.blendMode = EGxBlendEnum::GxBlend_Alpha;

    meshTemplate.textureCount = 1;
    if (basetextureFDID != 0) {

        auto htext = m_api->cacheStorage->getTextureCache()->getFileId(basetextureFDID);
        meshTemplate.texture[0] = m_api->hDevice->createBlpTexture(htext, true, true);
    } else {
        meshTemplate.texture[0] = m_api->hDevice->getBlackTexturePixel();
    }

    meshTemplate.ubo[0] = nullptr; //m_api->getSceneWideUniformBuffer();
    meshTemplate.ubo[1] = nullptr;
    meshTemplate.ubo[2] = nullptr;

    meshTemplate.ubo[3] = nullptr;
    meshTemplate.ubo[4] = device->createUniformBufferChunk(16);

    meshTemplate.start = 0;
    meshTemplate.end = indexBuffer.size();
    meshTemplate.element = DrawElementMode::TRIANGLES;

    auto l_liquidType = 0;
    meshTemplate.ubo[4]->setUpdateHandler([this, l_liquidType, color](IUniformBufferChunk* self) -> void {
//        int (&waterType)[4] = self->getObject<int[4]>();

        mathfu::vec4 closeRiverColor = this->m_api->getConfig()->getCloseRiverColor();
        mathfu::vec4_packed &color_ = self->getObject<mathfu::vec4_packed>();

        color_ = mathfu::vec4(closeRiverColor.xyz(), 0.7);
    });


    waterMesh = m_api->hDevice->createMesh(meshTemplate);


}


void AdtObject::createVBO() {
    /* 1. help index + Heights + texCoords +  */
    std::vector<float> vboArray ;

    //DEBUG
//    //Interate MLLL
//    for (int i = 0; i < m_adtFileLod->mlll_len; i++) {
//        std::cout<< "MLLL index "<< i << std::endl << std::flush;
//        MLLL & mlll = m_adtFileLod->mllls[i];
//
//        int max_index = -999999;
//        int min_index = 999999;
//
//        for (int j = mlll.height_index; j < mlll.height_index+mlll.height_length; j++) {
//            int index = m_adtFileLod->mvli_indicies[j];
//            if (index < min_index) min_index = index;
//            if (index > max_index) max_index = index;
//        }
//
//        std::cout<< "min_index = "<< min_index << " max_index = " << max_index<< std::endl << std::flush;
//    }

//    int max_index = -999999;
//    int min_index = 999999;
//    for (int i = 0; i < m_adtFileLod->mlsi_len; i++) {
//        int index = m_adtFileLod->mlsi_indicies[i];
//        if (index < min_index) min_index = index;
//        if (index > max_index) max_index = index;
//    }
//    std::cout<< "mlsi: min_index = "<< min_index << " max_index = " << max_index<< std::endl << std::flush;

    const float UNITSIZE =  (1600.0 / 3.0) / 16.0 / 8.0;
    for (int i = 0; i <= m_adtFile->mcnkRead; i++) {
        for (int j = 0; j < 9 * 9 + 8 * 8; j++) {
            /* 1.1 help index */
            vboArray.push_back((float)j);
            /* 1.2 Heights */
            float iX = fmod(j, 17.0);
            float iY = floor(j/17.0);

            if (iX > 8.01f) {
                iY = iY + 0.5;
                iX = iX - 8.5;
            }
            mathfu::vec3 pos = mathfu::vec3(
                m_adtFile->mapTile[i].position.x - iY * UNITSIZE,
                m_adtFile->mapTile[i].position.y - iX * UNITSIZE,
                m_adtFile->mapTile[i].position.z + m_adtFile->mcnkStructs[i].mcvt->height[j] );

            vboArray.push_back(pos.x);
            vboArray.push_back(pos.y);
            vboArray.push_back(pos.z);

//            std::cout << " i = " << i << " j =  " << j << " pos = " << pos.x << ", " << pos.y << ", " << pos.z << std::endl;

            /* 1.3 Normals */
            for (int k = 0; k < 3; k++) {
                vboArray.push_back(m_adtFile->mcnkStructs[i].mcnr->entries[j].normal[k] / 127.0f);
            }
            /* 1.4 MCCV */ //Color vertex
            if (m_adtFile->mcnkStructs[i].mccv != nullptr) {
                auto &mccv = m_adtFile->mcnkStructs[i].mccv;
                vboArray.push_back(mccv->entries[j].red / 255.0f);
                vboArray.push_back(mccv->entries[j].green / 255.0f);
                vboArray.push_back(mccv->entries[j].blue / 255.0f);
                vboArray.push_back(mccv->entries[j].alpha / 255.0f);
            } else {
                vboArray.push_back(0.5f);
                vboArray.push_back(0.5f);
                vboArray.push_back(0.5f);
                vboArray.push_back(0.5f);
            }
            /* 1.4 MCLV */ //Lightning Vertex
            if (m_adtFile->mcnkStructs[i].mclv != nullptr) {
                auto &mclv = m_adtFile->mcnkStructs[i].mclv;
                vboArray.push_back(mclv->values[j].b / 255.0f);
                vboArray.push_back(mclv->values[j].g / 255.0f);
                vboArray.push_back(mclv->values[j].r / 255.0f);
                vboArray.push_back(mclv->values[j].a / 255.0f);
            } else {
                //If MCLV is empty, localDiffuse doesnt exist in shader
                vboArray.push_back(0.0f);
                vboArray.push_back(0.0f);
                vboArray.push_back(0.0f);
                vboArray.push_back(0.0f);
            }
        }
    }

    /* 1.3 Make combinedVbo */
    std::shared_ptr<IDevice> device = m_api->hDevice;
    combinedVbo = device->createVertexBuffer();
    combinedVbo->uploadData(vboArray.data(), vboArray.size()*sizeof(float));

    /* 2. Strips */
    stripVBO = device->createIndexBuffer();
    if (m_adtFile->strips.size() > 0) {
        stripVBO->uploadData(m_adtFile->strips.data(), m_adtFile->strips.size() * sizeof(int16_t));
    } else {
        stripVBO->uploadData(nullptr, 0);
    }

    adtVertexBindings = device->createVertexBufferBindings();
    adtVertexBindings->setIndexBuffer(stripVBO);

    GVertexBufferBinding vertexBinding;
    vertexBinding.vertexBuffer = combinedVbo;
    vertexBinding.bindings = std::vector<GBufferBinding>(&bufferBinding[0], &bufferBinding[5]);

    adtVertexBindings->addVertexBufferBinding(vertexBinding);
    adtVertexBindings->save();

    //Sometimes mvli can be zero, while there is still data in floatDataBlob
    if (m_adtFileLod!= nullptr && m_adtFileLod->getStatus()==FileStatus::FSLoaded && m_adtFileLod->floatDataBlob_len > 0 && m_adtFileLod->mvli_len > 0) {
        //Generate MLLL buffers
        //Index buffer for lod
        std::vector<float> vboLod;

        for (int i = 0; i < m_adtFileLod->floatDataBlob_len ; i++) {
            vboLod.push_back(this->m_adtFileLod->floatDataBlob[i]);
        }
        uint32_t indexVBOLodOffset = vboLod.size();
        for (int i = 0; i < (129 * 129 + 128 * 128); i++) {
            vboLod.push_back((float) i);
        }

        heightVboLod = device->createVertexBuffer();
        heightVboLod->uploadData(&vboLod[0], vboLod.size()*sizeof(float));

        /* 2. Index buffer */
        stripVBOLod = device->createIndexBuffer();
        stripVBOLod->uploadData(&m_adtFileLod->mvli_indicies[0],  m_adtFileLod->mvli_len * sizeof(int16_t));


        lodVertexBindings = device->createVertexBufferBindings();
        lodVertexBindings->setIndexBuffer(stripVBOLod);

        GVertexBufferBinding vertexBinding;
        vertexBinding.vertexBuffer = combinedVbo;

		GBufferBinding bufferBinding = { +adtLodShader::Attribute::aHeight, 1, GBindingType::GFLOAT, false, 4, 0 };
		vertexBinding.bindings.push_back(bufferBinding);
		bufferBinding = { +adtLodShader::Attribute::aIndex, 1, GBindingType::GFLOAT, false, 4, static_cast<uint32_t>(indexVBOLodOffset * sizeof(float))};
        vertexBinding.bindings.push_back(bufferBinding);

        lodVertexBindings->addVertexBufferBinding(vertexBinding);
        lodVertexBindings->save();
    }
}

void AdtObject::calcBoundingBoxes() {
    for (int i = 0; i < 256; i++) {
        mcnkStruct_t *mcnkContent = &this->m_adtFile->mcnkStructs[i];
        SMChunk *mcnkChunk = &m_adtFile->mapTile[i];

        //Loop over heights
        float minZ = 999999;
        float maxZ = -999999;
        for (int j = 0; j < 8*8+9*9; j++) {
            float heightVal = mcnkContent->mcvt->height[j];
            if (minZ > heightVal) minZ = heightVal;
            if (maxZ < heightVal) maxZ = heightVal;
        }

        float minX = mcnkChunk->position.x - (533.3433333 / 16.0);
        float maxX = mcnkChunk->position.x;
        float minY = mcnkChunk->position.y - (533.3433333 / 16.0);
        float maxY = mcnkChunk->position.y;
        minZ += mcnkChunk->position.z;
        maxZ += mcnkChunk->position.z;

        this->tileAabb[i] = CAaBox(
            C3Vector(mathfu::vec3(minX, minY, minZ)),
            C3Vector(mathfu::vec3(maxX, maxY, maxZ))
        );

        this->globIndexY[i] = worldCoordinateToGlobalAdtChunk((minX + maxX) / 2.0f);
        this->globIndexX[i] = worldCoordinateToGlobalAdtChunk((minY + maxY) / 2.0f);
    }
}

void AdtObject::createMeshes() {
    std::shared_ptr<IDevice> device = m_api->hDevice;

    auto adtFileTex = m_adtFileTex;
    auto adtFile = m_adtFile;

    adtWideBlockPS = m_api->hDevice->createUniformBufferChunk(sizeof(adtModelWideBlockPS));

    auto api = m_api;
    adtWideBlockPS->setUpdateHandler([api](IUniformBufferChunk *self){
        auto *adtWideblockPS = &self->getObject<adtModelWideBlockPS>();
        adtWideblockPS->uViewUp = mathfu::vec4_packed(mathfu::vec4(api->getViewUp(), 0.0));;
        adtWideblockPS->uSunDir_FogStart = mathfu::vec4_packed(
            mathfu::vec4(api->getGlobalSunDir(), api->getGlobalFogStart()));
        adtWideblockPS->uSunColor_uFogEnd = mathfu::vec4_packed(
            mathfu::vec4(api->getGlobalSunColor().xyz(), api->getGlobalFogEnd()));
        adtWideblockPS->uAmbientLight = api->getGlobalAmbientColor();
        adtWideblockPS->FogColor = mathfu::vec4_packed(mathfu::vec4(api->getGlobalFogColor().xyz(), 0));
    });

    for (int i = 0; i < 256; i++) {
        //Cant be used only in Wotlk
        //if (m_adtFile->mapTile[i].nLayers <= 0) continue;
        bool noLayers = m_adtFileTex->mcnkStructs[i].mcly == nullptr || m_adtFileTex->mcnkStructs[i].mclyCnt <= 0;

        HGShaderPermutation hgShaderPermutation = device->getShader("adtShader", nullptr);
        gMeshTemplate aTemplate(adtVertexBindings, hgShaderPermutation);

        aTemplate.meshType = MeshType::eAdtMesh;
        aTemplate.triCCW = 1;
        aTemplate.depthWrite = 1;
        aTemplate.depthCulling = 1;
        aTemplate.backFaceCulling = 1;
        aTemplate.blendMode = EGxBlendEnum::GxBlend_Opaque;

        aTemplate.start = m_adtFile->stripOffsets[i] * 2;
        aTemplate.end = m_adtFile->stripOffsets[i + 1] - m_adtFile->stripOffsets[i];
        aTemplate.element = DrawElementMode::TRIANGLES;

        aTemplate.ubo[0] = nullptr; //m_api->getSceneWideUniformBuffer();
        aTemplate.ubo[1] = nullptr;
        aTemplate.ubo[2] = m_api->hDevice->createUniformBufferChunk(sizeof(adtMeshWideBlockVS));
        aTemplate.ubo[3] = adtWideBlockPS;
        aTemplate.ubo[4] = m_api->hDevice->createUniformBufferChunk(sizeof(adtMeshWideBlockPS));

        aTemplate.textureCount = 9;

        aTemplate.texture = std::vector<HGTexture>(aTemplate.textureCount, nullptr);

        aTemplate.ubo[4]->setUpdateHandler([&api, adtFileTex, noLayers, i](IUniformBufferChunk *self){
            auto &blockPS = self->getObject<adtMeshWideBlockPS>();
            for (int j = 0; j < 4; j++) {
                blockPS.uHeightOffset[j] = 0.0f;
                blockPS.uHeightScale[j] = 1.0f;
            }
            if ((adtFileTex->mtxp_len > 0) && !noLayers) {
                for (int j = 0; j < adtFileTex->mcnkStructs[i].mclyCnt; j++) {
                    auto const &textureParams = adtFileTex->mtxp[adtFileTex->mcnkStructs[i].mcly[j].textureId];

                    blockPS.uHeightOffset[j] = textureParams.heightOffset;
                    blockPS.uHeightScale[j] = textureParams.heightScale;
                }
            }
        });

        aTemplate.ubo[2]->setUpdateHandler([this, i](IUniformBufferChunk *self){
            auto &blockVS = self->getObject<adtMeshWideBlockVS>();
            blockVS.uPos = mathfu::vec4(
                this->m_adtFile->mapTile[i].position.x,
                this->m_adtFile->mapTile[i].position.y,
                this->m_adtFile->mapTile[i].position.z,
                0
            );
        });


        if (m_adtFileTex->mtxp_len > 0 && !noLayers) {
            for (int j = 0; j < m_adtFileTex->mcnkStructs[i].mclyCnt; j++) {
                auto const &textureParams = m_adtFileTex->mtxp[m_adtFileTex->mcnkStructs[i].mcly[j].textureId];

                HGTexture layer_height = nullptr;
                if (textureParams.flags.do_not_load_specular_or_height_texture_but_use_cubemap == 0) {
                    if (!feq(textureParams.heightOffset, 1.0f) && !feq(textureParams.heightScale, 0.0)) {
                        layer_height = getAdtHeightTexture(m_adtFileTex->mcnkStructs[i].mcly[j].textureId);
                    }
                }

                aTemplate.texture[j + 5] = layer_height;
            }
        } else {
            for (int j = 0; j < 4; j++) {
                aTemplate.texture[j + 5] = device->getWhiteTexturePixel();
            }
        }

        if (!noLayers) {
            aTemplate.texture[4] = alphaTextures[i];
        } else {
            aTemplate.texture[4] = device->getBlackTexturePixel();
        }

        if (!noLayers) {
            for (int j = 0; j < m_adtFileTex->mcnkStructs[i].mclyCnt; j++) {
                auto &layerDef = m_adtFileTex->mcnkStructs[i].mcly[j];

                HGTexture layer_x = getAdtTexture(m_adtFileTex->mcnkStructs[i].mcly[j].textureId);
//            BlpTexture &layer_spec = getAdtSpecularTexture(m_adtFileTex->mcnkStructs[i].mcly[j].textureId);
                aTemplate.texture[j] = layer_x;
            }
        } else {
            for (int j = 0; j < 4; j++) {
                aTemplate.texture[j] = device->getWhiteTexturePixel();
            }
        }

        HGMesh hgMesh = device->createMesh(aTemplate);
        adtMeshes.push_back(hgMesh);
    }
}

void AdtObject::loadAlphaTextures() {
    //int chunkCount = m_adtFile->mcnkRead+1;
    int chunkCount = m_adtFileTex->mcnkRead+1;
    int maxAlphaTexPerChunk = 4;
    int alphaTexSize = 64;

    int texWidth = alphaTexSize;
    int texHeight = alphaTexSize;

    int createdThisRun = 0;
    for (int i = 0; i < chunkCount; i++) {
        HGTexture alphaTexture = m_api->hDevice->createTexture();
        std::vector<uint8_t> alphaTextureData;
        m_adtFileTex->processTexture(m_wdtFile->mphd->flags, i, alphaTextureData);

        alphaTexture->loadData(texWidth, texHeight, &alphaTextureData[0]);

        alphaTextures.push_back(alphaTexture);
    }
    this->alphaTexturesLoaded += createdThisRun;
}



void AdtObject::collectMeshes(ADTObjRenderRes &adtRes, std::vector<HGMesh> &renderedThisFrame, int renderOrder) {
    m_lastTimeOfUpdateOrRefCheck = m_mapApi->getCurrentSceneTime();

    if (!m_loaded) return;

    adtRes.wasLoaded = true;

    size_t meshCount = adtMeshes.size();
    for (int i = 0; i < meshCount; i++) {
        if (!adtRes.drawChunk[i]) continue;
        adtMeshes[i]->setRenderOrder(renderOrder);
        renderedThisFrame.push_back(adtMeshes[i]);
    }
    if (waterMesh!= nullptr) {
        renderedThisFrame.push_back(waterMesh);
    }
}
void AdtObject::collectMeshesLod(std::vector<HGMesh> &renderedThisFrame) {
   /*
    if (!m_loaded) return;
    if (lodCommands.size() <= 0) return;

    GLuint blackPixelTexture = this->m_api->getBlackPixelTexture();
    ShaderRuntimeData *adtLodShader = this->m_api->getAdtLodShader();

    mathfu::vec3 adtPos = mathfu::vec3(m_adtFile->mapTile[m_adtFile->mcnkMap[0][0]].position);
    glUniform3f(adtLodShader->getUnf("uPos"),
                adtPos.x,
                adtPos.y,
                adtPos.z);

    glBindBuffer(GL_ARRAY_BUFFER, this->heightVboLod);
    glVertexAttribPointer(+adtLodShader::Attribute::aHeight, 1, GL_FLOAT, GL_FALSE, 4, nullptr);

    glBindBuffer(GL_ARRAY_BUFFER, this->indexVBOLod);
    glVertexAttribPointer(+adtLodShader::Attribute::aIndex, 1, GL_FLOAT, GL_FALSE, 4, nullptr);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->stripVBOLod);

    glActiveTexture(GL_TEXTURE0);
    if (lodDiffuseTexture->getIsLoaded()) {
        glBindTexture(GL_TEXTURE_2D, lodDiffuseTexture->getGlTexture());
    } else {
        glBindTexture(GL_TEXTURE_2D, blackPixelTexture);
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glActiveTexture(GL_TEXTURE1);
    if (lodNormalTexture->getIsLoaded()) {
        glBindTexture(GL_TEXTURE_2D, lodNormalTexture->getGlTexture());
    } else {
        glBindTexture(GL_TEXTURE_2D, blackPixelTexture);
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    for (int i = 0; i < lodCommands.size(); i++) {
        glDrawElements(GL_TRIANGLES, lodCommands[i].length, GL_UNSIGNED_SHORT, (void *)(lodCommands[i].index * 2));
    }
    */
}

void AdtObject::doPostLoad() {
//    std::cout << "AdtObject::doPostLoad finished called" << std::endl;
    if (!m_loaded) {
        if (m_adtFile->getStatus()==FileStatus::FSLoaded &&
            m_adtFileObj->getStatus()==FileStatus::FSLoaded &&
            m_adtFileObjLod->getStatus()==FileStatus::FSLoaded &&
            ((m_adtFileLod != nullptr && m_adtFileLod->getStatus()==FileStatus::FSLoaded) || !m_wdtFile->mphd->flags.unk_0x0100) &&
            m_adtFileTex->getStatus()==FileStatus::FSLoaded) {
            this->loadingFinished();
            m_loaded = true;
        }
    }
}
void AdtObject::update() {
    m_lastTimeOfUpdateOrRefCheck = m_mapApi->getCurrentSceneTime();
//    std::cout << "AdtObject::update finished called" << std::endl;
    if (!m_loaded) return;
    if (adtWideBlockPS == nullptr) return;

}

void AdtObject::uploadGeneratorBuffers(ADTObjRenderRes &adtRes) {
    if (!m_loaded) return;

    if (!adtRes.wasLoaded) return;



    for (int i = 0; i < adtMeshes.size(); i++) {
        bool noLayers = m_adtFileTex->mcnkStructs[i].mcly == nullptr || m_adtFileTex->mcnkStructs[i].mclyCnt <= 0;




    }

}

HGTexture AdtObject::getAdtTexture(int textureId) {
    auto item = m_requestedTextures.find(textureId);
    if (item != m_requestedTextures.end()) {
        return item->second;
    }

    HBlpTexture texture;
    if (m_adtFileTex->textureNames.size() != 0) {
        std::string &materialTexture = m_adtFileTex->textureNames[textureId];
        texture = m_api->cacheStorage->getTextureCache()->get(materialTexture);
    } else if (textureId < m_adtFileTex->mdid_len) {
        uint32_t filedataId = m_adtFileTex->mdid[textureId];
        texture = m_api->cacheStorage->getTextureCache()->getFileId(filedataId);
    }

    HGTexture h_gblpTexture = m_api->hDevice->createBlpTexture(texture, true, true);
    m_requestedTextures[textureId] = h_gblpTexture;

    return h_gblpTexture;
}

HGTexture AdtObject::getAdtHeightTexture(int textureId) {
    auto item = m_requestedTexturesHeight.find(textureId);
    if (item != m_requestedTexturesHeight.end()) {
        return item->second;
    }

    HBlpTexture texture;
    if (m_adtFileTex->textureNames.size() != 0) {
        std::string &materialTexture = m_adtFileTex->textureNames[textureId];
        std::string matHeightText = materialTexture.substr(0, materialTexture.size() - 4) + "_h.blp";

        texture = m_api->cacheStorage->getTextureCache()->get(matHeightText);
    } else if (textureId < m_adtFileTex->mhid_len) {
        uint32_t filedataId = m_adtFileTex->mhid[textureId];
        texture = m_api->cacheStorage->getTextureCache()->getFileId(filedataId);
    }

    HGTexture h_gblpTexture = m_api->hDevice->createBlpTexture(texture, true, true);
    m_requestedTexturesHeight[textureId] = h_gblpTexture;

    return h_gblpTexture;
}

HGTexture AdtObject::getAdtSpecularTexture(int textureId) {
    auto item = m_requestedTexturesSpec.find(textureId);
    if (item != m_requestedTexturesSpec.end()) {
        return item->second;
    }

    std::string &materialTexture = m_adtFileTex->textureNames[textureId];

    std::string matHeightText = materialTexture.substr(0, materialTexture.size() - 4) + "_s.blp";

    HBlpTexture texture = m_api->cacheStorage->getTextureCache()->get(matHeightText);
    HGTexture h_gblpTexture = m_api->hDevice->createBlpTexture(texture, true, true);
    m_requestedTexturesSpec[textureId] = h_gblpTexture;

    return h_gblpTexture;
}

const float dist = 533.0f/16;
static const float perLodDist[5] = {9999999999.99f,
                                    std::pow(dist*60, 2.0f),        //32
                                    std::pow(dist*40, 2.0f), //16
                                    std::pow(dist*20, 2.0f), //8
                                    std::pow(dist*10, 2.0f), //4
//                                  std::pow(dist / (16), 2) //2
};


bool AdtObject::iterateQuadTree(ADTObjRenderRes &adtFrustRes, mathfu::vec4 &camera, const mathfu::vec3 &pos,
                                float x_offset, float y_offset, float cell_len,
                                int curentLod, int lastFoundLod,
                                const PointerChecker<MLND> &quadTree, int quadTreeInd,
                                std::vector<mathfu::vec4> &frustumPlanes,
                                std::vector<mathfu::vec3> &frustumPoints,
                                std::vector<mathfu::vec3> &hullLines,
                                mathfu::mat4 &lookAtMat4,
                                std::vector<std::shared_ptr<M2Object>> &m2ObjectsCandidates,
                                std::vector<std::shared_ptr<WmoObject>> &wmoCandidates) {



    bool drawLodBasedOnDist = false;
    const MLND *quadTreeNode = nullptr;
    int foundLod = lastFoundLod;

    if (quadTree == nullptr || quadTreeInd == -1 || curentLod == 4) {
        mathfu::vec2 aabb2D[2];
        aabb2D[0] = pos.xy() - mathfu::vec2(533.3333f * (x_offset + cell_len), 533.3333f*(y_offset + cell_len));
        aabb2D[1] = pos.xy() -  mathfu::vec2(533.3333f *x_offset, 533.3333f*y_offset);
        mathfu::vec2 point = camera.xy();

        //General frustum cull!
        bool atLeastOneIsDrawn = false;

        atLeastOneIsDrawn = checkNonLodChunkCulling(adtFrustRes, camera, frustumPlanes, frustumPoints, hullLines,
                                                         16.0f * x_offset, 16.0f * y_offset,
                                                         16.0f * cell_len, 16.0f * cell_len
        );

        checkReferences(adtFrustRes, camera, frustumPlanes, frustumPoints,
                        lookAtMat4, 5,
                        m2ObjectsCandidates, wmoCandidates,
                        16.1f * x_offset, 16.1f * y_offset,
                        16.1f * cell_len, 16.1f * cell_len);

        return atLeastOneIsDrawn;
    }
    quadTreeNode = &quadTree[quadTreeInd];

    if (foundLod == curentLod) {
        quadTreeNode = &quadTree[quadTreeInd];

        mathfu::vec2 aabb2D[2];
        aabb2D[0] = pos.xy() - mathfu::vec2(533.3333f * (x_offset + cell_len), 533.3333f*(y_offset + cell_len));
        aabb2D[1] = pos.xy() -  mathfu::vec2(533.3333f *x_offset, 533.3333f*y_offset);

        mathfu::vec2 point = camera.xy();
        float dist = MathHelper::distanceFromAABBToPoint2DSquared(aabb2D, point);

        while (dist < perLodDist[foundLod] && foundLod < 5) foundLod++;
    }



    if (quadTreeNode != nullptr && foundLod > curentLod) {
        //check all others
        float newCellLen = cell_len/2.0f;
        bool atLeastOneIsDrawn = false;

        //1. Node 1
        atLeastOneIsDrawn |= iterateQuadTree(adtFrustRes, camera, pos, x_offset, y_offset , newCellLen, curentLod + 1, foundLod, quadTree, quadTreeNode->indices[0],
                                            frustumPlanes, frustumPoints, hullLines, lookAtMat4, m2ObjectsCandidates, wmoCandidates);
        //2. Node 2
        atLeastOneIsDrawn |= iterateQuadTree(adtFrustRes,camera, pos, x_offset, y_offset + newCellLen, newCellLen, curentLod + 1, foundLod, quadTree, quadTreeNode->indices[1],
                                            frustumPlanes, frustumPoints, hullLines, lookAtMat4, m2ObjectsCandidates, wmoCandidates);
        //3. Node 3
        atLeastOneIsDrawn |= iterateQuadTree(adtFrustRes,camera, pos, x_offset + newCellLen, y_offset, newCellLen, curentLod + 1, foundLod, quadTree, quadTreeNode->indices[2],
                                            frustumPlanes, frustumPoints, hullLines, lookAtMat4, m2ObjectsCandidates, wmoCandidates);
        //4. Node 4
        atLeastOneIsDrawn |= iterateQuadTree(adtFrustRes, camera, pos, x_offset+ newCellLen, y_offset+ newCellLen, newCellLen, curentLod + 1, foundLod, quadTree, quadTreeNode->indices[3],
                                            frustumPlanes, frustumPoints, hullLines, lookAtMat4, m2ObjectsCandidates, wmoCandidates);

        return atLeastOneIsDrawn;
    } else {
            //Push the drawCall for this lod
            LodCommand command;
            command.index = quadTreeNode->index;
            command.length = quadTreeNode->length;

            lodCommands.push_back(command);

            checkReferences(adtFrustRes, camera, frustumPlanes, frustumPoints,
                            lookAtMat4, curentLod,
                            m2ObjectsCandidates, wmoCandidates,
                            16.0f * x_offset, 16.0f * y_offset,
                            16.0f * cell_len, 16.0f * cell_len);

            bool atLeastOneIsDrawn = true;

        return atLeastOneIsDrawn;
    }
}

bool AdtObject::checkNonLodChunkCulling(ADTObjRenderRes &adtFrustRes, mathfu::vec4 &cameraPos,
                                        std::vector<mathfu::vec4> &frustumPlanes,
                                        std::vector<mathfu::vec3> &frustumPoints,
                                        std::vector<mathfu::vec3> &hullLines,

                                        int x, int y, int x_len, int y_len) {

    bool atLeastOneIsDrawn = false;
    for (int k = x; k < x + x_len; k++) {
        for (int l = y; l < y + y_len; l++) {
            int i = this->m_adtFile->mcnkMap[l][k];
            if (i < 0)
                continue;

            mcnkStruct_t &mcnk = this->m_adtFile->mcnkStructs[i];
            CAaBox &aabb = this->tileAabb[i];
            adtFrustRes.drawChunk[i] = false;

            //1. Check if camera position is inside Bounding Box
            bool cameraOnChunk =
                    (cameraPos[0] > aabb.min.x && cameraPos[0] < aabb.max.x &&
                     cameraPos[1] > aabb.min.y && cameraPos[1] < aabb.max.y);
            if (cameraOnChunk &&
                cameraPos[2] > aabb.min.z && cameraPos[2] < aabb.max.z
                    ) {
                adtFrustRes.drawChunk[i] = true;
                atLeastOneIsDrawn = true;
            }

            //2. Check aabb is inside camera frustum
            bool result = false;
            adtFrustRes.checkRefs[i] = adtFrustRes.drawChunk[i];
            if (!adtFrustRes.drawChunk[i]) {
                result = MathHelper::checkFrustum(frustumPlanes, aabb, frustumPoints);
                bool frustum2DRes = MathHelper::checkFrustum2D(hullLines, aabb);
                adtFrustRes.checkRefs[i] = result || frustum2DRes;

                adtFrustRes.drawChunk[i] = result;
                atLeastOneIsDrawn = atLeastOneIsDrawn || result;
            }
        }
    }

    return atLeastOneIsDrawn;
}

bool AdtObject::checkReferences(
                          ADTObjRenderRes &adtFrustRes,
                          mathfu::vec4 &cameraPos,
                          std::vector<mathfu::vec4> &frustumPlanes,
                          std::vector<mathfu::vec3> &frustumPoints,
                          mathfu::mat4 &lookAtMat4,
                          int lodLevel,
                          std::vector<std::shared_ptr<M2Object>> &m2ObjectsCandidates,
                          std::vector<std::shared_ptr<WmoObject>> &wmoCandidates,
                          int x, int y, int x_len, int y_len) {
    m_lastTimeOfUpdateOrRefCheck = m_mapApi->getCurrentSceneTime();

    if (!m_loaded) return false;

    for (int k = x; k < x+x_len; k++) {
        for (int l = y; l < y + y_len; l++) {
            int i = this->m_adtFile->mcnkMap[k][l];

            bool wotlk = false;
            float chunkDist = 1.0;
            if (wotlk) {
//                SMChunk *mapTile = &m_adtFile->mapTile[i];
//                mcnkStruct_t *mcnkContent = &m_adtFile->mcnkStructs[i];
//
//
//                if (mcnkContent != nullptr && mcnkContent->mcrf.doodad_refs != nullptr) {
//                    for (int j = 0; j < mapTile->nDoodadRefs; j++) {
//                        uint32_t m2Ref = mcnkContent->mcrf.doodad_refs[j];
//                        m2ObjectsCandidates.insert(this->m2Objects[m2Ref]);
//                    }
//                }
//                if (mcnkContent != nullptr && mcnkContent->mcrf.object_refs != nullptr) {
//                    for (int j = 0; j < mapTile->nMapObjRefs; j++) {
//                        uint32_t wmoRef = mcnkContent->mcrf.object_refs[j];
//                        wmoCandidates.insert(this->wmoObjects[wmoRef]);
//                    }
//                }
//            for (int j = 0; j < this->m2Objects.size(); j++) {
//                m2ObjectsCandidates.insert(this->m2Objects[j]);
//            }
//            for (int j = 0; j < wmoObjects.size(); j++) {
//                uint32_t wmoRef = mcnkContent->mcrf.object_refs[j];
//                wmoCandidates.insert(this->wmoObjects[j]);
//            }
            } else {
                SMChunk *mapTile = &m_adtFile->mapTile[i];
                mcnkStruct_t *mcnkContent = &m_adtFileObj->mcnkStructs[i];
                if (lodLevel >= 4) {
                    if (mcnkContent->mcrd_doodad_refs_len > 0) {
                        for (int j = 0; j < mcnkContent->mcrd_doodad_refs_len; j++) {
                            uint32_t m2Ref = mcnkContent->mcrd_doodad_refs[j];
                            m2ObjectsCandidates.push_back(this->objectLods[0].m2Objects[m2Ref]);
                        }
                    }
                } else {
                    for (auto m2Object : this->objectLods[1].m2Objects) {
                        m2ObjectsCandidates.push_back(m2Object);
                    }
                }

                if (lodLevel >= 4) {
                    if (mcnkContent->mcrw_object_refs_len > 0) {
                        for (int j = 0; j < mcnkContent->mcrw_object_refs_len; j++) {
                            uint32_t wmoRef = mcnkContent->mcrw_object_refs[j];
                            wmoCandidates.push_back(this->objectLods[0].wmoObjects[wmoRef]);
                        }
                    }
                } else {
                    for (auto wmoObject : this->objectLods[1].wmoObjects) {
                        wmoCandidates.push_back(wmoObject);
                    }
                }
            }
        }
    }
	return true;
}

bool AdtObject::checkFrustumCulling(ADTObjRenderRes &adtFrustRes,
                                    mathfu::vec4 &cameraPos,
                                    int adt_glob_x,
                                    int adt_glob_y,
                                    std::vector<mathfu::vec4> &frustumPlanes,
                                    std::vector<mathfu::vec3> &frustumPoints,
                                    std::vector<mathfu::vec3> &hullLines,
                                    mathfu::mat4 &lookAtMat4,
                                    std::vector<std::shared_ptr<M2Object>> &m2ObjectsCandidates,
                                    std::vector<std::shared_ptr<WmoObject>> &wmoCandidates) {

    if (!this->m_loaded) return true;
    bool atLeastOneIsDrawn = false;

    mostDetailedLod = 5;
    leastDetiledLod = 0;

    for (int i = 0; i < 256; i++) {
        adtFrustRes.drawChunk[i] = false;
    }
    lodCommands.clear();

    //For new ADT with _lod.adt
    if (false) {
//    if (m_wdtFile->mphd->flags.unk_0x0100) {
        mathfu::vec3 adtPos = mathfu::vec3(m_adtFile->mapTile[m_adtFile->mcnkMap[0][0]].position);
        atLeastOneIsDrawn |= iterateQuadTree(adtFrustRes,
                                             cameraPos, adtPos,
                                             0, 0, 1.0,
                                             0, 0,
                                             m_adtFileLod->mlnds, 0,
                                             frustumPlanes, frustumPoints, hullLines, lookAtMat4, m2ObjectsCandidates,
                                             wmoCandidates);
    } else {
        //For old ADT without _lod.adt
        atLeastOneIsDrawn |= checkNonLodChunkCulling(adtFrustRes, cameraPos,
                                frustumPlanes, frustumPoints, hullLines,
                                0, 0, 16, 16);
        checkReferences(adtFrustRes, cameraPos, frustumPlanes, frustumPoints,
                        lookAtMat4, 5,
                        m2ObjectsCandidates, wmoCandidates,
                        0, 0,
                        16, 16);
    }


    return atLeastOneIsDrawn;
}

AdtObject::AdtObject(ApiContainer *api, std::string &adtFileTemplate, std::string mapname, int adt_x, int adt_y, HWdtFile wdtFile) : alphaTextures(), adt_x(adt_x), adt_y(adt_y){
    m_api = api;
    tileAabb = std::vector<CAaBox>(256);
    globIndexX = std::vector<int>(256);
    globIndexY = std::vector<int>(256);
    adtFileTemplate = adtFileTemplate;

    m_wdtFile = wdtFile;

    auto adtGeomCache = m_api->cacheStorage->getAdtGeomCache();

    m_adtFile =         adtGeomCache->get(adtFileTemplate+".adt");
    m_adtFile->setIsMain(true);
    m_adtFileTex =      adtGeomCache->get(adtFileTemplate+"_tex"+std::to_string(0)+".adt");
    m_adtFileObj =      adtGeomCache->get(adtFileTemplate+"_obj"+std::to_string(0)+".adt");
    m_adtFileObjLod =   adtGeomCache->get(adtFileTemplate+"_obj"+std::to_string(1)+".adt");
    m_adtFileLod =      adtGeomCache->get(adtFileTemplate+"_lod.adt");

    lodDiffuseTexture = m_api->cacheStorage->getTextureCache()->get("world/maptextures/"+mapname+"/"
        +mapname+"_"+std::to_string(adt_x)+"_"+std::to_string(adt_y)+".blp");
    lodNormalTexture = m_api->cacheStorage->getTextureCache()->get("world/maptextures/"+mapname+"/"
        +mapname+"_"+std::to_string(adt_x)+"_"+std::to_string(adt_y)+"_n.blp");

}

AdtObject::AdtObject(ApiContainer *api, int adt_x, int adt_y, WdtFile::MapFileDataIDs &fileDataIDs, HWdtFile wdtFile): adt_x(adt_x), adt_y(adt_y) {
    m_api = api;
    tileAabb = std::vector<CAaBox>(256);
    globIndexX = std::vector<int>(256);
    globIndexY = std::vector<int>(256);

    m_wdtFile = wdtFile;

    m_adtFile = m_api->cacheStorage->getAdtGeomCache()->getFileId(fileDataIDs.rootADT);
    m_adtFile->setIsMain(true);
    m_adtFileTex = m_api->cacheStorage->getAdtGeomCache()->getFileId(fileDataIDs.tex0ADT);
    m_adtFileObj = m_api->cacheStorage->getAdtGeomCache()->getFileId(fileDataIDs.obj0ADT);
    m_adtFileObjLod = m_api->cacheStorage->getAdtGeomCache()->getFileId(fileDataIDs.obj1ADT);
    if (fileDataIDs.lodADT != 0) {
        m_adtFileLod = m_api->cacheStorage->getAdtGeomCache()->getFileId(fileDataIDs.lodADT);
    } else {
        m_adtFileLod = nullptr;
    }

    lodDiffuseTexture = m_api->cacheStorage->getTextureCache()->getFileId(fileDataIDs.mapTexture);
    lodNormalTexture = m_api->cacheStorage->getTextureCache()->getFileId(fileDataIDs.mapTextureN);
}

int AdtObject::getAreaId(int mcnk_x, int mcnk_y) {
    auto index = m_adtFile->mcnkMap[mcnk_x][mcnk_y];
    if (index > -1) {
        return m_adtFile->mapTile[index].areaid;
    }

    return 0;
}
