//
// Created by deamon on 19.06.17.
//

#ifndef WOWVIEWERLIB_M2GEOM_H
#define WOWVIEWERLIB_M2GEOM_H

#include <vector>
#include <unordered_map>
#include "../persistance/header/M2FileHeader.h"
#include "../persistance/animFile.h"
#include "../../gapi/interface/IDevice.h"
#include "../persistance/helper/ChunkFileReader.h"
#include "../ApiContainer.h"
#include "../persistance/header/skinFileHeader.h"


class M2Geom : public PersistentFile {
public:
    M2Geom(std::string fileName){
        std::string delimiter = ".";
        std::string nameTemplate = fileName.substr(0, fileName.find_last_of(delimiter));
        std::string modelFileName = nameTemplate + ".m2";

        this->m_modelName = modelFileName;
        this->m_nameTemplate= nameTemplate;
    };
    M2Geom(int fileDataId){
        useFileId = true;
        m_modelFileId = fileDataId;
    };

    std::string getName() {
        return m_modelName;
    }

    void process(HFileContent m2File, const std::string &fileName) override;
    HGVertexBuffer getVBO(IDevice &device);
    HGVertexBufferBindings getVAO(IDevice &device, SkinGeom *skinGeom);
    std::array<HGVertexBufferBindings, 4> createDynamicVao(IDevice &device, std::array<HGVertexBufferDynamic, 4> &dynVBOs,
                                                               SkinGeom *skinGeom, M2SkinSection *skinSection);
    void loadLowPriority(HApiContainer m_api, uint32_t animationId, uint32_t subAnimationId);

    M2Data * getM2Data(){ if (fsStatus == FileStatus::FSLoaded) {return m_m2Data;} else {return nullptr;}};

    M2Data *m_m2Data = nullptr;
    std::vector<uint32_t> skinFileDataIDs;
    std::vector<uint32_t> recursiveFileDataIDs;
    std::vector<uint32_t> particleModelFileDataIDs;
    std::vector<uint32_t> textureFileDataIDs;
    std::vector<M2_AFID> animationFileDataIDs;

    std::vector<uint16_t> blackListAnimations;

    EXP2 *exp2 = nullptr;
    std::vector<TXAC> txacMesh = {};
    std::vector<TXAC> txacMParticle = {};

    int m_skid = -1;
    WaterFallDataV3 *m_wfv3 = 0;
    WaterFallDataV3 *m_wfv1 = 0;


private:
    HFileContent m2File;

    std::string m_modelName;
    std::string m_nameTemplate = "";

    bool useFileId = false;
    int m_modelFileId;

    HGVertexBuffer vertexVbo = HGVertexBuffer(nullptr);
    std::unordered_map<SkinGeom *, HGVertexBufferBindings> vaoMap;


    static chunkDef<M2Geom> m2FileTable;

    void initTracks(CM2SequenceLoad *cm2SequenceLoad);
    struct AnimCacheRecord {
        uint32_t animationId;
        uint32_t variationId;

        bool operator==(const AnimCacheRecord &other) const {
            return
                (animationId == other.animationId) &&
                (variationId == other.variationId);
        };
    };
    struct AnimCacheRecordHasher {
        std::size_t operator()(const AnimCacheRecord& k) const {
            using std::hash;
            return hash<uint32_t >{}(k.animationId) ^ (hash<uint32_t>{}(k.variationId) << 8);
        };
    };

    std::unordered_map<AnimCacheRecord, std::shared_ptr<AnimFile>, AnimCacheRecordHasher> loadedAnimationMap;
};
typedef std::shared_ptr<M2Geom> HM2Geom;

#endif //WOWVIEWERLIB_M2GEOM_H
