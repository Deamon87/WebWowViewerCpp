//
// Created by Deamon on 5/20/2023.
//

#include "LiquidInstance.h"
#include "LiquidDataGetters.h"
#include "../../algorithms/mathHelper.h"
#include "../iMapApi.h"


LiquidInstance::LiquidInstance(const HApiContainer &api,
                               const HMapSceneBufferCreate &sceneRenderer,
                               const SMLiquidInstance &liquidInstance,
                               const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &waterPlacementChunk,
                               const mathfu::vec3 &liquidBasePos,
                               const PointerChecker<char> &mH2OBlob, CAaBox &waterAaBB) :
    m_api(api), m_waterPlacementChunk(waterPlacementChunk),
    m_waterBBox(waterAaBB){

    //Creating liquid instance from ADT data
    liquid_object = liquidInstance.liquid_object_or_lvf >= 42 ? liquidInstance.liquid_object_or_lvf : 0;
    liquidType = liquidInstance.liquid_type;

    //1. Get Data from DB
    bool generateTexCoordsFromPos;

    getInfoFromDatabase(liquidInstance.liquid_object_or_lvf, liquidInstance.liquid_type, generateTexCoordsFromPos);

    //2. Parse Vertex Data from ADT
    std::vector<LiquidVertexFormat> vertexBuffer;
    std::vector<uint16_t> indexBuffer;

    createAdtVertexData(liquidInstance, liquidBasePos, mH2OBlob, m_waterBBox, m_liqMatAndType.matLVF,
                        generateTexCoordsFromPos, vertexBuffer,
                        indexBuffer);

    HGVertexBufferBindings vertexWaterBufferBindings = createLiquidVao(sceneRenderer, vertexBuffer, indexBuffer);

    createMaterialAndMesh(sceneRenderer, indexBuffer.size(), vertexWaterBufferBindings);
}

LiquidInstance::LiquidInstance(const HApiContainer &api,
                               const HMapSceneBufferCreate &sceneRenderer,
                               const HGVertexBufferBindings &binding,
                               int liquidType,
                               int indexBufferSize,
                               const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &waterPlacementChunk,
                               CAaBox &waterAaBB): m_api(api),
                                                   m_waterPlacementChunk(waterPlacementChunk), m_waterBBox(waterAaBB){
    bool generateTexCoordsFromPos;
    getInfoFromDatabase(0,
                        liquidType,
                        generateTexCoordsFromPos);

    createMaterialAndMesh(sceneRenderer, indexBufferSize, binding);
}

void LiquidInstance::createMaterialAndMesh(const HMapSceneBufferCreate &sceneRenderer,
                                           int indexBufferSize,
                                           const HGVertexBufferBindings &vertexWaterBufferBindings) {
    //Create material(s)
    int basetextureFDID = m_liquidTextureData.size() > 0 ? m_liquidTextureData[0].fileDataId : 0;

    mathfu::vec3 color = mathfu::vec3(0,0,0);
    if (m_liqMatAndType.color1[0] > 0 || m_liqMatAndType.color1[1] > 0 || m_liqMatAndType.color1[2] > 0) {
        color = mathfu::vec3(m_liqMatAndType.color1[2], m_liqMatAndType.color1[1], m_liqMatAndType.color1[0]);
    }
//    minimapStaticCol = mathfu::vec3(liqMatAndType.minimapStaticCol[2], liqMatAndType.minimapStaticCol[1], liqMatAndType.minimapStaticCol[0]);

    int liquidFlags = m_liqMatAndType.flags;
    int liquidMaterialId = m_liqMatAndType.materialID;

    PipelineTemplate pipelineTemplate;
    pipelineTemplate.element = DrawElementMode::TRIANGLES;
    pipelineTemplate.depthWrite = true;
    pipelineTemplate.depthCulling = true;
    pipelineTemplate.backFaceCulling = false;
    pipelineTemplate.blendMode = EGxBlendEnum::GxBlend_Alpha;

    WaterMaterialTemplate waterMaterialTemplate;
    waterMaterialTemplate.color = color;
    waterMaterialTemplate.liquidFlags = liquidFlags;
    waterMaterialTemplate.liquidMaterialId = liquidMaterialId;



    assert(liquidFlags >= 0 && liquidFlags <= 2063);

    if (basetextureFDID != 0) {
        auto htext = m_api->cacheStorage->getTextureCache()->getFileId(basetextureFDID);
        waterMaterialTemplate.texture = m_api->hDevice->createBlpTexture(htext, true, true);
    } else {
        waterMaterialTemplate.texture = m_api->hDevice->getBlackTexturePixel();
    }
    auto waterMaterial = sceneRenderer->createWaterMaterial(m_waterPlacementChunk, pipelineTemplate, waterMaterialTemplate);

    m_liquidMaterials.push_back(waterMaterial);
    m_vertexWaterBufferBindings.push_back(vertexWaterBufferBindings);

    {
        auto &waterChunk = waterMaterial->m_materialPS->getObject();
        waterChunk.materialId = waterMaterial->materialId;
        waterChunk.liquidFlags = waterMaterial->liquidFlags;
        waterChunk.float0_float1.x = m_liqMatAndType.m_floats[0];
        waterChunk.float0_float1.y = m_liqMatAndType.m_floats[1];
        waterChunk.matColor = mathfu::vec4(waterMaterial->color, 0.7f);
        waterMaterial->m_materialPS->save();
    }


    //Create mesh(es)
    for (int i = 0; i < m_liquidMaterials.size(); i++) {
        gMeshTemplate meshTemplate(vertexWaterBufferBindings);
        meshTemplate.meshType = MeshType::eWmoMesh;

        meshTemplate.start = 0;
        meshTemplate.end = indexBufferSize;

        auto mesh = sceneRenderer->createWaterMesh(meshTemplate, waterMaterial, 99);
        m_liquidMeshes.push_back(mesh);
    }

    assert(waterMaterial->liquidFlags >= 0 && waterMaterial->liquidFlags <= 2063);
}

void
LiquidInstance::getInfoFromDatabase(int liquid_object_or_lvf, int liquid_type, bool &generateTexCoordsFromPos) {
    m_liqMatAndType.color1 = {0, 0, 0};
    m_liqMatAndType.flags = 0;
    m_liqMatAndType.matLVF = 0;

    generateTexCoordsFromPos= false;
    mathfu::vec3 minimapStaticCol = {0, 0, 0};
    if (m_api->databaseHandler != nullptr) {
        if (liquid_object_or_lvf > 41) {

            m_api->databaseHandler->getLiquidObjectData(liquid_object_or_lvf, liquid_type, m_liqMatAndType, m_liquidTextureData);
            //From Liquid::GetVertexFormat
            if (liquid_type == 2) {
                m_liqMatAndType.matLVF = 2;
            }
        } else {

            m_api->databaseHandler->getLiquidTypeData(liquid_type, m_liqMatAndType, m_liquidTextureData);

            m_liqMatAndType.matLVF = liquid_object_or_lvf;
        }

        generateTexCoordsFromPos = getLiquidSettings(liquid_object_or_lvf,
                                                     liquid_type,
                                                     m_liqMatAndType.materialID,
                                                     m_liqMatAndType.flowSpeed <=0).generateTexCoordsFromPos;
    }

    assert(m_liqMatAndType.flags >= 0 && m_liqMatAndType.flags <= 2063);
}

HGVertexBufferBindings LiquidInstance::createLiquidVao(const HMapSceneBufferCreate &sceneRenderer,
                                                       std::vector<LiquidVertexFormat> &vertexBuffer,
                                                       std::vector<uint16_t> &indexBuffer) const {
    auto waterIBO = sceneRenderer->createWaterIndexBuffer(indexBuffer.size() * sizeof(uint16_t));
    waterIBO->uploadData(indexBuffer.data(), indexBuffer.size() * sizeof(uint16_t));

    auto waterVBO = sceneRenderer->createWaterVertexBuffer(vertexBuffer.size() * sizeof(LiquidVertexFormat));
    waterVBO->uploadData(
        vertexBuffer.data(),
        vertexBuffer.size() * sizeof(LiquidVertexFormat)
    );


    auto vertexWaterBufferBindings = sceneRenderer->createWaterVAO(waterVBO, waterIBO);
    return vertexWaterBufferBindings;
}

void LiquidInstance::createAdtVertexData(const SMLiquidInstance &liquidInstance, const mathfu::vec3 &liquidBasePos,
                                         const PointerChecker<char> &mH2OBlob, CAaBox &waterAaBB,
                                         int liquidVertexFormat, bool generateTexCoordsFromPos,
                                         std::vector<LiquidVertexFormat> &vertexBuffer,
                                         std::vector<uint16_t> &indexBuffer) const {
    if (!liquidInstance.offset_vertex_data && liquidInstance.liquid_type != 2) {
        liquidVertexFormat = 2;
    }

    float *vertexDataPtr = nullptr;
    if (liquidInstance.offset_vertex_data != 0) {
        vertexDataPtr = ((float *) (&mH2OBlob[liquidInstance.offset_vertex_data]));
    }

    //Set iteration restrictions for triangles
    int y_begin = 0;
    int x_begin = 0;
    int x_end = 8;
    int y_end = 8;
    int totalCount = 9 * 9;
    if (liquidInstance.liquid_object_or_lvf <= 41) {
        x_begin = liquidInstance.x_offset;
        y_begin = liquidInstance.y_offset;
        x_end = liquidInstance.x_offset + liquidInstance.width;
        y_end = liquidInstance.y_offset + liquidInstance.height;

        totalCount = (liquidInstance.width + 1) * (liquidInstance.height + 1) ;
    }

    float minX = 999999;
    float maxX = -999999;
    float minY = 999999;
    float maxY = -999999;
    float minZ = 999999;
    float maxZ = -999999;

    minX = std::min(minX, waterAaBB.min.x);
    maxX = std::max(maxX, waterAaBB.max.x);
    minY = std::min(minY, waterAaBB.min.y);
    maxY = std::max(maxY, waterAaBB.max.y);
    minZ = std::min(minZ, waterAaBB.min.z);
    maxZ = std::max(maxZ, waterAaBB.max.z);

    //Parse the blob
    for (int y = 0; y < liquidInstance.height + 1; y++) {
        for (int x = 0; x < liquidInstance.width + 1; x++) {
            mathfu::vec3 pos =
                liquidBasePos -
                mathfu::vec3(
                    MathHelper::UNITSIZE*(y+liquidInstance.y_offset),
                    MathHelper::UNITSIZE*(x+liquidInstance.x_offset),
                    -liquidInstance.min_height_level
                );

            mathfu::vec2 uv = mathfu::vec2(0,0);
            if (vertexDataPtr!= nullptr) {
                pos.z = getLiquidVertexHeight(liquidVertexFormat, vertexDataPtr, totalCount, y * (liquidInstance.width + 1) + x);
            }

            if (generateTexCoordsFromPos) {
                uv = mathfu::vec2(pos.x * 0.06f, pos.y * 0.06f);
            } else {
                uv = getLiquidVertexCoords(liquidVertexFormat, vertexDataPtr, totalCount, y * (liquidInstance.width + 1) + x);
            }

            minX = std::min(minX, pos.x);  maxX = std::max(maxX, pos.x);
            minY = std::min(minY, pos.y);  maxY = std::max(maxY, pos.y);
            minZ = std::min(minZ, pos.z);  maxZ = std::max(maxZ, pos.z);

            LiquidVertexFormat vertex;
            vertex.pos_transp = mathfu::vec4(pos, 1.0);
            vertex.uv = uv;

            vertexBuffer.push_back(vertex);
        }
    }
    waterAaBB = CAaBox(
        C3Vector(mathfu::vec3(minX, minY, minZ)),
        C3Vector(mathfu::vec3(maxX, maxY, maxZ))
    );

    uint8_t *existsTable = getLiquidExistsTable(mH2OBlob, liquidInstance);

    for (int y = y_begin; y < y_end; y++) {
        for (int x = x_begin; x < x_end; x++) {

            auto x_local = x - x_begin;
            auto y_local = y - y_begin;

            int maskIndex = (y_local) * (x_end - x_begin) + (x_local);
            bool exists = (existsTable[maskIndex >> 3] >> ((maskIndex & 7))) & 1;

            if (!exists) continue;

            const int16_t vertIndexes[4] = {
                (int16_t) (y_local * (liquidInstance.width + 1 ) + x_local),
                (int16_t) (y_local * (liquidInstance.width + 1) + x_local + 1),
                (int16_t) ((y_local + 1) * (liquidInstance.width + 1) + x_local),
                (int16_t) ((y_local + 1) * (liquidInstance.width + 1) + x_local + 1),
            };

            indexBuffer.push_back (vertIndexes[0]);
            indexBuffer.push_back (vertIndexes[1]);
            indexBuffer.push_back (vertIndexes[2]);

            indexBuffer.push_back (vertIndexes[1]);
            indexBuffer.push_back (vertIndexes[3]);
            indexBuffer.push_back (vertIndexes[2]);
        }
    }
}

void LiquidInstance::updateLiquidMaterials(const HFrameDependantData &frameDependantData, animTime_t mapCurrentTime) {
}

void LiquidInstance::collectMeshes(COpaqueMeshCollector &opaqueMeshCollector) {
    //TODO: Get time and right mesh instance for animation
    if (m_api->getConfig()->renderLiquid) {
        opaqueMeshCollector.addWaterMesh(m_liquidMeshes[0]);
    }
}

void LiquidInstance::collectMeshes(framebased::vector<HGSortableMesh> &transparentMeshes) {
    //TODO: Get time and right mesh instance for animation
    if (m_api->getConfig()->renderLiquid) {
        transparentMeshes.emplace_back() = m_liquidMeshes[0];
    }
}

mathfu::mat4 LiquidInstance::GetTexScrollMtx(int time, mathfu::vec2 scrollVec) {
    float scrollX = 0.0f;
    float scrollY = 0.0f;

    if (!feq(scrollVec.x, 0.0)) {
        scrollX = (time % (int)(1000.0f / scrollVec.x)) / (float)(int)(1000.0f / scrollVec.x);
    }
    if (!feq(scrollVec.y, 0.0)) {
        scrollY = (time % (int)(1000.0f / scrollVec.y)) / (float) (int)(1000.0f / scrollVec.y);
    }

    return mathfu::mat4::FromTranslationVector(mathfu::vec3(scrollX, scrollY, 0.0f));
}

std::shared_ptr<LiquidInstanceEntityFactory> liquidInstanceFactory = std::make_shared<LiquidInstanceEntityFactory>();