#ifndef WEBWOWVIEWERCPP_WDTFILE_H_H
#define WEBWOWVIEWERCPP_WDTFILE_H_H

#include "header/wdtFileHeader.h"
#include "helper/ChunkFileReader.h"
#include "header/adtFileHeader.h"
#include "../../include/wowScene.h"

class WdtFile {
public:
    WdtFile(std::string fileName){};
    WdtFile(int fileDataId){};

    void process(const std::vector<unsigned char> &wdtFile, const std::string &fileName);
    bool getIsLoaded() { return m_loaded; };
public:
    MPHD *mphd;
    struct {
        MAIN mainInfo[64][64];
    } *mapTileTable;

    std::string wmoFileName;
    SMMapObjDef *wmoDef;

private:
    bool m_loaded = false;

    std::vector<unsigned char> m_wdtFile;
    static chunkDef<WdtFile> wdtFileTable;
};

#endif //WEBWOWVIEWERCPP_WDTFILE_H_H
