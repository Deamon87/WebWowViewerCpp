//
// Created by Deamon on 7/12/2025.
//

#include "LiquidMaterialManager.h"
#include "../LiquidDataGetters.h"
#include "../../../algorithms/mathHelper.h"


LiquidMaterialManager::LiquidMaterialManager(const HApiContainer &api, const HMapSceneBufferCreate &mapBuffCreator) : m_api(api), m_mapBuffCreator(mapBuffCreator) {
    //Placement chunk for ADTs
    m_waterPlacementChunk = mapBuffCreator->createWMOWideChunk(0)->m_placementMatrix;
    m_waterPlacementChunk->getObject().uPlacementMat = mathfu::mat4::Identity();
    m_waterPlacementChunk->save();

    // Kick off the magma noise volume blob fetch; the texture handle is bound eagerly
    // (content arrives async and is uploaded in update())
    if (m_api->requestProcessor) {
        m_rawBlobCache = std::make_unique<Cache<RawBlobFile>>(m_api->requestProcessor.get(), CacheHolderType::CACHE_BLP);
        m_magmaNoiseBlob = m_rawBlobCache->getFileId(MAGMA_NOISE_FILE_DATA_ID);
    }
    m_magmaNoiseTex = m_api->hDevice->createTexture(true, true);
}

std::shared_ptr<ILiquidMaterial> LiquidMaterialManager::getLiquidMaterial(
    int liquid_object_or_lvf, int liquid_type,
    bool isInterior,
    const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide,
    uint8_t vtxCellWidth, uint8_t vtxCellHeight,
    uint8_t vtxCellOffsetX, uint8_t vtxCellOffsetY
) {
    if (m_api->databaseHandler == nullptr) {
        return nullptr;
    }

    auto placementChunk = modelWide ? modelWide : m_waterPlacementChunk;

    uint32_t liquidObjectId = liquid_object_or_lvf > 41 ? liquid_object_or_lvf : liquid_type;

    // Check if we have this liquid material cached
    LiquidMaterialHashRecord hashRecord = {
        .placementChunk = std::weak_ptr(placementChunk),
        .liquidObjectId = liquidObjectId,
        .isInterior = isInterior,
        .vtxCellWidth = vtxCellWidth,
        .vtxCellHeight = vtxCellHeight,
        .vtxCellOffsetX = vtxCellOffsetX,
        .vtxCellOffsetY = vtxCellOffsetY
    };

    {
        std::lock_guard<std::mutex> lock(m_liquidMaterialCacheMutex);

        auto weakPlacementChunk = std::weak_ptr(placementChunk);

        auto it_cache = m_liquidMaterialCachePerObject.find(hashRecord);
        if (it_cache != m_liquidMaterialCachePerObject.end()) {
            if (auto liquidMaterial = it_cache->second.lock()) {
                return liquidMaterial;
            }
            m_liquidMaterialCachePerObject.erase(it_cache);
        }
    }

    LiquidObjectRec liquidObjRec;
    liquidObjRec.liquid_object_or_lvf = liquid_object_or_lvf;
    liquidObjRec.liquidTypeId = liquid_type;
    if (liquid_object_or_lvf > 41) {
        m_api->databaseHandler->getLiquidObjectData(liquid_object_or_lvf, liquid_type, liquidObjRec);
    }

    auto liquidData = getLiquidTypeData(liquidObjRec.liquidTypeId);
    if (!liquidData) {
        return nullptr;
    }

    auto liquidMaterial = createLiquidMaterial(liquidData, placementChunk, liquidObjRec, isInterior,
                                               vtxCellWidth, vtxCellHeight, vtxCellOffsetX, vtxCellOffsetY);

    if (liquidMaterial) {
        std::lock_guard<std::mutex> lock(m_liquidMaterialCacheMutex);

        m_liquidMaterialCachePerObject[hashRecord] = liquidMaterial;
    }

    return liquidMaterial;
}

std::shared_ptr<ILiquidMaterial> LiquidMaterialManager::createLiquidMaterial(
    const std::shared_ptr<ILiquidDataBase> &liquidData,
    const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide,
    const LiquidObjectRec &liquidObjRec,
    bool isInterior,
    uint8_t vtxCellWidth, uint8_t vtxCellHeight, uint8_t vtxCellOffsetX, uint8_t vtxCellOffsetY
) {
    std::shared_ptr<ILiquidMaterial> liquidMaterial;

    switch (liquidData->liquidMaterial) {
        case 1:
        case 3:
            if (auto waterData = std::dynamic_pointer_cast<WaterLiquidData>(liquidData)) {
                liquidMaterial = m_mapBuffCreator->createWaterLiquidMaterial(modelWide, waterData);
            }
            break;
        case 2:
        case 4:
            if (auto magmaData = std::dynamic_pointer_cast<MagmaLiquidData>(liquidData)) {
                liquidMaterial = m_mapBuffCreator->createMagmaLiquidMaterial(modelWide, magmaData);
            }
            break;
        case 5:
            if (auto mercuryData = std::dynamic_pointer_cast<MercuryLiquidData>(liquidData)) {
                liquidMaterial = m_mapBuffCreator->createMercuryLiquidMaterial(modelWide, mercuryData);
            }
            break;
        case 10:
            if (auto fogData = std::dynamic_pointer_cast<FogLiquidData>(liquidData)) {
                liquidMaterial = m_mapBuffCreator->createFogLiquidMaterial(modelWide, fogData);
            }
            break;
        case 12:
            if (auto leyLineData = std::dynamic_pointer_cast<LeyLineLiquidData>(liquidData)) {
                liquidMaterial = m_mapBuffCreator->createLeyLineLiquidMaterial(modelWide, leyLineData);
            }
            break;
        case 13:
            if (auto felData = std::dynamic_pointer_cast<FelLiquidData>(liquidData)) {
                liquidMaterial = m_mapBuffCreator->createFelLiquidMaterial(modelWide, felData);
            }
            break;
        case 14:
            if (auto swampData = std::dynamic_pointer_cast<SwampLiquidData>(liquidData)) {
                liquidMaterial = m_mapBuffCreator->createSwampLiquidMaterial(modelWide, swampData);
            }
            break;
        case 18:
            if (auto azeritheData = std::dynamic_pointer_cast<AzeritheLiquidData>(liquidData)) {
                liquidMaterial = m_mapBuffCreator->createAzeritheLiquidMaterial(modelWide, azeritheData);
            }
            break;
        default:
            if (auto waterData = std::dynamic_pointer_cast<WaterLiquidData>(liquidData)) {
                liquidMaterial = m_mapBuffCreator->createWaterLiquidMaterial(modelWide, waterData);
            }
            break;
    }

    if (!liquidMaterial) return nullptr;

    //Update Liquid instance data
    {
        auto &liquidInstanceData = liquidMaterial->m_instance->getObject();
        liquidInstanceData.liquidMaterial = liquidData->liquidMaterial;
        liquidInstanceData.flowDirection = liquidObjRec.flowDirection;
        liquidInstanceData.flowSpeed = liquidObjRec.flowSpeed;
        liquidInstanceData.isInterior = isInterior ? 1 : 0;
        liquidInstanceData.vtxCellWidthHeight =
            (static_cast<int>(vtxCellOffsetX) << 24) | (static_cast<int>(vtxCellOffsetY) << 16) |
            (static_cast<int>(vtxCellWidth) << 8) | static_cast<int>(vtxCellHeight);

        liquidMaterial->m_instance->save();
    }

    liquidMaterial->generateTexCoordsFromPos = getLiquidSettings(
                                                     liquidObjRec.liquid_object_or_lvf,
                                                     liquidObjRec.liquidTypeId,
                                                     liquidData->liquidMaterial,
                                                     false).generateTexCoordsFromPos;

    liquidMaterial->generateTexCoordsFromPos = !isInterior;

    liquidMaterial->matLVF = liquidData->matLVF;

    return liquidMaterial;
}

std::shared_ptr<ILiquidDataBase> LiquidMaterialManager::getLiquidTypeData(int liquidTypeId) {
    // Check if we have this liquid data cached
    {
        std::lock_guard<std::mutex> lock(m_liquidDataCacheMutex);
        auto it = m_liquidDataCache.find(liquidTypeId);
        if (it != m_liquidDataCache.end()) {
            if (auto liquidData = it->second.lock()) {
                return liquidData;
            }
            // If the weak_ptr is expired, remove it from cache
            m_liquidDataCache.erase(it);
        }
    }

    LiquidTypeAndMat liquidTypeAndMat;

    liquidTypeAndMat.flags = 0;
    liquidTypeAndMat.matLVF = 0;

    std::vector<LiquidTextureData> liquidTextureData;
    m_api->databaseHandler->getLiquidTypeData(liquidTypeId, liquidTypeAndMat, liquidTextureData);

    if (liquidTypeId == 2) {
        liquidTypeAndMat.matLVF = 2;
    }

    // Create appropriate liquid data based on material ID
    std::shared_ptr<ILiquidDataBase> liquidData;
    switch (liquidTypeAndMat.materialID) {
        case 1:
        case 3:
            liquidData = createWaterLiquidData(liquidTypeAndMat, liquidTextureData);
            break;
        case 2:
        case 4:
            liquidData = createMagmaLiquidData(liquidTypeAndMat, liquidTextureData);
            break;
        case 5:
            liquidData = createMercuryLiquidData(liquidTypeAndMat, liquidTextureData);
            break;
        case 10:
            liquidData = createFogLiquidData(liquidTypeAndMat, liquidTextureData);
            break;
        case 12:
            liquidData = createLeyLineLiquidData(liquidTypeAndMat, liquidTextureData);
            break;
        case 13:
            liquidData = createFelLiquidData(liquidTypeAndMat, liquidTextureData);
            break;
        case 14:
            liquidData = createSwampLiquidData(liquidTypeAndMat, liquidTextureData);
            break;
        case 18:
            liquidData = createAzeritheLiquidData(liquidTypeAndMat, liquidTextureData);
            break;
        default:
            liquidData = createWaterLiquidData(liquidTypeAndMat, liquidTextureData); // Default to water
            break;
    }

    liquidData->matLVF = liquidTypeAndMat.matLVF;

    // Cache the created liquid data
    {
        std::lock_guard<std::mutex> lock(m_liquidDataCacheMutex);
        m_liquidDataCache[liquidTypeId] = liquidData;
    }
    return liquidData;
}

void LiquidMaterialManager::assignLiquidTextures(const std::shared_ptr<ILiquidDataBase> &liquidData,
                                                 const std::array<std::string, 6> &textureStrings,
                                                 const std::array<uint8_t, 6> &frameCountTexture,
                                                 const std::vector<LiquidTextureData> &liquidTextureData,
                                                 int &waterDepthType) {

    int textureSlot = 0;
    int insertedForThisSlot = 0;
    if (liquidTextureData.size() > 0) {
        //Client has FileDataIds and LiquidTypeXTexture table
        for (size_t i = 0; i < liquidTextureData.size(); ++i) {
            const auto &textureData = liquidTextureData[i];

            HGSamplableTexture liquidTex = nullptr;
            if (textureData.fileDataId != 0)
            {
                auto htext = m_api->cacheStorage->getTextureCache()->getFileId(liquidTextureData[i].fileDataId);
                liquidTex = m_api->hDevice->createBlpTexture(htext, true, true);
            } else {
                //Type 0 = proceduralOceanDepthTex
                //Type 1 = proceduralRiverDepthTex
                //Type 2 = proceduralWmoWaterTex

                waterDepthType = textureData.type;

                liquidTex = m_api->hDevice->getBlackTexturePixel();
            }

            liquidData->usedTextures[textureSlot].push_back(liquidTex);
            insertedForThisSlot++;

            if (frameCountTexture[textureSlot] == 0 || insertedForThisSlot == frameCountTexture[textureSlot] ) {
                textureSlot++;
                insertedForThisSlot = 0;
            }

            if (textureSlot >= MAX_LIQUID_TYPE_TEXTURES) {
                std::cout << "[ERROR] Something went wrong with textures for " << liquidData->liquidMaterial << std::endl;
            }
        }

        //Update DS with textures
        updateLiquidDataAnimatedTextures(liquidData, 0);
    } else {
        //Client doesn't have those. So use filenames
    }

}

template<>
void LiquidData<Liquid::WaterData>::updateData() {

}

void LiquidMaterialManager::updateLiquidDataAnimatedTextures(const std::shared_ptr<ILiquidDataBase> &liquidData, float currentTime) {
    std::array<HGSamplableTexture, MAX_LIQUID_TYPE_TEXTURES + 1> textures;
    for (size_t i = 0; i < MAX_LIQUID_TYPE_TEXTURES; ++i) {
        auto &usedTextures = liquidData->usedTextures[i];
        if (!usedTextures.empty()) {
            //scenetime in seconds divide by number of textures
            int textureAnimIndex = (((int)(currentTime * 0.001f)) % usedTextures.size());

            assert(textureAnimIndex < usedTextures.size());

            textures[i] = usedTextures[textureAnimIndex];
        } else {
            textures[i] = m_api->hDevice->getBlackTexturePixel();
        }
    }
    // Slot 7 is material-specific (fel's second normal frame, magma's noise volume);
    // filled per type via resolveAnimatedTextures inside updateAnimatedTextures
    textures[MAX_LIQUID_TYPE_TEXTURES] = nullptr;

    liquidData->updateAnimatedTextures(textures, currentTime);
}

void LiquidMaterialManager::update(float currentTime) {
    // Upload the magma noise volume once its blob arrives (128x128x16 RGBA as 128x2048 2D atlas)
    if (!m_magmaNoiseUploaded && m_magmaNoiseBlob) {
        auto status = m_magmaNoiseBlob->getStatus();
        if (status == FileStatus::FSLoaded && m_magmaNoiseBlob->data) {
            auto &blob = *m_magmaNoiseBlob->data;
            const int width = 128, height = 128 * 16;
            if ((int)blob.size() >= width * height * 4) {
                m_magmaNoiseTex->getTexture()->loadData(width, height, blob.data(), ITextureFormat::itRGBA);
            } else {
                std::cout << "[ERROR] Magma noise blob has unexpected size " << blob.size() << std::endl;
            }
            m_magmaNoiseUploaded = true;
        } else if (status == FileStatus::FSRejected) {
            std::cout << "[ERROR] Magma noise blob " << MAGMA_NOISE_FILE_DATA_ID << " was rejected" << std::endl;
            m_magmaNoiseUploaded = true;
        }
    }

    // Create a list of keys to remove
    std::vector<uint16_t> keysToRemove;

    // Go through all cached liquid data
    {
        std::lock_guard<std::mutex> lock(m_liquidDataCacheMutex);
        for (auto it = m_liquidDataCache.begin(); it != m_liquidDataCache.end(); ++it) {
            if (auto liquidData = it->second.lock()) {
                // If the weak_ptr is still valid, update the animated textures
                updateLiquidDataAnimatedTextures(liquidData, currentTime);
            } else {
                // If the weak_ptr is expired, mark this key for removal
                keysToRemove.push_back(it->first);
            }
        }

        // Remove all expired entries
        for (const auto& key : keysToRemove) {
            m_liquidDataCache.erase(key);
        }
    }
}
