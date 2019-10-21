//
// Created by Deamon on 8/6/2019.
//

#ifndef AWEBWOWVIEWERCPP_SKELFILE_H
#define AWEBWOWVIEWERCPP_SKELFILE_H

#include "helper/ChunkFileReader.h"
#include "header/skelFileHeader.h"
#include "../../include/sharedFile.h"
#include <vector>

class SkelFile {
public:
    SkelFile (std::string fileName){};
    SkelFile(int fileDataId){};

    void process(HFileContent animFile, const std::string &fileName);
    bool getIsLoaded() { return m_loaded; };

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

private:
    bool m_loaded = false;

    HFileContent m_skelFile;
    static chunkDef<SkelFile> skelFileTable;

    std::function<void ()> m_postLoadFunction = nullptr;
};


#endif //AWEBWOWVIEWERCPP_SKELFILE_H
