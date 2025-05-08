//
// Created by Deamon on 7/12/2025.
//

#ifndef WOWSTUDIO_LIQUIDMATERIALMANAGER_H
#define WOWSTUDIO_LIQUIDMATERIALMANAGER_H

#include <memory>
#include <unordered_map>
#include <mutex>
#include "../../../ApiContainer.h"
#include "../../../persistance/header/adtFileHeader.h"
#include "../../../persistance/PersistentFile.h"
#include "../../../cache/cache.h"
#include "../../../../gapi/interface/IDevice.h"
#include "../../../../include/custom_container_key.h"

class ILiquidDataBase;
class ILiquidMaterial;

// Minimal raw-blob file for fetching non-typed files by fileDataId
// (used for the magma 3D noise volume, blob 768431)
class RawBlobFile : public PersistentFile {
public:
    explicit RawBlobFile(int fileDataId) : PersistentFile(fileDataId) {};
    explicit RawBlobFile(const std::string &fileName) : PersistentFile(fileName) {};

    void process(HFileContent fileContent) override {
        data = fileContent;
        fsStatus = FileStatus::FSLoaded;
    }

    HFileContent data = nullptr;
};

class LiquidMaterialManager {
public:
    explicit LiquidMaterialManager(const HApiContainer &api, const HMapSceneBufferCreate &mapBuffCreator);

    std::shared_ptr<ILiquidMaterial> getLiquidMaterial(int liquid_object_or_lvf, int liquid_type, bool isInterior, const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide, uint8_t vtxCellWidth, uint8_t vtxCellHeight, uint8_t vtxCellOffsetX, uint8_t vtxCellOffsetY);
    void update(float currentTime);

private:
    const HApiContainer m_api;
    const HMapSceneBufferCreate m_mapBuffCreator;
    std::unordered_map<uint16_t, std::weak_ptr<ILiquidDataBase>> m_liquidDataCache;

    std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> m_waterPlacementChunk = nullptr;

    // Magma noise volume texture: blob fileDataId 768431 (128x128x16 RGBA,
    // uploaded as a 128x2048 2D atlas, bound at set=2 binding=7 of magma liquids)
    static constexpr int MAGMA_NOISE_FILE_DATA_ID = 768431;
    std::unique_ptr<Cache<RawBlobFile>> m_rawBlobCache;
    std::shared_ptr<RawBlobFile> m_magmaNoiseBlob;
    HGSamplableTexture m_magmaNoiseTex = nullptr;
    bool m_magmaNoiseUploaded = false;


    struct LiquidMaterialHashRecord {
        wtf::KeyContainer<std::weak_ptr<IBufferChunk<WMO::modelWideBlockVS>>> placementChunk;
        uint32_t liquidObjectId;
        bool isInterior;
        uint8_t vtxCellWidth;
        uint8_t vtxCellHeight;
        uint8_t vtxCellOffsetX;
        uint8_t vtxCellOffsetY;

        bool operator==(const LiquidMaterialHashRecord &other) const {
            return
                (placementChunk == other.placementChunk) &&
                (liquidObjectId == other.liquidObjectId) &&
                (isInterior == other.isInterior) &&
                (vtxCellWidth == other.vtxCellWidth) &&
                (vtxCellHeight == other.vtxCellHeight) &&
                (vtxCellOffsetX == other.vtxCellOffsetX) &&
                (vtxCellOffsetY == other.vtxCellOffsetY);
        };
    };

    struct LiquidMaterialRecordHasher {
        std::size_t operator()(const LiquidMaterialHashRecord& k) const {
            using std::hash;
            return hash<decltype(k.placementChunk)>{}(k.placementChunk) ^
                   (hash<uint32_t>{}(k.liquidObjectId) << 1) ^
                   (hash<bool>{}(k.isInterior) << 2) ^
                   (hash<uint8_t>{}(k.vtxCellWidth) << 3) ^
                   (hash<uint8_t>{}(k.vtxCellHeight) << 4) ^
                   (hash<uint8_t>{}(k.vtxCellOffsetX) << 5) ^
                   (hash<uint8_t>{}(k.vtxCellOffsetY) << 6);
        };
    };

    typedef std::unordered_map<LiquidMaterialHashRecord, std::weak_ptr<ILiquidMaterial>, LiquidMaterialRecordHasher> MaterialCachePerObject;

    MaterialCachePerObject m_liquidMaterialCachePerObject;
    mutable std::mutex m_liquidDataCacheMutex;
    mutable std::mutex m_liquidMaterialCacheMutex;

    std::shared_ptr<ILiquidDataBase> getLiquidTypeData(int liquidTypeId);
    std::shared_ptr<ILiquidMaterial> createLiquidMaterial(const std::shared_ptr<ILiquidDataBase> &liquidData,
                                                          const std::shared_ptr<IBufferChunk<WMO::modelWideBlockVS>> &modelWide,
                                                          const LiquidObjectRec &liquidObjRec,
                                                          bool isInterior,
                                                          uint8_t vtxCellWidth, uint8_t vtxCellHeight, uint8_t vtxCellOffsetX, uint8_t vtxCellOffsetY);
    void assignLiquidTextures(const std::shared_ptr<ILiquidDataBase> &liquidData, 
                            const std::array<std::string, 6> &textureStrings,
                            const std::array<uint8_t, 6> &frameCountTexture,
                            const std::vector<LiquidTextureData> &liquidTextureData,
                            int &waterDepthType);
    void updateLiquidDataAnimatedTextures(const std::shared_ptr<ILiquidDataBase> &liquidData, float currentTime);

    std::shared_ptr<ILiquidDataBase> createWaterLiquidData(const LiquidTypeAndMat &liquidTypeAndMat, const std::vector<LiquidTextureData> &liquidTextureData);
    std::shared_ptr<ILiquidDataBase> createMagmaLiquidData(const LiquidTypeAndMat &liquidTypeAndMat, const std::vector<LiquidTextureData> &liquidTextureData);
    std::shared_ptr<ILiquidDataBase> createMercuryLiquidData(const LiquidTypeAndMat &liquidTypeAndMat, const std::vector<LiquidTextureData> &liquidTextureData);
    std::shared_ptr<ILiquidDataBase> createFogLiquidData(const LiquidTypeAndMat &liquidTypeAndMat, const std::vector<LiquidTextureData> &liquidTextureData);
    std::shared_ptr<ILiquidDataBase> createLeyLineLiquidData(const LiquidTypeAndMat &liquidTypeAndMat, const std::vector<LiquidTextureData> &liquidTextureData);
    std::shared_ptr<ILiquidDataBase> createFelLiquidData(const LiquidTypeAndMat &liquidTypeAndMat, const std::vector<LiquidTextureData> &liquidTextureData);
    std::shared_ptr<ILiquidDataBase> createSwampLiquidData(const LiquidTypeAndMat &liquidTypeAndMat, const std::vector<LiquidTextureData> &liquidTextureData);
    std::shared_ptr<ILiquidDataBase> createAzeritheLiquidData(const LiquidTypeAndMat &liquidTypeAndMat, const std::vector<LiquidTextureData> &liquidTextureData);
};

#endif //WOWSTUDIO_LIQUIDMATERIALMANAGER_H
