//
// Created by deamon on 11.01.18.
//

#ifndef WEBWOWVIEWERCPP_WDLFILE_H
#define WEBWOWVIEWERCPP_WDLFILE_H

#include <vector>
#include <string>
#include "helper/ChunkFileReader.h"
#include "header/adtFileHeader.h"

class WdlFile {
public:
   WdlFile() {};

    void process(std::vector<unsigned char> &wdlFile, std::string &fileName);
    bool getIsLoaded() { return m_loaded; };
public:
    SMDoodadDef * doodadDefObj = nullptr;
    int doodadDefObj_len = -1;

    SMMapObjDefObj1 * mapObjDefObj = nullptr;
    int mapObjDefObj_len = -1;


private:
    bool m_loaded = false;

    std::vector<unsigned char> m_wdlFile;
    static chunkDef<WdlFile> wdlFileTable;
};


#endif //WEBWOWVIEWERCPP_WDLFILE_H
