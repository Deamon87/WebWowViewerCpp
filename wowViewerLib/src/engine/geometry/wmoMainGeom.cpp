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
                            debuglog("Entered MVER");
                        }
                    }
                },
                {
                    'MOHD', {
                        handler: [](WmoMainGeom &object, ChunkData &chunkData) {
                            chunkData.readValue(object.header);

                            debuglog("Entered MOHD");
                        }
                    }
                },
                {
                    'MOGI', {
                        handler: [](WmoMainGeom &object, ChunkData &chunkData) {
                            object.groupsLen = chunkData.chunkLen / sizeof(SMOGroupInfo);
                            chunkData.readValues(object.groups, object.groupsLen);
                            debuglog("Entered MOGI");
                        }
                    }
                },
                {
                    'MLIQ', {
                        handler: [](WmoMainGeom &object, ChunkData &chunkData) {
                            debuglog("Entered MLIQ");
                        }
                    }
                },
                {
                    'MOPV', {
                        handler: [](WmoMainGeom &object, ChunkData &chunkData) {
                            object.portal_verticesLen = chunkData.chunkLen / sizeof(C3Vector);
                            chunkData.readValues(object.portal_vertices, object.portal_verticesLen);

                            debuglog("Entered MOPV");
                        }
                    }
                },
                {
                    'MOPT', {
                        handler: [](WmoMainGeom &object, ChunkData &chunkData) {
                            object.portalsLen = chunkData.chunkLen / sizeof(SMOPortal);
                            chunkData.readValues(object.portals, object.portalsLen);
                            debuglog("Entered MOPT");
                        }
                    }
                },
                {
                    'MOPR', {
                        handler: [](WmoMainGeom &object, ChunkData &chunkData) {
                            object.portalReferencesLen = chunkData.chunkLen / sizeof(SMOPortalRef);
                            chunkData.readValues(object.portalReferences, object.portalReferencesLen);
                            debuglog("Entered MOPR");
                        }
                    }
                },
                {
                    'MOLT', {
                        handler: [](WmoMainGeom &object, ChunkData &chunkData) {
                            debuglog("Entered MOLT");
                        }
                    }
                },
                {
                    'MOMT', {
                        handler: [](WmoMainGeom &object, ChunkData &chunkData) {
                            object.materialsLen = chunkData.chunkLen / sizeof(SMOMaterial);
                            chunkData.readValues(object.materials, object.materialsLen);
                            debuglog("Entered MOMT");
                        }
                    }
                },
                {
                    'MOTX', {
                        handler: [](WmoMainGeom &object, ChunkData &chunkData) {
                            object.textureNamesFieldLen = chunkData.chunkLen;
                            chunkData.readValues(object.textureNamesField, object.textureNamesFieldLen);
                            debuglog("Entered MOTX");
                        }
                    }
                },
                {
                    'MODN', {
                        handler: [](WmoMainGeom &object, ChunkData &chunkData) {
                            object.doodadNamesFieldLen = chunkData.chunkLen;
                            chunkData.readValues(object.doodadNamesField, object.doodadNamesFieldLen);
                            debuglog("Entered MODN");
                        }
                    }
                },
                {
                    'MODS', {
                        handler: [](WmoMainGeom &object, ChunkData &chunkData) {
                            object.doodadSetsLen = chunkData.chunkLen / sizeof(SMODoodadSet);
                            chunkData.readValues(object.doodadSets, object.doodadSetsLen);
                            debuglog("Entered MODS");
                        }
                    }
                },
                {
                    'MODD', {
                        handler: [](WmoMainGeom &object, ChunkData &chunkData) {
                            object.doodadDefsLen = chunkData.chunkLen / sizeof(SMODoodadDef);
                            chunkData.readValues(object.doodadDefs, object.doodadDefsLen);
                            debuglog("Entered MODD");
                        }
                    }
                },
                {
                    'MFOG', {
                        handler: [](WmoMainGeom &object, ChunkData &chunkData) {
                            object.fogsLen = chunkData.chunkLen / sizeof(SMOFog);
                            chunkData.readValues(object.fogs, object.fogsLen);

                            debuglog("Entered MFOG");
                        }
                    }
                }
        }
};



void WmoMainGeom::process(std::vector<unsigned char> &wmoMainFile) {
    m_wmoMainFile = wmoMainFile;

    CChunkFileReader reader(m_wmoMainFile);
    reader.processFile(*this, &WmoMainGeom::wmoMainTable);

    m_loaded = true;
}

bool WmoMainGeom::getIsLoaded() {
    return m_loaded;
}
