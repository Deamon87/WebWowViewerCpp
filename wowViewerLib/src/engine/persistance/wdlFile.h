//
// Created by deamon on 11.01.18.
//

#ifndef WEBWOWVIEWERCPP_WDLFILE_H
#define WEBWOWVIEWERCPP_WDLFILE_H

#include <vector>
#include <string>
#include "helper/ChunkFileReader.h"
#include "header/adtFileHeader.h"
#include "../../include/wowScene.h"

class WdlFile : public PersistentFile {
public:
    WdlFile(std::string fileName){};
    WdlFile(int fileDataId){};

    void process(HFileContent wdlFile, const std::string &fileName) override;

public:
    SMDoodadDef * doodadDefObj = nullptr;
    int doodadDefObj_len = 0;

    SMMapObjDefObj1 * mapObjDefObj = nullptr;
    int mapObjDefObj_len = 0;


private:

    HFileContent m_wdlFile;
    static chunkDef<WdlFile> wdlFileTable;
};


#endif //WEBWOWVIEWERCPP_WDLFILE_H
