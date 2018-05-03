//
// Created by deamon on 03.05.18.
//

#ifndef WEBWOWVIEWERCPP_TEXFILE_H
#define WEBWOWVIEWERCPP_TEXFILE_H

#include <vector>
#include "helper/ChunkFileReader.h"
#include "header/texFileHeader.h"

class TexFile {
public:
    TexFile() {};
    void process(std::vector<unsigned char> &wdlFile);
    bool getIsLoaded() { return m_loaded; };
public:
    SBlobTexture * entries = nullptr;
    int entriesNum = -1;

    char *filenames = nullptr;
    int filenames_len = -1;

    char * textureData = nullptr;
    int textureData_len = -1;
private:
    bool m_loaded = false;

    std::vector<unsigned char> m_texFile;
    static chunkDef<TexFile> texFileTable;
};


#endif //WEBWOWVIEWERCPP_TEXFILE_H
