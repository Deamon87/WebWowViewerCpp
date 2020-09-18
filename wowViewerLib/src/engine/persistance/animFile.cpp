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
            'ASFA',
            {
                [](AnimFile &file, ChunkData &chunkData) {
                    file.m_animFileAttachAnimDataBlob_len = chunkData.chunkLen;
                    chunkData.readValues(file.m_animFileAttachAnimDataBlob, chunkData.chunkLen);
                },
            }
        },
        {
            'BSFA',
            {
                [](AnimFile &file, ChunkData &chunkData) {
                    file.m_animFileBoneAnimDataBlob_len = chunkData.chunkLen;
                    chunkData.readValues(file.m_animFileBoneAnimDataBlob, chunkData.chunkLen);
                },
            }
        }
    }
};

void AnimFile::process(HFileContent animFile, const std::string &fileName) {
    m_animFile = animFile;
    auto &fileVec = *m_animFile.get();
    int chunk = *(uint32_t *) &fileVec[0];


    if (chunk == 'BSFA' || chunk == 'ASFA' || chunk == '2MFA') {
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

