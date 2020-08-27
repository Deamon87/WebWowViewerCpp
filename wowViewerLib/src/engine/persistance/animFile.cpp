//
// Created by deamon on 26.11.18.
//

#include "animFile.h"

chunkDef<AnimFile> AnimFile::animFileTable = {
    [](AnimFile &file, ChunkData &chunkData) {},
    {
        {
            '2MFA',
            {
                [](AnimFile &file, ChunkData &chunkData) {
                    file.m_animFileDataBlob_len = chunkData.chunkLen;
                    chunkData.readValues(file.m_animFileDataBlob, chunkData.chunkLen);
                },
            }
        },
        {
            'AFSA',
            {
                [](AnimFile &file, ChunkData &chunkData) {


                },
            }
        },
        {
            'AFSB',
            {
                [](AnimFile &file, ChunkData &chunkData) {


                },
            }
        }
    }
};

void AnimFile::process(HFileContent animFile, const std::string &fileName) {
    m_animFile = animFile;
    auto &fileVec = *m_animFile.get();
    if (

        fileVec[0] == 'A' &&
        fileVec[1] == 'F' &&
        fileVec[2] == 'M' &&
        fileVec[3] == '2'
    ) {
        CChunkFileReader reader(fileVec);
        reader.processFile(*this, &AnimFile::animFileTable);
    } else {
        m_animFileDataBlob = (uint8_t *) &fileVec[0];
        m_animFileDataBlob_len = m_animFile->size();
    }

    fsStatus = FileStatus::FSLoaded;

    if (m_postLoadFunction != nullptr) {
        m_postLoadFunction();
    }
}

