//
// Created by deamon on 10.07.17.
//

#include <cstring>
#include <bitset>
#include "adtObject.h"
#include "../../shader/ShaderDefinitions.h"
#include "../../algorithms/mathHelper.h"
#include "../../persistance/adtFile.h"
#include "../../persistance/wdtFile.h"
#include "../../../gapi/interface/IDevice.h"
#include "../../../gapi/UniformBufferStructures.h"
#include "../../persistance/header/adtFileHeader.h"
#include "tbb/parallel_for.h"
#include "tbb/blocked_range2d.h"
#include "../../../gapi/interface/materials/IMaterial.h"


void AdtObject::loadingFinished(const HMapSceneBufferCreate &sceneRenderer) {
//    std::cout << "AdtObject::loadingFinished finished called";

    texturesPerMCNK = std::vector<AnimTextures>(m_adtFile->mcnkRead+1);
    animationTranslationPerMCNK = std::vector<AnimTrans>(m_adtFile->mcnkRead+1);

    loadAlphaTextures();

    createVBO(sceneRenderer);
    createMeshes(sceneRenderer);

    calcBoundingBoxes();

    loadM2s();
    loadWmos();

    m_loaded = true;

    loadWater(sceneRenderer);
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

void AdtObject::loadWater(const HMapSceneBufferCreate &sceneRenderer ) {
    if (m_adtFile->mH2OHeader == nullptr) return;

    m_waterPlacementChunk = sceneRenderer->createWMOWideChunk();
    m_waterPlacementChunk->getObject().uPlacementMat = mathfu::mat4::Identity();
    m_waterPlacementChunk->save();

    mathfu::vec3 adtBasePos = mathfu::vec3(AdtIndexToWorldCoordinate(adt_y), AdtIndexToWorldCoordinate(adt_x), 0);

    for (int y_chunk = 0; y_chunk < 16; y_chunk++) {
        for (int x_chunk = 0; x_chunk < 16; x_chunk++) {
            M2HOHeader::SMLiquidChunk &liquidChunk = m_adtFile->mH2OHeader->chunks[y_chunk*16 + x_chunk];
            if (liquidChunk.layer_count == 0) continue;

            auto liquidInstOffset = liquidChunk.offset_instances;

            auto *liquidInstPtr =
                ((SMLiquidInstance *)(&m_adtFile->mH2OBlob[liquidInstOffset]));

            mathfu::vec3 liquidBasePos =
                adtBasePos -
                mathfu::vec3(
                    MathHelper::CHUNKSIZE*y_chunk,
                    MathHelper::CHUNKSIZE*x_chunk,
                0);

            int i = this->m_adtFile->mcnkMap[x_chunk][y_chunk];
            auto &waterAaBB = waterTileAabb[i];
            waterAaBB.min = mathfu::vec3(99999,99999,99999);
            waterAaBB.max = mathfu::vec3(-99999,-99999,-99999);

            for (int layerInd = 0; layerInd < liquidChunk.layer_count; layerInd++) {
                SMLiquidInstance &liquidInstance = liquidInstPtr[layerInd];

                auto l_liquidInstance = std::make_shared<LiquidInstance>(
                    m_api, sceneRenderer, liquidInstance,
                    m_waterPlacementChunk, liquidBasePos, m_adtFile->mH2OBlob,
                    waterTileAabb[i]
                );

                m_liquidInstances.push_back(l_liquidInstance);
                m_liquidInstancesPerChunk[i].push_back(l_liquidInstance);
            }
        }
    }
}


void AdtObject::createVBO(const HMapSceneBufferCreate &sceneRenderer) {
    /* 1. help index + Heights + texCoords +  */

    std::vector<AdtVertex> vboArray ;

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
            AdtVertex &adtVertex = vboArray.emplace_back();

            /* 1.2 Heights */
            float iX = fmod(j, 17.0f);
            float iY = floor(j/17.0f);

            if (iX > 8.01f) {
                iY = iY + 0.5f;
                iX = iX - 8.5f;
            }
            adtVertex.pos = { m_adtFile->mapTile[i].position.x - iY * UNITSIZE,
                              m_adtFile->mapTile[i].position.y - iX * UNITSIZE,
                              m_adtFile->mapTile[i].position.z + m_adtFile->mcnkStructs[i].mcvt->height[j] };

            /* 1.3 Normals */
            if (m_adtFile->mcnkStructs[i].mcnr != nullptr) {
                for (int k = 0; k < 3; k++) {
                    adtVertex.normal.data_[k] = m_adtFile->mcnkStructs[i].mcnr->entries[j].normal[k] / 127.0f;
                }
            } else {
                adtVertex.normal = { 0, 0, 1.0 };
            }
            /* 1.4 MCCV */ //Color vertex
            if (m_adtFile->mcnkStructs[i].mccv != nullptr) {
                auto &mccv = m_adtFile->mcnkStructs[i].mccv;
                adtVertex.mccv = {
                    mccv->entries[j].red / 255.0f,
                    mccv->entries[j].green / 255.0f,
                    mccv->entries[j].blue / 255.0f,
                    mccv->entries[j].alpha / 255.0f,
                };
            } else {
                adtVertex.mccv = { 0.5f, 0.5f, 0.5f, 0.5f};
            }
            /* 1.4 MCLV */ //Lightning Vertex
            if (m_adtFile->mcnkStructs[i].mclv != nullptr) {
                auto &mclv = m_adtFile->mcnkStructs[i].mclv;
                adtVertex.mclv = {
                    mclv->values[j].b / 255.0f,
                    mclv->values[j].g / 255.0f,
                    mclv->values[j].r / 255.0f,
                    mclv->values[j].a / 255.0f
                };
            } else {
                //If MCLV is empty, localDiffuse doesn't exist in shader
                adtVertex.mclv = { 0.0f, 0.0f, 0.0f, 0.0f };
            }
        }
    }

    /* 1.3 Make combinedVbo */
    HGDevice device = m_api->hDevice;
    //TODO:
    combinedVbo = sceneRenderer->createADTVertexBuffer(vboArray.size()*sizeof(AdtVertex));
    combinedVbo->uploadData(vboArray.data(), vboArray.size()*sizeof(AdtVertex));

    /* 2. Strips */

    createIBOAndBinding(sceneRenderer);

    //Sometimes mvli can be zero, while there is still data in floatDataBlob
    if (m_adtFileLod!= nullptr && m_adtFileLod->getStatus()==FileStatus::FSLoaded && m_adtFileLod->floatDataBlob_len > 0 && m_adtFileLod->mvli_len > 0) {
        //Generate MLLL buffers
        //Index buffer for lod
        std::vector<float> vboLod;

        for (int i = 0; i < m_adtFileLod->floatDataBlob_len ; i++) {
            vboLod.push_back(this->m_adtFileLod->floatDataBlob[i]);
            vboLod.push_back((float) i);
        }

        //TODO:
        /*
        heightVboLod = device->createVertexBuffer();
        heightVboLod->uploadData(&vboLod[0], vboLod.size()*sizeof(float));

        /* 2. Index buffer */
//        stripVBOLod = device->createIndexBuffer();
//        stripVBOLod->uploadData(&m_adtFileLod->mvli_indicies[0],  m_adtFileLod->mvli_len * sizeof(int16_t));


        lodVertexBindings = device->createVertexBufferBindings();
        lodVertexBindings->setIndexBuffer(stripVBOLod);

        GVertexBufferBinding vertexBinding;
        vertexBinding.vertexBuffer = combinedVbo;

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
        if (mcnkContent->mcvt != nullptr) {
            for (int j = 0; j < (8 * 8 + 9 * 9); j++) {
                float heightVal = mcnkContent->mcvt->height[j];
                if (heightVal < minZ) minZ = heightVal;
                if (heightVal > maxZ) maxZ = heightVal;
            }
        }

        float minX = mcnkChunk->position.x - (MathHelper::CHUNKSIZE);
        float maxX = mcnkChunk->position.x;
        float minY = mcnkChunk->position.y - (MathHelper::CHUNKSIZE);
        float maxY = mcnkChunk->position.y;
        minZ += mcnkChunk->position.z;
        maxZ += mcnkChunk->position.z;

        this->tileAabb[i] = CAaBox(
            C3Vector(mathfu::vec3(minX, minY, minZ-10)),
            C3Vector(mathfu::vec3(maxX, maxY, maxZ+10))
        );

        this->globIndexY[i] = worldCoordinateToGlobalAdtChunk((minX + maxX) / 2.0f);
        this->globIndexX[i] = worldCoordinateToGlobalAdtChunk((minY + maxY) / 2.0f);
    }
}

void AdtObject::createMeshes(const HMapSceneBufferCreate &sceneRenderer) {
    HGDevice device = m_api->hDevice;

    auto adtFileTex = m_adtFileTex;
    auto adtFile = m_adtFile;

    int useHeightMixFormula = m_wdtFile->mphd->flags.adt_has_height_texturing > 0;
    auto api = m_api;

    PipelineTemplate pipelineTemplate;
    pipelineTemplate.element = DrawElementMode::TRIANGLES;
    pipelineTemplate.triCCW = true;
    pipelineTemplate.depthWrite = true;
    pipelineTemplate.depthCulling = true;
    pipelineTemplate.backFaceCulling = true;
    pipelineTemplate.blendMode = EGxBlendEnum::GxBlend_Opaque;

    auto const &stripOffsets = !m_api->getConfig()->ignoreADTHoles ?
                         m_adtFile->stripOffsets :
                         m_adtFile->stripOffsetsNoHoles;

    adtMeshes = {};
    adtMaterials = {};

    if (adtVertexBindings != nullptr) {
        for (int i = 0; i < 256; i++) {
            //Cant be used only in Wotlk
            //if (m_adtFile->mapTile[i].nLayers <= 0) continue;
            bool noLayers = m_adtFileTex->mcnkStructs[i].mcly == nullptr || m_adtFileTex->mcnkStructs[i].mclyCnt <= 0;

            ADTMaterialTemplate adtMaterialTemplate;
            fillTextureForMCNK(device, i, noLayers, adtMaterialTemplate);

            auto adtMaterial = sceneRenderer->createAdtMaterial(pipelineTemplate, adtMaterialTemplate);

            //Create mesh
            gMeshTemplate aTemplate(adtVertexBindings);
            aTemplate.meshType = MeshType::eAdtMesh;

            aTemplate.start = stripOffsets[i] * 2;
            aTemplate.end = stripOffsets[i + 1] - stripOffsets[i];

            HGMesh hgMesh = sceneRenderer->createMesh(aTemplate, adtMaterial);
            adtMeshes[i] = hgMesh;
            adtMaterials[i] = adtMaterial;

            //Upload data to static UBO
            auto &matVSPS = adtMaterial->m_materialVSPS->getObject();
            matVSPS.uPos = mathfu::vec4(
                this->m_adtFile->mapTile[i].position.x,
                this->m_adtFile->mapTile[i].position.y,
                this->m_adtFile->mapTile[i].position.z,
                0
            );

            matVSPS.useHeightMixFormula[0] = useHeightMixFormula;
            for (int j = 0; j < 4; j++) {
                matVSPS.uHeightOffset[j] = 0.0f;
                matVSPS.uHeightScale[j] = 1.0f;
            }
            for (int j = 0; j < adtFileTex->mcnkStructs[i].mclyCnt; j++) {
                if ((adtFileTex->mtxp_len > 0) && !noLayers) {
                    auto const &textureParams = adtFileTex->mtxp[adtFileTex->mcnkStructs[i].mcly[j].textureId];
                    matVSPS.uHeightOffset[j] = textureParams.heightOffset;
                    matVSPS.uHeightScale[j] = textureParams.heightScale;
                }
            }
            adtMaterial->m_materialVSPS->save();

            {
                auto &matPS = adtMaterial->m_materialPS->getObject();
                for (int j = 0; j < 4; j++) {
                    matPS.scaleFactorPerLayer[j] = 1;
                    matPS.animation_rotationPerLayer[j] = -1;
                    matPS.animation_speedPerLayer[j] = 0;
                }
                for (int j = 0; j < adtFileTex->mcnkStructs[i].mclyCnt; j++) {
                    if ((adtFileTex->mtxp_len > 0) && !noLayers) {
                        auto const &textureParams = adtFileTex->mtxp[adtFileTex->mcnkStructs[i].mcly[j].textureId];
                        float scaleFactor = (1.0f / (float)(1u << (textureParams.flags.texture_scale )));

                        matPS.scaleFactorPerLayer[j] = scaleFactor;
                    }
                    if (m_adtFileTex->mcnkStructs[i].mcly[j].flags.animation_enabled != 0) {
                        matPS.animation_rotationPerLayer[j] = m_adtFileTex->mcnkStructs[i].mcly[j].flags.animation_rotation;
                        matPS.animation_speedPerLayer[j] = m_adtFileTex->mcnkStructs[i].mcly[j].flags.animation_speed;
                    }
                }
                adtMaterial->m_materialPS->save();
            }
        }
    }
}

void AdtObject::fillTextureForMCNK(HGDevice &device, int i, bool noLayers, ADTMaterialTemplate &adtMaterialTemplate) {
    if (m_adtFileTex->mtxp_len > 0 && !noLayers) {
        for (int j = 0; j < m_adtFileTex->mcnkStructs[i].mclyCnt; j++) {
            auto const &textureParams = m_adtFileTex->mtxp[m_adtFileTex->mcnkStructs[i].mcly[j].textureId];

            auto layer_height = device->getWhiteTexturePixel();
            if (textureParams.flags.do_not_load_specular_or_height_texture_but_use_cubemap == 0) {
                if (!feq(textureParams.heightScale, 0.0)) {
                    layer_height = getAdtHeightTexture(m_adtFileTex->mcnkStructs[i].mcly[j].textureId);
                }
            }

            adtMaterialTemplate.textures[j + 5] = layer_height;
        }
    } else {
        for (int j = 0; j < 4; j++) {
            adtMaterialTemplate.textures[j + 5] = device->getWhiteTexturePixel();
        }
    }

    if (!noLayers) {
        adtMaterialTemplate.textures[4] = alphaTextures[i];
    } else {
        adtMaterialTemplate.textures[4] = device->getBlackTexturePixel();
    }

    if (!noLayers) {
        for (int j = 0; j < m_adtFileTex->mcnkStructs[i].mclyCnt; j++) {
            auto &layerDef = m_adtFileTex->mcnkStructs[i].mcly[j];

            HGSamplableTexture layer_x = getAdtTexture(m_adtFileTex->mcnkStructs[i].mcly[j].textureId);
//            BlpTexture &layer_spec = getAdtSpecularTexture(m_adtFileTex->mcnkStructs[i].mcly[j].textureId);
            adtMaterialTemplate.textures[j] = layer_x;
        }
    } else {
        for (int j = 0; j < 4; j++) {
            adtMaterialTemplate.textures[j] = device->getWhiteTexturePixel();
        }
    }
}

void AdtObject::loadAlphaTextures() {
    int chunkCount = m_adtFileTex->mcnkRead+1;
    int maxAlphaTexPerChunk = 4;
    int alphaTexSize = 64;

    int texWidth = alphaTexSize;
    int texHeight = alphaTexSize;

    int createdThisRun = 0;
    for (int i = 0; i < chunkCount; i++) {
        HGSamplableTexture alphaTexture = m_api->hDevice->createTexture(false, false);
        std::vector<uint8_t> alphaTextureData;
        m_adtFileTex->processTexture(m_wdtFile->mphd->flags, i, alphaTextureData);

        alphaTexture->getTexture()->loadData(texWidth, texHeight, &alphaTextureData[0], ITextureFormat::itRGBA);

        alphaTextures.push_back(alphaTexture);
    }
    this->alphaTexturesLoaded += createdThisRun;
}



void AdtObject::collectMeshes(ADTObjRenderRes &adtRes, std::vector<HGMesh> &opaqueMeshes, std::vector<HGSortableMesh> &transparentMeshes, int renderOrder) {
    if (m_freeStrategy != nullptr) m_freeStrategy(false, true, m_mapApi->getCurrentSceneTime());

    if (!m_loaded) return;

    adtRes.wasLoaded = true;

    size_t meshCount = adtMeshes.size();
    opaqueMeshes.reserve(opaqueMeshes.size() + adtMeshes.size());
    transparentMeshes.reserve(transparentMeshes.size() + m_liquidInstancesPerChunk.size());
    for (int i = 0; i < meshCount; i++) {
        if (adtRes.drawChunk[i] && (adtMeshes[i] != nullptr)) {
            opaqueMeshes.push_back(adtMeshes[i]);
        }
        if (adtRes.drawWaterChunk[i]) {
            for (auto const &liquidInstance : m_liquidInstancesPerChunk[i]) {
                liquidInstance->collectMeshes(transparentMeshes);
            }
        }
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


FileStatus AdtObject::getLoadedStatus() {
    const std::array<std::shared_ptr<PersistentFile>, 5> filesToCheck = {
        m_adtFile, m_adtFileObj, m_adtFileObjLod, m_adtFileLod, m_adtFileTex
    };

    for (auto &fileToCheck : filesToCheck) {
        if (fileToCheck == nullptr) continue;
        if (fileToCheck->getStatus()==FileStatus::FSRejected) {
            return FileStatus::FSRejected;
        }
        if (fileToCheck->getStatus()==FileStatus::FSNotLoaded) {
            return FileStatus::FSNotLoaded;
        }
    }
    return FileStatus::FSLoaded;
}


void AdtObject::doPostLoad(const HMapSceneBufferCreate &sceneRenderer) {
    if (!m_loaded) {
        if (getLoadedStatus() == FileStatus::FSLoaded) {
            this->loadingFinished(sceneRenderer);
            m_loaded = true;
        }
    }

    if (m_loaded && m_holesIgnored != m_api->getConfig()->ignoreADTHoles) {
        createIBOAndBinding(sceneRenderer);
        createMeshes(sceneRenderer);
    }
}
void AdtObject::update(animTime_t deltaTime ) {
    m_lastDeltaTime = deltaTime;
    m_lastTimeOfUpdate = m_mapApi->getCurrentSceneTime();

//    std::cout << "AdtObject::update finished called" << std::endl;
    if (!m_loaded) {
        return;
    }

    return;


    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < m_adtFileTex->mcnkStructs[i].mclyCnt; j++) {
            texturesPerMCNK[i].animTexture[j] = mathfu::mat4::Identity();
            if (m_adtFileTex->mtxp_len > 0) {
                auto const &textureParams = m_adtFileTex->mtxp[m_adtFileTex->mcnkStructs[i].mcly[j].textureId];

//                if (m_adtFileTex->mcnkStructs[i].mcly[j].flags.unknown_0x800 != 0 || m_adtFileTex->mcnkStructs[i].mcly[j].flags.unknown_0x1000 != 0) {
                    float scaleFactor = (1.0f / (float)(1u << (textureParams.flags.texture_scale )));
//                    float scaleFactor = ((float)(1u << (textureParams.flags.texture_scale )) / 4.0f);
                    texturesPerMCNK[i].animTexture[j]*=mathfu::mat4::FromScaleVector({scaleFactor, scaleFactor, scaleFactor});
//                }
            }

            if (m_adtFileTex->mcnkStructs[i].mcly[j].flags.animation_enabled != 0) {
                static const std::array<mathfu::vec2, 8> s_texDir =
                    {{
                         {-1.0, 0.0},
                         {-1.0, 1.0},
                         {0.0, 1.0},
                         {1.0, 1.0},
                         {1.0, 0.0},
                         {1.0, -1.0},
                         {0.0, -1.0},
                         {-1.0, -1.0}
                     }};
                static const std::array<float, 8> s_tempTexSpeed = {64.0, 48.0, 32.0, 16.0, 8.0, 4.0, 2.0, 1.0};

                auto &transVector = animationTranslationPerMCNK[i].transVectors[j];
                transVector += s_texDir[m_adtFileTex->mcnkStructs[i].mcly[j].flags.animation_rotation] * ((float) deltaTime * 0.001f);
                if (transVector.x >= 64.0) {
                    transVector.x = 0;
                }
                if (transVector.x <= -64.0) {
                    transVector.x = 0;
                }
                if (transVector.y >= 64.0) {
                    transVector.y = 0;
                }
                if (transVector.y <= -64.0) {
                    transVector.y = 0;
                }

    //                        std::cout << "this->m_lastDeltaTime = " << this->m_lastDeltaTime << std::endl;

                auto finalTransVector = (transVector ) *
                                   (1.0f / ((1.0f / -4.1666665f) *
                                            s_tempTexSpeed[m_adtFileTex->mcnkStructs[i].mcly[j].flags.animation_speed]));
                //                        std::cout << "transVector = " << transVector.x << " " << transVector.y << std::endl;

                texturesPerMCNK[i].animTexture[j] *= mathfu::mat4::FromTranslationVector({finalTransVector.y, finalTransVector.x, 0.0f});
            }
        }
    }

    for (int i = 0; i < 256; i++) {
        if (adtMaterials[i] != nullptr) {
            auto &psBlock = adtMaterials[i]->m_materialPS->getObject();

            for (int j = 0; j < 4; j++) {
                //psBlock.animationMat[j] = this->texturesPerMCNK[i].animTexture[j];
            }
            adtMaterials[i]->m_materialPS->save();
        }
    }
}

void AdtObject::uploadGeneratorBuffers(const HFrameDependantData &frameDependantData) {
    if (!m_loaded) return;

    if (m_waterPlacementChunk != nullptr) {
        m_waterPlacementChunk->getObject().uPlacementMat = mathfu::mat4::Identity();
        m_waterPlacementChunk->save();
    }

    for(auto &liquidInstance : m_liquidInstances) {
        liquidInstance->updateLiquidMaterials(frameDependantData, m_mapApi->getCurrentSceneTime());
    }
}

HGSamplableTexture AdtObject::getAdtTexture(int textureId) {
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

    HGSamplableTexture h_gblpTexture = nullptr;
    if (texture != nullptr) {
        h_gblpTexture = m_api->hDevice->createBlpTexture(texture, true, true);
    } else {
        //TODO: idk. DEBUG?
    }

    m_requestedTextures[textureId] = h_gblpTexture;

    return h_gblpTexture;
}

HGSamplableTexture AdtObject::getAdtHeightTexture(int textureId) {
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

    HGSamplableTexture h_gblpTexture = m_api->hDevice->createBlpTexture(texture, true, true);
    m_requestedTexturesHeight[textureId] = h_gblpTexture;

    return h_gblpTexture;
}

HGSamplableTexture AdtObject::getAdtSpecularTexture(int textureId) {
    auto item = m_requestedTexturesSpec.find(textureId);
    if (item != m_requestedTexturesSpec.end()) {
        return item->second;
    }

    std::string &materialTexture = m_adtFileTex->textureNames[textureId];

    std::string matHeightText = materialTexture.substr(0, materialTexture.size() - 4) + "_s.blp";

    HBlpTexture texture = m_api->cacheStorage->getTextureCache()->get(matHeightText);
    HGSamplableTexture h_gblpTexture = m_api->hDevice->createBlpTexture(texture, true, true);
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


bool AdtObject::iterateQuadTree(ADTObjRenderRes &adtFrustRes,
                                const mathfu::vec4 &camera,
                                const mathfu::vec3 &pos,
                                float x_offset, float y_offset, float cell_len,
                                int currentLod, int lastFoundLod,
                                const PointerChecker<MLND> &quadTree,
                                int quadTreeInd,
                                const MathHelper::FrustumCullingData &frustumData,
                                M2ObjectListContainer &m2ObjectsCandidates,
                                WMOListContainer &wmoCandidates) {



    bool drawLodBasedOnDist = false;
    const MLND *quadTreeNode = nullptr;
    int foundLod = lastFoundLod;

    if (quadTree == nullptr || quadTreeInd == -1 || currentLod == 4) {
        mathfu::vec2 aabb2D[2];
        aabb2D[0] = pos.xy() - mathfu::vec2(MathHelper::TILESIZE * (x_offset + cell_len), MathHelper::TILESIZE*(y_offset + cell_len));
        aabb2D[1] = pos.xy() -  mathfu::vec2(MathHelper::TILESIZE *x_offset, MathHelper::TILESIZE*y_offset);
        mathfu::vec2 point = camera.xy();

        //General frustum cull!
        bool atLeastOneIsDrawn = false;

        atLeastOneIsDrawn = checkNonLodChunkCulling(adtFrustRes, camera, frustumData,
                                                         16.0f * x_offset, 16.0f * y_offset,
                                                         16.0f * cell_len, 16.0f * cell_len
        );

        checkReferences(adtFrustRes, camera,
                        frustumData,
                         5,
                        m2ObjectsCandidates, wmoCandidates,
                        16.1f * x_offset, 16.1f * y_offset,
                        16.1f * cell_len, 16.1f * cell_len);

        return atLeastOneIsDrawn;
    }
    quadTreeNode = &quadTree[quadTreeInd];

    if (foundLod == currentLod) {
        quadTreeNode = &quadTree[quadTreeInd];

        mathfu::vec2 aabb2D[2];
        aabb2D[0] = pos.xy() - mathfu::vec2(533.3333f * (x_offset + cell_len), 533.3333f*(y_offset + cell_len));
        aabb2D[1] = pos.xy() -  mathfu::vec2(533.3333f *x_offset, 533.3333f*y_offset);

        mathfu::vec2 point = camera.xy();
        float dist = MathHelper::distanceFromAABBToPoint2DSquared(aabb2D, point);

        while (dist < perLodDist[foundLod] && foundLod < 5) foundLod++;
    }



    if (quadTreeNode != nullptr && foundLod > currentLod) {
        //check all others
        float newCellLen = cell_len/2.0f;
        bool atLeastOneIsDrawn = false;

        //1. Node 1
        atLeastOneIsDrawn |= iterateQuadTree(adtFrustRes, camera, pos, x_offset, y_offset , newCellLen, currentLod + 1, foundLod, quadTree, quadTreeNode->indices[0],
                                            frustumData, m2ObjectsCandidates, wmoCandidates);
        //2. Node 2
        atLeastOneIsDrawn |= iterateQuadTree(adtFrustRes,camera, pos, x_offset, y_offset + newCellLen, newCellLen, currentLod + 1, foundLod, quadTree, quadTreeNode->indices[1],
                                            frustumData, m2ObjectsCandidates, wmoCandidates);
        //3. Node 3
        atLeastOneIsDrawn |= iterateQuadTree(adtFrustRes,camera, pos, x_offset + newCellLen, y_offset, newCellLen, currentLod + 1, foundLod, quadTree, quadTreeNode->indices[2],
                                            frustumData,  m2ObjectsCandidates, wmoCandidates);
        //4. Node 4
        atLeastOneIsDrawn |= iterateQuadTree(adtFrustRes, camera, pos, x_offset+ newCellLen, y_offset+ newCellLen, newCellLen, currentLod + 1, foundLod, quadTree, quadTreeNode->indices[3],
                                            frustumData,  m2ObjectsCandidates, wmoCandidates);

        return atLeastOneIsDrawn;
    } else {
            //Push the drawCall for this lod
            LodCommand command;
            command.index = quadTreeNode->index;
            command.length = quadTreeNode->length;

            lodCommands.push_back(command);

            checkReferences(adtFrustRes, camera,
                            frustumData,
                            currentLod,
                            m2ObjectsCandidates, wmoCandidates,
                            16.0f * x_offset, 16.0f * y_offset,
                            16.0f * cell_len, 16.0f * cell_len);

            bool atLeastOneIsDrawn = true;

        return atLeastOneIsDrawn;
    }
}

bool AdtObject::checkNonLodChunkCulling(ADTObjRenderRes &adtFrustRes,
                                        const mathfu::vec4 &cameraPos,
                                        const MathHelper::FrustumCullingData &frustumData,

                                        int x, int y, int x_len, int y_len) {

    bool atLeastOneIsDrawn = false;
//    tbb::parallel_for(tbb::blocked_range2d<int,int>(x,x+x_len,y,y+y_len), [&](const tbb::blocked_range2d<int,int>& r) {
//        for (size_t k = r.rows().begin(); k != r.rows().end(); ++k) {
//            for (size_t l = r.cols().begin(); l != r.cols().end(); ++l) {
    {
        for (size_t k = x; k < x+x_len; k++) {
            for (size_t l = y; l < y+y_len; ++l) {
                int i = this->m_adtFile->mcnkMap[l][k];
                if (i < 0)
                    continue;

                mcnkStruct_t &mcnk = this->m_adtFile->mcnkStructs[i];

                adtFrustRes.drawChunk[i] = false;
                adtFrustRes.drawWaterChunk[i] = false;

                {
                    CAaBox &aabb = this->tileAabb[i];
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
                        result = MathHelper::checkFrustum(frustumData, aabb);
                        bool frustum2DRes = MathHelper::checkFrustum2D(frustumData.frustums, aabb);
                        adtFrustRes.checkRefs[i] = result || frustum2DRes;

                        adtFrustRes.drawChunk[i] = result;
                        atLeastOneIsDrawn = atLeastOneIsDrawn || result;
                    }
                }

                //Do the same for Water tile
                {
                    CAaBox &aabb = this->waterTileAabb[i];
                    bool cameraOnChunk =
                        (cameraPos[0] > aabb.min.x && cameraPos[0] < aabb.max.x &&
                         cameraPos[1] > aabb.min.y && cameraPos[1] < aabb.max.y);
                    if (cameraOnChunk &&
                        cameraPos[2] > aabb.min.z && cameraPos[2] < aabb.max.z
                        ) {
                        adtFrustRes.drawWaterChunk[i] = true;
                        atLeastOneIsDrawn = true;
                    }

                    //2. Check aabb is inside camera frustum
                    bool result = false;
                    if (!adtFrustRes.drawWaterChunk[i]) {
                        result = MathHelper::checkFrustum(frustumData, aabb);
                        adtFrustRes.drawWaterChunk[i] = result;
                        atLeastOneIsDrawn = atLeastOneIsDrawn || result;
                    }
                }
            }
        }
    }//,tbb::auto_partitioner());

    return atLeastOneIsDrawn;
}

bool AdtObject::checkReferences(
    ADTObjRenderRes &adtFrustRes,
    const mathfu::vec4 &cameraPos,
    const MathHelper::FrustumCullingData &frustumData,
    int lodLevel,
    M2ObjectListContainer &m2ObjectsCandidates,
    WMOListContainer &wmoCandidates,
    int x, int y, int x_len, int y_len) {
    if (!m_loaded) return false;

    if (m_freeStrategy != nullptr)
        m_freeStrategy(false, true, m_mapApi->getCurrentSceneTime());

//    tbb::parallel_for(tbb::blocked_range2d<int,int>(x,x+x_len,y,y+y_len), [&](const tbb::blocked_range2d<int,int>& r) {
//        for (size_t k = r.rows().begin(); k != r.rows().end(); ++k) {
//            for (size_t l = r.cols().begin(); l != r.cols().end(); ++l) {
//    {
        for (size_t k = x; k < x+x_len; k++) {
            for (size_t l = y; l < y+y_len; ++l) {
                int i = this->m_adtFile->mcnkMap[k][l];

                if (i < 0) continue;

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
                                m2ObjectsCandidates.addCandidate(this->objectLods[0].m2Objects[m2Ref]);
                            }
                        }
                    } else {
                        for (auto &m2Object: this->objectLods[1].m2Objects) {
                            m2ObjectsCandidates.addCandidate(m2Object);
                        }
                    }

                    if (lodLevel >= 4) {
                        if (mcnkContent->mcrw_object_refs_len > 0) {
                            for (int j = 0; j < mcnkContent->mcrw_object_refs_len; j++) {
                                uint32_t wmoRef = mcnkContent->mcrw_object_refs[j];
                                wmoCandidates.addCand(this->objectLods[0].wmoObjects[wmoRef]);
                            }
                        }
                    } else {
                        for (auto &wmoObject: this->objectLods[1].wmoObjects) {
                            wmoCandidates.addCand(wmoObject);
                        }
                    }
                }
            }
        }
//    },tbb::auto_partitioner());

	return true;
}

bool AdtObject::checkFrustumCulling(ADTObjRenderRes &adtFrustRes,
                                    const mathfu::vec4 &cameraPos,
                                    const MathHelper::FrustumCullingData &frustumData,
                                    M2ObjectListContainer &m2ObjectsCandidates,
                                    WMOListContainer &wmoCandidates) {
    if (!this->m_loaded) {
        if (m_freeStrategy != nullptr)
            m_freeStrategy(false, true, m_mapApi->getCurrentSceneTime());
        return true;
    }
    bool atLeastOneIsDrawn = false;

    mostDetailedLod = 5;
    leastDetiledLod = 0;

    for (int i = 0; i < 256; i++) {
        adtFrustRes.drawChunk[i] = false;
        adtFrustRes.drawWaterChunk[i] = false;
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
                                             frustumData,
                                             m2ObjectsCandidates,
                                             wmoCandidates);
    } else {
        //For old ADT without _lod.adt
        atLeastOneIsDrawn |= checkNonLodChunkCulling(adtFrustRes, cameraPos,
                                frustumData,
                                0, 0, 16, 16);
        checkReferences(adtFrustRes, cameraPos,
                        frustumData,
                         5,
                        m2ObjectsCandidates, wmoCandidates,
                        0, 0,
                        16, 16);
    }

//    if (atLeastOneIsDrawn) {
        if (m_freeStrategy != nullptr)
            m_freeStrategy(false, true, m_mapApi->getCurrentSceneTime());
//    }

    return atLeastOneIsDrawn;
}

AdtObject::AdtObject(HApiContainer api, std::string &adtFileTemplate, std::string mapname, int adt_x, int adt_y, HWdtFile wdtFile) : alphaTextures(), adt_x(adt_x), adt_y(adt_y){
    m_api = api;
    tileAabb = std::vector<CAaBox>(256);
    waterTileAabb = std::vector<CAaBox>(256);
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

AdtObject::AdtObject(HApiContainer api, int adt_x, int adt_y, WdtFile::MapFileDataIDs &fileDataIDs, HWdtFile wdtFile): adt_x(adt_x), adt_y(adt_y) {
    m_api = api;
    tileAabb = std::vector<CAaBox>(256);
    waterTileAabb = std::vector<CAaBox>(256);
    globIndexX = std::vector<int>(256);
    globIndexY = std::vector<int>(256);

    m_wdtFile = wdtFile;

    m_adtFile = m_api->cacheStorage->getAdtGeomCache()->getFileId(fileDataIDs.rootADT);
    m_adtFile->setIsMain(true);
    m_adtFileTex = m_api->cacheStorage->getAdtGeomCache()->getFileId(fileDataIDs.tex0ADT);
    m_adtFileObj = m_api->cacheStorage->getAdtGeomCache()->getFileId(fileDataIDs.obj0ADT);
    m_adtFileObjLod = m_api->cacheStorage->getAdtGeomCache()->getFileId(fileDataIDs.obj1ADT);
    if (fileDataIDs.lodADT != 0 && m_wdtFile->mphd->flags.unk_0x0100) {
        m_adtFileLod = m_api->cacheStorage->getAdtGeomCache()->getFileId(fileDataIDs.lodADT);
    } else {
        m_adtFileLod = nullptr;
    }

    lodDiffuseTexture = m_api->cacheStorage->getTextureCache()->getFileId(fileDataIDs.mapTexture);
    lodNormalTexture = m_api->cacheStorage->getTextureCache()->getFileId(fileDataIDs.mapTextureN);
}

bool AdtObject::getWaterColorFromDB(mathfu::vec4 cameraPos, mathfu::vec3 &closeRiverColor) {
    if (!m_loaded) return false;

    auto adt_x = worldCoordinateToAdtIndex(cameraPos.y);
    auto adt_y = worldCoordinateToAdtIndex(cameraPos.x);

    if (adt_x != getAdtX() || adt_y != getAdtY())
        return false;

    int x_chunk = worldCoordinateToGlobalAdtChunk(cameraPos.y) % 16;
    int y_chunk = worldCoordinateToGlobalAdtChunk(cameraPos.x) % 16;

    int i = this->m_adtFile->mcnkMap[x_chunk][y_chunk];
    auto &waterAaBB = waterTileAabb[i];

    if (
        waterAaBB.min.x > 32*MathHelper::TILESIZE || waterAaBB.max.x < -32*MathHelper::TILESIZE ||
        waterAaBB.min.y > 32*MathHelper::TILESIZE || waterAaBB.max.x < -32*MathHelper::TILESIZE ||
        waterAaBB.min.z > 32*MathHelper::TILESIZE || waterAaBB.max.x < -32*MathHelper::TILESIZE
    ) return false;

    mathfu::vec3 waterPos = (mathfu::vec3(waterAaBB.max) + mathfu::vec3(waterAaBB.min)) / 2.0f;
    std::vector<LightResult> lightResults = {};
    closeRiverColor = {0,0,0};
    this->m_mapApi->getLightResultsFromDB(waterPos, m_api->getConfig(), lightResults, nullptr);
    for (auto &_light : lightResults) {
        closeRiverColor += mathfu::vec3(_light.closeRiverColor) * _light.blendCoef;
    }
    closeRiverColor = mathfu::vec3(closeRiverColor[2], closeRiverColor[1], closeRiverColor[0]);

    return true;
}

CAaBox AdtObject::calcAABB() {
    mathfu::vec3 minCoord = mathfu::vec3(20000, 20000, 20000);
    mathfu::vec3 maxCoord = mathfu::vec3(-20000, -20000, -20000);

    for (auto &aaBox : tileAabb) {
        minCoord = mathfu::vec3(
            std::min<float>(minCoord.x, aaBox.min.x),
            std::min<float>(minCoord.y, aaBox.min.y),
            std::min<float>(minCoord.z, aaBox.min.z)
        );
        maxCoord = mathfu::vec3(
            std::max<float>(maxCoord.x, aaBox.max.x),
            std::max<float>(maxCoord.y, aaBox.max.y),
            std::max<float>(maxCoord.z, aaBox.max.z)
        );
    }
    for (auto &aaBox : waterTileAabb) {
        minCoord = mathfu::vec3(
            std::min<float>(minCoord.x, aaBox.min.x),
            std::min<float>(minCoord.y, aaBox.min.y),
            std::min<float>(minCoord.z, aaBox.min.z)
        );
        maxCoord = mathfu::vec3(
            std::max<float>(maxCoord.x, aaBox.max.x),
            std::max<float>(maxCoord.y, aaBox.max.y),
            std::max<float>(maxCoord.z, aaBox.max.z)
        );
    }

    return CAaBox(mathfu::vec3_packed(minCoord), mathfu::vec3_packed(maxCoord));
}

int AdtObject::getAreaId(int mcnk_x, int mcnk_y) {
    if (!m_loaded) {
        return 0;
    }
    auto index = m_adtFile->mcnkMap[mcnk_x][mcnk_y];
    if (index > -1) {
        return m_adtFile->mapTile[index].areaid;
    }

    return 0;
}

void AdtObject::createIBOAndBinding(const HMapSceneBufferCreate &sceneRenderer) {
    auto const &strips = !m_api->getConfig()->ignoreADTHoles ?
         m_adtFile->strips :
         m_adtFile->stripsNoHoles;


    if (strips.size() > 0) {
        stripIBO = sceneRenderer->createADTIndexBuffer(strips.size() * sizeof(int16_t));
        stripIBO->uploadData(strips.data(), strips.size() * sizeof(int16_t));

        adtVertexBindings = sceneRenderer->createADTVAO(combinedVbo, stripIBO);
    } else {
        stripIBO = nullptr;
        adtVertexBindings = nullptr;
    }

    m_holesIgnored = m_api->getConfig()->ignoreADTHoles;
}
