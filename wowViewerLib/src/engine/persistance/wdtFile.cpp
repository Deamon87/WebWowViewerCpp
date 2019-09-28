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
            'MAID',
            {
                [](WdtFile &file, ChunkData &chunkData) {
                    debuglog("Entered MAID");
                    int *ptr123 ;
                    chunkData.readValues(ptr123, chunkData.chunkLen/4);

//                    for (int i = 0; i < chunkData.chunkLen/4; i++) {
//                        std::cout << "i = " << ptr123[i] << std::endl;
//                    }
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

void WdtFile::process(HFileContent wdtFile, const std::string &fileName) {
    m_wdtFile = wdtFile;
    CChunkFileReader reader(*m_wdtFile.get());
    reader.processFile(*this, &WdtFile::wdtFileTable);

    m_loaded = true;
}
