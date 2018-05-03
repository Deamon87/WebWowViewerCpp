//
// Created by deamon on 03.05.18.
//

#include "texFile.h"

chunkDef<TexFile> TexFile::texFileTable = {
        handler : [](TexFile &file, ChunkData &chunkData) {},
        subChunks : {
                {
                        'TXVR',
                        {
                                handler: [](TexFile &file, ChunkData &chunkData) {
                                    debuglog("Entered TXVR");
                                    uint32_t version;
                                    chunkData.readValue(version);
                                }
                        }
                },
                {
                        'TXBT',
                        {
                                handler: [](TexFile &file, ChunkData &chunkData) {
                                    debuglog("Entered TXBT");
                                    file.entriesNum = chunkData.chunkLen / sizeof(SBlobTexture);
                                    chunkData.readValues(file.entries, file.entriesNum);
                                }
                        },
                },
                {
                        'TXFN',
                        {
                                handler: [](TexFile &file, ChunkData &chunkData) {
                                    debuglog("Entered TXFN");
                                    file.filenames_len = chunkData.chunkLen;
                                    chunkData.readValues(file.filenames, chunkData.chunkLen);
                                },
                        }
                },
                {
                        'TXMD',
                        {
                                handler: [](TexFile &file, ChunkData &chunkData) {
                                    debuglog("Entered TXMD");
                                    file.textureData_len = chunkData.chunkLen;
                                    chunkData.readValues(file.textureData, file.textureData_len);
                                }
                        },
                }
        }
};