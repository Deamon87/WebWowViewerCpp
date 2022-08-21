//
// Created by Deamon on 8/6/2019.
//

#ifndef AWEBWOWVIEWERCPP_SKELFILE_H
#define AWEBWOWVIEWERCPP_SKELFILE_H

#include "helper/ChunkFileReader.h"
#include "header/skelFileHeader.h"
#include "../../include/sharedFile.h"
#include "PersistentFile.h"
#include "animFile.h"
#include <vector>
#include <unordered_map>
#include "../ApiContainer.h"

class SkelFile : public PersistentFile {
public:
    SkelFile (std::string fileName){};
    SkelFile(int fileDataId){};

    void process(HFileContent animFile, const std::string &fileName) override;

    void setPostLoad(std::function<void ()> postLoadFunction) {
        this->m_postLoadFunction = postLoadFunction;
    };
public:
    skeleton_l_header *m_skl1 = 0;
    int m_skl1_len = -1;

    skeleton_attachment_header *m_ska1 = 0;
    int m_ska1_len = -1;

    skeleton_bone_header *m_skb1 = 0;
    int m_skb1_len = -1;

    skeleton_sequence_header *m_sks1 = 0;
    int m_sks1_len = -1;

    skeleton_parent_data *m_skpd = 0;
    int m_skpd_len = -1;


    std::vector<M2_AFID> animationFileDataIDs;

    void loadLowPriority(HApiContainer m_api, uint32_t animationId, uint32_t variationId);
private:
    HFileContent m_skelFile;
    static chunkDef<SkelFile> skelFileTable;

    std::function<void ()> m_postLoadFunction = nullptr;



    void initTracks(CSkelSequenceLoad *cm2SequenceLoad);
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


#endif //AWEBWOWVIEWERCPP_SKELFILE_H
