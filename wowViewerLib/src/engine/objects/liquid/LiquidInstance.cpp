//
// Created by Deamon on 5/20/2023.
//

#include "LiquidInstance.h"
#include "LiquidDataGetters.h"
#include "../../algorithms/mathHelper.h"
#include "../iMapApi.h"


LiquidInstance::LiquidInstance(const HApiContainer &api,
                               const HMapSceneBufferCreate &sceneRenderer,
                               const std::shared_ptr<ILiquidMaterial> &liquidMaterial,
                               const SMLiquidInstance &liquidInstance,
                               const mathfu::vec3 &liquidBasePos,
                               const PointerChecker<char> &mH2OBlob, CAaBox &waterAaBB) :
    m_api(api), m_waterBBox(waterAaBB), m_liquidMaterial(liquidMaterial) {

    //Creating liquid instance from ADT data

    //Parse Vertex Data from ADT
    std::vector<LiquidVertexFormat> vertexBuffer;
    std::vector<uint16_t> indexBuffer;

    createAdtVertexData(liquidInstance, liquidBasePos, mH2OBlob, m_waterBBox, liquidMaterial->matLVF,
                        liquidMaterial->generateTexCoordsFromPos, vertexBuffer,
                        indexBuffer);

    HGVertexBufferBindings vertexWaterBufferBindings = createLiquidVao(sceneRenderer, vertexBuffer, indexBuffer);

    createMesh(sceneRenderer, indexBuffer.size(), vertexWaterBufferBindings);
}

LiquidInstance::LiquidInstance(const HApiContainer &api,
                               const HMapSceneBufferCreate &sceneRenderer,
                               const HGVertexBufferBindings &binding,
                               const std::shared_ptr<ILiquidMaterial> &liquidMaterial,
                               int liquidType,
                               int indexBufferSize,
                               CAaBox &waterAaBB): m_api(api), m_waterBBox(waterAaBB), m_liquidMaterial(liquidMaterial) {

    createMesh(sceneRenderer, indexBufferSize, binding);
}

void LiquidInstance::createMesh(const HMapSceneBufferCreate &sceneRenderer,
                                int indexBufferSize,
                                const HGVertexBufferBindings &vertexWaterBufferBindings) {
    m_vertexWaterBufferBindings.push_back(vertexWaterBufferBindings);

    //Create mesh(es)
    {
        gMeshTemplate meshTemplate(vertexWaterBufferBindings);
#ifdef DEBUG_MESH_NAMES
        meshTemplate.name = "Liquid, Mat = " + std::to_string(m_liquidMaterial->materialId);
#endif
        meshTemplate.meshType = MeshType::eWmoMesh;

        meshTemplate.start = 0;
        meshTemplate.end = indexBufferSize;

        auto mesh = sceneRenderer->createWaterMesh(meshTemplate, m_liquidMaterial, 99);
        m_liquidMeshes.push_back(mesh);
//        m_liquidMeshes.push_back(nullptr);
    }
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

template<int liquidVertexFormat, bool generateTexCoordsFromPos>
inline void parseMH2OVertexData(const SMLiquidInstance &liquidInstance, const mathfu::vec3 &liquidBasePos,
                                uint8_t defaultDepth,
                                std::vector<LiquidVertexFormat> &vertexBuffer, float *vertexDataPtr,
                                int totalCount, CAaBox &waterAaBB) {
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

    vertexBuffer.reserve( (liquidInstance.height + 1) * (liquidInstance.width + 1));

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

            pos.z = getLiquidVertexHeight(liquidVertexFormat, vertexDataPtr, totalCount, y * (liquidInstance.width + 1) + x);

            if constexpr (generateTexCoordsFromPos) {
                uv = mathfu::vec2(pos.x * 0.06f, pos.y * 0.06f);
            } else {
                uv = getLiquidVertexCoords(liquidVertexFormat, vertexDataPtr, totalCount, y * (liquidInstance.width + 1) + x);
            }

            uint8_t depth = getLiquidDepth(defaultDepth, liquidVertexFormat, vertexDataPtr, totalCount,  y * (liquidInstance.width + 1) + x);

            minX = std::min(minX, pos.x);  maxX = std::max(maxX, pos.x);
            minY = std::min(minY, pos.y);  maxY = std::max(maxY, pos.y);
            minZ = std::min(minZ, pos.z);  maxZ = std::max(maxZ, pos.z);

            LiquidVertexFormat &vertex = vertexBuffer.emplace_back();

            vertex.pos_transp = mathfu::vec4(pos, depth/255.0f);
            vertex.uv = uv;
        }
    }

    waterAaBB = CAaBox(
        C3Vector(mathfu::vec3(minX, minY, minZ)),
        C3Vector(mathfu::vec3(maxX, maxY, maxZ))
    );
}

template<bool generateTexCoordsFromPos>
inline void vertexParseSelection(int liquidVertexFormat, const SMLiquidInstance &liquidInstance, const mathfu::vec3 &liquidBasePos,
                                uint8_t defaultDepth,
                                std::vector<LiquidVertexFormat> &vertexBuffer, float *vertexDataPtr,
                                int totalCount, CAaBox &waterAaBB)
{
    switch (liquidVertexFormat) {
        case -1:
            parseMH2OVertexData<-1, generateTexCoordsFromPos>
                (liquidInstance, liquidBasePos,
                    defaultDepth,
                    vertexBuffer, vertexDataPtr, totalCount, waterAaBB);
            break;
        case 0:
            parseMH2OVertexData<0, generateTexCoordsFromPos>
                (liquidInstance, liquidBasePos,
                    defaultDepth,
                    vertexBuffer, vertexDataPtr, totalCount, waterAaBB);
            break;
        case 1:
            parseMH2OVertexData<1, generateTexCoordsFromPos>
                (liquidInstance, liquidBasePos,
                    defaultDepth,
                    vertexBuffer, vertexDataPtr, totalCount, waterAaBB);
            break;
        case 2:
            parseMH2OVertexData<2, generateTexCoordsFromPos>
                (liquidInstance, liquidBasePos,
                    defaultDepth,
                    vertexBuffer, vertexDataPtr, totalCount, waterAaBB);
            break;
        case 3:
            parseMH2OVertexData<3, generateTexCoordsFromPos>
                (liquidInstance, liquidBasePos,
                    defaultDepth,
                    vertexBuffer, vertexDataPtr, totalCount, waterAaBB);
            break;
        case 4:
            parseMH2OVertexData<4, generateTexCoordsFromPos>
                (liquidInstance, liquidBasePos,
                    defaultDepth,
                    vertexBuffer, vertexDataPtr, totalCount, waterAaBB);
            break;
        case 5:
            parseMH2OVertexData<5, generateTexCoordsFromPos>
                (liquidInstance, liquidBasePos,
                    defaultDepth,
                    vertexBuffer, vertexDataPtr, totalCount, waterAaBB);
            break;
    }

}


void LiquidInstance::createAdtVertexData(const SMLiquidInstance &liquidInstance, const mathfu::vec3 &liquidBasePos,
                                         const PointerChecker<char> &mH2OBlob, CAaBox &waterAaBB,
                                         int liquidVertexFormat, bool generateTexCoordsFromPos,
                                         std::vector<LiquidVertexFormat> &vertexBuffer,
                                     std::vector<uint16_t> &indexBuffer) const {

    if (liquidInstance.liquid_object_or_lvf < 42) {
        //This is LVF override then
        liquidVertexFormat = liquidInstance.liquid_object_or_lvf;
    }

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

    uint8_t defaultDepth = 0;
    if (liquidVertexFormat == 2) {
        defaultDepth = 255;
    }
    if (vertexDataPtr == nullptr) {
        liquidVertexFormat = -1;
    }


    //Parse the blob
    if (generateTexCoordsFromPos) {
        vertexParseSelection<true>(liquidVertexFormat,
                             liquidInstance, liquidBasePos, defaultDepth,
                             vertexBuffer, vertexDataPtr, totalCount, waterAaBB);
    } else {
        vertexParseSelection<false>(liquidVertexFormat,
                             liquidInstance, liquidBasePos, defaultDepth,
                             vertexBuffer, vertexDataPtr, totalCount, waterAaBB);
    }


    uint8_t *existsTable = getLiquidExistsTable(mH2OBlob, liquidInstance);

    indexBuffer.reserve( (y_end - y_begin) * (x_end - x_begin) * 6);

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

void LiquidInstance::collectMeshes(COpaqueMeshCollector &opaqueMeshCollector) {
    if (m_api->getConfig()->renderLiquid) {
        opaqueMeshCollector.addWaterMesh(m_liquidMeshes[0]);
    }
}

void LiquidInstance::collectMeshes(framebased::vector<HGSortableMesh> &transparentMeshes) {
    if (m_api->getConfig()->renderLiquid) {
        transparentMeshes.emplace_back() = m_liquidMeshes[0];
    }
}

std::shared_ptr<LiquidInstanceEntityFactory> liquidInstanceFactory = std::make_shared<LiquidInstanceEntityFactory>();