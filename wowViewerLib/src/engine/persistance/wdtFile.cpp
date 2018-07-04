#include "wdtFile.h"

chunkDef<WdtFile> WdtFile::wdtFileTable = {
    [](WdtFile &file, ChunkData &chunkData) {},
    {
        {
            'MPHD',
            {
                    [](WdtFile &file, ChunkData &chunkData) {
                        debuglog("Entered MPHD");
                        chunkData.readValue(file.mphd);
                    }
            }
        },
        {
            'MAIN',
            {
                [](WdtFile &file, ChunkData &chunkData) {
                    debuglog("Entered MAIN");
                    chunkData.readValue(file.mapTileTable);
                }
            },
        },
        {
            'MWMO',
            {
                [](WdtFile &file, ChunkData &chunkData) {
                    debuglog("Entered MWMO");
                    char * fileName;
                    chunkData.readValues(fileName, chunkData.chunkLen);
                    file.wmoFileName = std::string(fileName, chunkData.chunkLen);
                }
            },
        },
        {
            'MODF',
            {
                [](WdtFile &file, ChunkData &chunkData) {
                    debuglog("Entered MODF");
                    chunkData.readValue(file.wmoDef);
                }
            },
        }
    }
};

void WdtFile::process(std::vector<unsigned char> &wdtFile, std::string &fileName) {
    m_wdtFile = std::vector<uint8_t>(wdtFile);
    CChunkFileReader reader(m_wdtFile);
    reader.processFile(*this, &WdtFile::wdtFileTable);

    m_loaded = true;
}
