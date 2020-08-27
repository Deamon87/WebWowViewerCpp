//
// Created by deamon on 11.01.18.
//
#include "wdlFile.h"


chunkDef<WdlFile> WdlFile::wdlFileTable = {
    [](WdlFile &file, ChunkData &chunkData) {},
    {
        {
            'MLDD',
            {
                [](WdlFile &file, ChunkData &chunkData) {
                    debuglog("Entered MLDD");

                    file.doodadDefObj_len = chunkData.chunkLen / sizeof(SMDoodadDef);
                    chunkData.readValues(file.doodadDefObj, file.doodadDefObj_len);
                }
            }
        },
        {
            'MLMD',
            {
                [](WdlFile& file, ChunkData& chunkData){
                    debuglog("Entered MLMD");

                    file.mapObjDefObj_len = chunkData.chunkLen / sizeof(SMMapObjDefObj1);
                    chunkData.readValues(file.mapObjDefObj, file.mapObjDefObj_len);
                }
            }
        },
        {
            'MSSO',
            {
                [](WdlFile& file, ChunkData& chunkData){
                    debuglog("Entered MSSO");

                    file.m_msso_len = chunkData.chunkLen / sizeof(msso_t);

                    chunkData.readValues(file.m_msso, file.m_msso_len);
                }
            }
        },
        {
            'MSSN',
            {
                [](WdlFile& file, ChunkData& chunkData){
                    debuglog("Entered MSSN");

                    file.m_mssn_len = chunkData.chunkLen / sizeof(mssn_t);

                    chunkData.readValues(file.m_mssn, file.m_mssn_len);
                }
            }
        }, {
            'MSSC',
            {
                [](WdlFile& file, ChunkData& chunkData){
                    debuglog("Entered MSSC");

                    file.m_mssc_len = chunkData.chunkLen / sizeof(mssc_t);

                    chunkData.readValues(file.m_mssc, file.m_mssc_len);
                }
            }
        }
    }
};

void WdlFile::process(HFileContent wdlFile, const std::string &fileName) {
    m_wdlFile = wdlFile;
    CChunkFileReader reader(*m_wdlFile.get());
    reader.processFile(*this, &WdlFile::wdlFileTable);

    fsStatus = FileStatus::FSLoaded;
}
