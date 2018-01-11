//
// Created by deamon on 11.01.18.
//

#include "wdlFile.h"

chunkDef<WdlFile> WdlFile::wdlFileTable = {
    handler : [](WdlFile &file, ChunkData &chunkData) {},
    subChunks : {
        {
            'MLDD',
            {
                handler: [](WdlFile &file, ChunkData &chunkData) {
                    debuglog("Entered MLDD");

                    file.doodadDefObj_len = chunkData.chunkLen / sizeof(SMDoodadDef);
                    chunkData.readValues(file.doodadDefObj, file.doodadDefObj_len);
                }
            }
        },
        {
            'MLMD',
            {
                handler: [](WdlFile& file, ChunkData& chunkData){
                    debuglog("Entered MLMD");

                    file.mapObjDefObj_len = chunkData.chunkLen / sizeof(SMMapObjDefObj1);
                    chunkData.readValues(file.mapObjDefObj, file.mapObjDefObj_len);
                }
            }
        },
    }
};

void WdlFile::process(std::vector<unsigned char> &wdlFile) {
    m_wdlFile = std::vector<uint8_t>(wdlFile);
    CChunkFileReader reader(m_wdlFile);
    reader.processFile(*this, &WdlFile::wdlFileTable);

    m_loaded = true;
}
