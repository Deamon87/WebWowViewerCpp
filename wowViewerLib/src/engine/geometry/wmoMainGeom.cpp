//
// Created by deamon on 03.07.17.
//

#include "wmoMainGeom.h"
#include <iostream>

chunkDef<WmoMainGeom> WmoMainGeom::wmoMainTable = {
        handler : [](WmoMainGeom& object, ChunkData& chunkData){},
        subChunks : {
                {
                    'MVER', {
                        handler: [](WmoMainGeom &object, ChunkData &chunkData) {
                            std::cout<<"Entered MVER"<<std::endl;
                        }
                    }
                },
                {
                    'MOHD', {
                        handler: [](WmoMainGeom &object, ChunkData &chunkData) {
                            std::cout<<"Entered MOHD"<<std::endl;
                        }
                    }
                },
                {
                    'MOGI', {
                        handler: [](WmoMainGeom &object, ChunkData &chunkData) {
                            std::cout<<"Entered MOGI"<<std::endl;
                        }
                    }
                },
                {
                    'MLIQ', {
                        handler: [](WmoMainGeom &object, ChunkData &chunkData) {
                            std::cout<<"Entered MLIQ"<<std::endl;
                        }
                    }
                },
                {
                    'MOPV', {
                        handler: [](WmoMainGeom &object, ChunkData &chunkData) {
                            std::cout<<"Entered MOPV"<<std::endl;
                        }
                    }
                },
                {
                    'MOPT', {
                        handler: [](WmoMainGeom &object, ChunkData &chunkData) {
                            std::cout<<"Entered MOPT"<<std::endl;
                        }
                    }
                },
                {
                    'MOPR', {
                        handler: [](WmoMainGeom &object, ChunkData &chunkData) {
                            std::cout<<"Entered MOPR"<<std::endl;
                        }
                    }
                },
                {
                    'MOLT', {
                        handler: [](WmoMainGeom &object, ChunkData &chunkData) {
                            std::cout<<"Entered MOLT"<<std::endl;
                        }
                    }
                },
                {
                    'MOMT', {
                        handler: [](WmoMainGeom &object, ChunkData &chunkData) {
                            std::cout<<"Entered MOMT"<<std::endl;
                        }
                    }
                },
                {
                    'MOTX', {
                        handler: [](WmoMainGeom &object, ChunkData &chunkData) {
                            std::cout<<"Entered MOTX"<<std::endl;
                        }
                    }
                },
                {
                    'MODN', {
                        handler: [](WmoMainGeom &object, ChunkData &chunkData) {
                            std::cout<<"Entered MODN"<<std::endl;
                        }
                    }
                },
                {
                    'MODS', {
                        handler: [](WmoMainGeom &object, ChunkData &chunkData) {
                            std::cout<<"Entered MODS"<<std::endl;
                        }
                    }
                },
                {
                    'MODD', {
                        handler: [](WmoMainGeom &object, ChunkData &chunkData) {
                            std::cout<<"Entered MODD"<<std::endl;
                        }
                    }
                },
                {
                    'MFOG', {
                        handler: [](WmoMainGeom &object, ChunkData &chunkData) {
                            std::cout<<"Entered MFOG"<<std::endl;
                        }
                    }
                }
        }
};

void WmoMainGeom::process(std::vector<unsigned char> &wmoMainFile) {
    CChunkFileReader<WmoMainGeom> reader(wmoMainFile, &WmoMainGeom::wmoMainTable);
    reader.processFile(*this);
}
