//
// Created by deamon on 03.07.17.
//

#ifndef WOWVIEWERLIB_WMOMAINGEOM_H
#define WOWVIEWERLIB_WMOMAINGEOM_H

#include <vector>
#include "../persistance/helper/ChunkFileReader.h"
#include "../persistance/header/wmoFileHeader.h"
#include "../../gapi/ogl3.3/GDevice.h"

class WmoMainGeom {
public:
    void process(std::vector<unsigned char> &wmoMainFile, std::string &fileName);
    bool getIsLoaded();
private:
    static chunkDef<WmoMainGeom> wmoMainTable;
    bool m_loaded = false;
public:
    std::vector<uint8_t> m_wmoMainFile;

    SMOHeader *header;

    SMOGroupInfo *groups;
    int groupsLen;

    std::vector<std::vector<uint32_t>> gfids;

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

    SMOLight * lights = nullptr;
    int lightsLen = 0;

    SMOFog *fogs;
    int fogsLen;

};


#endif //WOWVIEWERLIB_WMOMAINGEOM_H
