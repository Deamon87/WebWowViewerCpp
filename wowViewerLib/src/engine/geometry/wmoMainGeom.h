//
// Created by deamon on 03.07.17.
//

#ifndef WOWVIEWERLIB_WMOMAINGEOM_H
#define WOWVIEWERLIB_WMOMAINGEOM_H

#include <vector>
#include "../persistance/ChunkFileReader.h"
#include "../persistance/wmoFile.h"

class WmoMainGeom {
public:
    void process(std::vector<unsigned char> &wmoMainFile);
    bool getIsLoaded();

private:
    static chunkDef<WmoMainGeom> wmoMainTable;
    bool m_loaded = false;
public:
    std::vector<uint8_t> m_wmoMainFile;

    SMOHeader *header;

    SMOGroupInfo *groups;
    int groupsLen;

    C3Vector *portal_vertices;
    int portal_verticesLen;

    SMOPortal *portals;
    int portalsLen;

    SMOPortalRef *portalReferences;
    int portalReferencesLen;

    SMOMaterial *materials;
    int materialsLen;

    char *textureNamesField;
    int textureNamesFieldLen;

    char *doodadNamesField;
    int doodadNamesFieldLen;

    SMODoodadSet *doodadSets;
    int doodadSetsLen;

    SMODoodadDef *doodadDefs;
    int doodadDefsLen;

    SMOFog *fogs;
    int fogsLen;
};


#endif //WOWVIEWERLIB_WMOMAINGEOM_H
