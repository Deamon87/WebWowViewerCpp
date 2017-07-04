//
// Created by deamon on 03.07.17.
//

#include "wmoMainGeom.h"

chunkDef<WmoMainGeom> WmoMainGeom::wmoMainTable = {
        handler : [](WmoMainGeom& object, ChunkData& chunkData){},
        subChunks : {
                {
                    'MVER', {
                        handler: [](WmoMainGeom &object, ChunkData &chunkData) {

                        }
                    }
                },
                {
                    'MOHD', {
                        handler: [](WmoMainGeom &object, ChunkData &chunkData) {

                        }
                    }
                },
                {
                    'MOGI', {
                        handler: [](WmoMainGeom &object, ChunkData &chunkData) {

                        }
                    }
                },
                {
                    'MLIQ', {
                        handler: [](WmoMainGeom &object, ChunkData &chunkData) {

                        }
                    }
                },
                {
                    'MOPV', {
                        handler: [](WmoMainGeom &object, ChunkData &chunkData) {

                        }
                    }
                },
                {
                    'MOPT', {
                        handler: [](WmoMainGeom &object, ChunkData &chunkData) {

                        }
                    }
                },
                {
                    'MOPR', {
                        handler: [](WmoMainGeom &object, ChunkData &chunkData) {

                        }
                    }
                },
                {
                    'MOLT', {
                        handler: [](WmoMainGeom &object, ChunkData &chunkData) {

                        }
                    }
                },
                {
                    'MOMT', {
                        handler: [](WmoMainGeom &object, ChunkData &chunkData) {

                        }
                    }
                },
                {
                    'MOTX', {
                        handler: [](WmoMainGeom &object, ChunkData &chunkData) {

                        }
                    }
                },
                {
                    'MODN', {
                        handler: [](WmoMainGeom &object, ChunkData &chunkData) {

                        }
                    }
                },
                {
                    'MODS', {
                        handler: [](WmoMainGeom &object, ChunkData &chunkData) {

                        }
                    }
                },
                {
                    'MODD', {
                        handler: [](WmoMainGeom &object, ChunkData &chunkData) {

                        }
                    }
                },
                {
                    'MFOG', {
                        handler: [](WmoMainGeom &object, ChunkData &chunkData) {

                        }
                    }
                }
        }
};

void WmoMainGeom::process(std::vector<unsigned char> &wmoMainFile) {

}
