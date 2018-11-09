//
// Created by deamon on 03.07.17.
//

#ifndef WOWVIEWERLIB_WMOMAINGEOM_H
#define WOWVIEWERLIB_WMOMAINGEOM_H

#include <vector>
#include "../persistance/helper/ChunkFileReader.h"
#include "../persistance/header/wmoFileHeader.h"
#include "../../gapi/interface/IDevice.h"

class WmoMainGeom {
public:
    void process(const std::vector<unsigned char> &wmoMainFile, const std::string &fileName);
    bool getIsLoaded();
private:
    static chunkDef<WmoMainGeom> wmoMainTable;
    bool m_loaded = false;
public:
    std::vector<uint8_t> m_wmoMainFile;

    SMOHeader *header;

    PointerChecker<SMOGroupInfo> groups = (groupsLen);
    int groupsLen;

    std::vector<std::vector<uint32_t>> gfids;

    PointerChecker<C3Vector> portal_vertices = (portal_verticesLen);
    int portal_verticesLen;

    PointerChecker<SMOPortal> portals = (portalsLen);
    int portalsLen;

    PointerChecker<SMOPortalRef> portalReferences = (portalReferencesLen);
    int portalReferencesLen;

    PointerChecker<SMOMaterial> materials = (materialsLen);
    int materialsLen;

    PointerChecker<char> textureNamesField = (textureNamesFieldLen);
    int textureNamesFieldLen = 0;

    PointerChecker<char> doodadNamesField = (doodadNamesFieldLen);
    int doodadNamesFieldLen;

    PointerChecker<int> doodadFileDataIds = (doodadFileDataIdsLen);
    int doodadFileDataIdsLen = 0;

    PointerChecker<SMODoodadSet> doodadSets = (doodadSetsLen);
    int doodadSetsLen;

    PointerChecker<SMODoodadDef> doodadDefs = (doodadDefsLen);
    int doodadDefsLen;

    PointerChecker<SMOLight> lights = (lightsLen);
    int lightsLen = 0;

    PointerChecker<SMOFog> fogs = (fogsLen);
    int fogsLen;

};


#endif //WOWVIEWERLIB_WMOMAINGEOM_H
