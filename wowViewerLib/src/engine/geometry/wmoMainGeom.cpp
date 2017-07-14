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
                            chunkData.readValue(object.header);

                            std::cout<<"Entered MOHD"<<std::endl;
                        }
                    }
                },
                {
                    'MOGI', {
                        handler: [](WmoMainGeom &object, ChunkData &chunkData) {
                            object.groupsLen = chunkData.chunkLen / sizeof(SMOGroupInfo);
                            chunkData.readValues(object.groups, object.groupsLen);
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
                            object.portal_verticesLen = chunkData.chunkLen / sizeof(C3Vector);
                            chunkData.readValues(object.portal_vertices, object.portal_verticesLen);

                            std::cout<<"Entered MOPV"<<std::endl;
                        }
                    }
                },
                {
                    'MOPT', {
                        handler: [](WmoMainGeom &object, ChunkData &chunkData) {
                            object.portalsLen = chunkData.chunkLen / sizeof(SMOPortal);
                            chunkData.readValues(object.portals, object.portalsLen);
                            std::cout<<"Entered MOPT"<<std::endl;
                        }
                    }
                },
                {
                    'MOPR', {
                        handler: [](WmoMainGeom &object, ChunkData &chunkData) {
                            object.portalReferencesLen = chunkData.chunkLen / sizeof(SMOPortalRef);
                            chunkData.readValues(object.portalReferences, object.portalReferencesLen);
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
                            object.materialsLen = chunkData.chunkLen / sizeof(SMOMaterial);
                            chunkData.readValues(object.materials, object.materialsLen);
                            std::cout<<"Entered MOMT"<<std::endl;
                        }
                    }
                },
                {
                    'MOTX', {
                        handler: [](WmoMainGeom &object, ChunkData &chunkData) {
                            object.textureNamesFieldLen = chunkData.chunkLen;
                            chunkData.readValues(object.textureNamesField, object.textureNamesFieldLen);
                            std::cout<<"Entered MOTX"<<std::endl;
                        }
                    }
                },
                {
                    'MODN', {
                        handler: [](WmoMainGeom &object, ChunkData &chunkData) {
                            object.doodadNamesFieldLen = chunkData.chunkLen;
                            chunkData.readValues(object.doodadNamesField, object.doodadNamesFieldLen);
                            std::cout<<"Entered MODN"<<std::endl;
                        }
                    }
                },
                {
                    'MODS', {
                        handler: [](WmoMainGeom &object, ChunkData &chunkData) {
                            object.doodadSetsLen = chunkData.chunkLen / sizeof(SMODoodadSet);
                            chunkData.readValues(object.doodadSets, object.doodadSetsLen);
                            std::cout<<"Entered MODS"<<std::endl;
                        }
                    }
                },
                {
                    'MODD', {
                        handler: [](WmoMainGeom &object, ChunkData &chunkData) {
                            object.doodadDefsLen = chunkData.chunkLen / sizeof(SMODoodadDef);
                            chunkData.readValues(object.doodadDefs, object.doodadDefsLen);
                            std::cout<<"Entered MODD"<<std::endl;
                        }
                    }
                },
                {
                    'MFOG', {
                        handler: [](WmoMainGeom &object, ChunkData &chunkData) {
                            object.fogsLen = chunkData.chunkLen / sizeof(SMOFog);
                            chunkData.readValues(object.fogs, object.fogsLen);

                            std::cout<<"Entered MFOG"<<std::endl;
                        }
                    }
                }
        }
};



void WmoMainGeom::process(std::vector<unsigned char> &wmoMainFile) {
    CChunkFileReader<WmoMainGeom> reader(wmoMainFile, &WmoMainGeom::wmoMainTable);
    reader.processFile(*this);

    m_loaded = true;
}

bool WmoMainGeom::getIsLoaded() {
    return m_loaded;
}
