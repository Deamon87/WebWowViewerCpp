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
        },
        {
            'MSSC',
            {
                [](WdlFile& file, ChunkData& chunkData){
                    debuglog("Entered MSSC");

                    file.m_mssc_len = chunkData.chunkLen / sizeof(mssc_t);

                    chunkData.readValues(file.m_mssc, file.m_mssc_len);
                }
            }
        },
        {
            'MSLI',
            {
                [](WdlFile& file, ChunkData& chunkData){
                    debuglog("Entered MSLI");

                    file.m_msli_len = chunkData.chunkLen / sizeof(uint32_t);

                    chunkData.readValues(file.m_msli, file.m_msli_len);
                }
            }
        },
        {
            'MSLD',
            {
                [](WdlFile& file, ChunkData& chunkData){
                    debuglog("Entered MSLD");

                    file.m_msld_len = chunkData.chunkLen / sizeof(msld_t);

                    chunkData.readValues(file.m_msld, file.m_msld_len);
                }
            }
        },
        {
            'MSSF',
            {
                [](WdlFile& file, ChunkData& chunkData){
                    debuglog("Entered MSSF");

                    chunkData.readValues(file.m_mssf, chunkData.chunkLen/8);
                }
            }
        }
    }
};

void WdlFile::process(HFileContent wdlFile, const std::string &fileName) {
    m_wdlFile = wdlFile;
    CChunkFileReader reader(*m_wdlFile.get(), fileName);
    reader.processFile(*this, &WdlFile::wdlFileTable);

    fsStatus = FileStatus::FSLoaded;
}
