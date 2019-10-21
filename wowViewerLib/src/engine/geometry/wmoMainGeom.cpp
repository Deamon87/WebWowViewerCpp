//
// Created by deamon on 03.07.17.
//

#include "wmoMainGeom.h"
#include "../../include/wowScene.h"
#include <iostream>

chunkDef<WmoMainGeom> WmoMainGeom::wmoMainTable = {
        [](WmoMainGeom& object, ChunkData& chunkData){},
        {
                {
                    'MVER', {
                        [](WmoMainGeom &object, ChunkData &chunkData) {
                            debuglog("Entered MVER");
                            int version;
                            chunkData.readValue(version);
                        }
                    }
                },
                {
                    'MOHD', {
                        [](WmoMainGeom &object, ChunkData &chunkData) {
                            debuglog("Entered MOHD");

                            chunkData.readValue(object.header);
                        }
                    }
                },
                {
                    'GFID', {
                        [](WmoMainGeom &object, ChunkData &chunkData) {
                            debuglog("Entered GFID");

                            int fileDataIdCount = chunkData.chunkLen / sizeof(uint32_t);
                            int lodCount = fileDataIdCount / object.header->nGroups;
                            object.gfids = std::vector<std::vector<uint32_t>>(lodCount);
                            for (int i = 0; i < lodCount; i++) {
                                object.gfids[i] = std::vector<uint32_t>(object.header->nGroups);
                                for (int j = 0; j < object.header->nGroups; j++) {
                                    uint32_t fileDataId;
                                    chunkData.readValue(fileDataId);
                                    object.gfids[i][j] = fileDataId;
                                }
                            }
                        }
                    }
                },

                {
                    'MOGI', {
                        [](WmoMainGeom &object, ChunkData &chunkData) {
                            debuglog("Entered MOGI");
                            object.groupsLen = chunkData.chunkLen / sizeof(SMOGroupInfo);
                            chunkData.readValues(object.groups, object.groupsLen);
                        }
                    }
                },
                {
                    'MODI', {
                        [](WmoMainGeom &object, ChunkData &chunkData) {
                            debuglog("Entered MODI");
                            object.doodadFileDataIdsLen = chunkData.chunkLen / sizeof(int);
                            chunkData.readValues(object.doodadFileDataIds, object.doodadFileDataIdsLen);
                        }
                    }
                },
                {
                    'MLIQ', {
                        [](WmoMainGeom &object, ChunkData &chunkData) {
                            debuglog("Entered MLIQ");
                        }
                    }
                },
                {
                    'MOPV', {
                        [](WmoMainGeom &object, ChunkData &chunkData) {
                            debuglog("Entered MOPV");
                            object.portal_verticesLen = chunkData.chunkLen / sizeof(C3Vector);
                            chunkData.readValues(object.portal_vertices, object.portal_verticesLen);
                        }
                    }
                },
                {
                    'MOPT', {
                        [](WmoMainGeom &object, ChunkData &chunkData) {
                            debuglog("Entered MOPT");
                            object.portalsLen = chunkData.chunkLen / sizeof(SMOPortal);
                            chunkData.readValues(object.portals, object.portalsLen);
                        }
                    }
                },
                {
                    'MOPR', {
                        [](WmoMainGeom &object, ChunkData &chunkData) {
                            object.portalReferencesLen = chunkData.chunkLen / sizeof(SMOPortalRef);
                            chunkData.readValues(object.portalReferences, object.portalReferencesLen);
                            debuglog("Entered MOPR");
                        }
                    }
                },
                {
                    'MOMT', {
                        [](WmoMainGeom &object, ChunkData &chunkData) {
                            debuglog("Entered MOMT");
                            object.materialsLen = chunkData.chunkLen / sizeof(SMOMaterial);
                            chunkData.readValues(object.materials, object.materialsLen);
                        }
                    }
                },
                {
                    'MOTX', {
                        [](WmoMainGeom &object, ChunkData &chunkData) {
                            debuglog("Entered MOTX");
                            object.textureNamesFieldLen = chunkData.chunkLen;
                            chunkData.readValues(object.textureNamesField, object.textureNamesFieldLen);
                        }
                    }
                },
                {
                    'MODN', {
                        [](WmoMainGeom &object, ChunkData &chunkData) {
                            debuglog("Entered MODN");
                            object.doodadNamesFieldLen = chunkData.chunkLen;
                            chunkData.readValues(object.doodadNamesField, object.doodadNamesFieldLen);
                        }
                    }
                },
                {
                    'MODS', {
                        [](WmoMainGeom &object, ChunkData &chunkData) {
                            debuglog("Entered MODS");
                            object.doodadSetsLen = chunkData.chunkLen / sizeof(SMODoodadSet);
                            chunkData.readValues(object.doodadSets, object.doodadSetsLen);
                        }
                    }
                },
                {
                    'MODD', {
                        [](WmoMainGeom &object, ChunkData &chunkData) {
                            debuglog("Entered MODD");
                            object.doodadDefsLen = chunkData.chunkLen / sizeof(SMODoodadDef);
                            chunkData.readValues(object.doodadDefs, object.doodadDefsLen);
                        }
                    }
                },
                {
                    'MFOG', {
                        [](WmoMainGeom &object, ChunkData &chunkData) {
                            debuglog("Entered MFOG");
                            object.fogsLen = chunkData.chunkLen / sizeof(SMOFog);
                            chunkData.readValues(object.fogs, object.fogsLen);
                        }
                    }
                },
                {
                    'MOLT', {
                        [](WmoMainGeom& object, ChunkData& chunkData) {
                            debuglog("Entered MOLT");
                            object.lightsLen = chunkData.chunkLen / sizeof(SMOLight);
                            chunkData.readValues(object.lights, object.lightsLen);
                        }
                    }
                },
                {
                    'MOSB', {
                        [](WmoMainGeom &object, ChunkData &chunkData) {
                            debuglog("Entered MOSB");
                            object.skyBoxM2FileNameLen = chunkData.chunkLen;
                            chunkData.readValues(object.skyBoxM2FileName, object.skyBoxM2FileNameLen);
                        }
                    }

                },
                {
                    'MOSI', {
                        [](WmoMainGeom &object, ChunkData &chunkData) {
                            debuglog("Entered MOSI");
                            chunkData.readValue(object.skyboxM2FileId);
                        }
                    }

                },

        }
};



void WmoMainGeom::process(HFileContent wmoMainFile, const std::string &fileName) {
    m_wmoMainFile = wmoMainFile;

    CChunkFileReader reader(*m_wmoMainFile.get());
    reader.processFile(*this, &WmoMainGeom::wmoMainTable);

    m_loaded = true;
}

bool WmoMainGeom::getIsLoaded() {
    return m_loaded;
}


