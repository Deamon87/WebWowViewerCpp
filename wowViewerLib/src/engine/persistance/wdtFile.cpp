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
                    chunkData.readValue(file.mapFileDataIDs);
                }
            }
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

void WdtFile::process(HFileContent wdtFile) {
    m_wdtFile = wdtFile;
    CChunkFileReader reader(*m_wdtFile.get(), getFileNameOrDataId());
    reader.processFile(*this, &WdtFile::wdtFileTable);

    if (mphd != nullptr) {
        fsStatus = FileStatus::FSLoaded;
    } else {
        //if MPHD is not present - WDT file is mailformed
        fsStatus = FileStatus::FSRejected;
    }
}
