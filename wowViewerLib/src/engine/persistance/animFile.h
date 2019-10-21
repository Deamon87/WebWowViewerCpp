//
// Created by deamon on 26.11.18.
//

#ifndef AWEBWOWVIEWERCPP_ANIMFILE_H
#define AWEBWOWVIEWERCPP_ANIMFILE_H

#include "helper/ChunkFileReader.h"
#include "../../include/sharedFile.h"
#include <vector>

class AnimFile {
public:
    AnimFile(std::string fileName){};
    AnimFile(int fileDataId){};

    void process(HFileContent animFile, const std::string &fileName);
    bool getIsLoaded() { return m_loaded; };

    void setPostLoad(std::function<void ()> postLoadFunction) {
        this->m_postLoadFunction = postLoadFunction;
    };
public:
    uint8_t *m_animFileDataBlob = 0;
    int m_animFileDataBlob_len = -1;

private:
    bool m_loaded = false;

    HFileContent m_animFile;
    static chunkDef<AnimFile> animFileTable;

    std::function<void ()> m_postLoadFunction = nullptr;
};


#endif //AWEBWOWVIEWERCPP_ANIMFILE_H
