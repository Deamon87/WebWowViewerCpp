//
// Created by deamon on 01.08.17.
//

#ifndef WOWVIEWERLIB_ADTFILE_H
#define WOWVIEWERLIB_ADTFILE_H
#include "helper/ChunkFileReader.h"
#include "header/adtFileHeader.h"

struct mcnkStruct_t {
    MCVT *mcvt;
    MCCV *mccv;
    SMNormal *mcnr;
    SMLayer *mcly;
    uint8_t *mcrf;

    uint8_t *mcal;
};

class AdtFile {
public:
    AdtFile(std::vector<unsigned char> &adtFile);
    std::vector<uint8_t> processTexture(int wdtObjFlags, int i);
public:
    SMMapHeader* mhdr;

    struct {
        SMChunkInfo chunkInfo[16][16];
    } *mcins;

    std::vector<std::string> textureNames;

    char *doodadNamesField;
    int doodadNamesFieldLen;

    char *wmoNamesField;
    int wmoNamesFieldLen;

    int mcnkRead = -1;
    SMChunk mapTile[16*16];
    mcnkStruct_t mcnkStructs[16*16];

    std::vector<int16_t> strips;
    std::vector<int> stripOffsets;
private:
    void createTriangleStrip();
    static chunkDef<AdtFile> adtFileTable;
    std::vector<unsigned char> m_adtFile;
};

#endif //WOWVIEWERLIB_ADTFILE_H
