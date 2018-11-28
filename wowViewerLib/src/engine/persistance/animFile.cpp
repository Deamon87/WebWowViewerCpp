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

void AnimFile::process(const std::vector<unsigned char> &animFile, const std::string &fileName) {
    m_animFile = std::vector<uint8_t>(animFile);
    if (
        m_animFile[0] == 'A' &&
        m_animFile[1] == 'F' &&
        m_animFile[2] == 'M' &&
        m_animFile[3] == '2'
    ) {
        CChunkFileReader reader(m_animFile);
        reader.processFile(*this, &AnimFile::animFileTable);
    } else {
        m_animFileDataBlob = (uint8_t *) &m_animFile[0];
        m_animFileDataBlob_len = m_animFile.size();
    }

    m_loaded = true;

    if (m_postLoadFunction != nullptr) {
        m_postLoadFunction();
    }
}

