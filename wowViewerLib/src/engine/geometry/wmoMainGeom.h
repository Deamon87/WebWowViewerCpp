//
// Created by deamon on 03.07.17.
//

#ifndef WOWVIEWERLIB_WMOMAINGEOM_H
#define WOWVIEWERLIB_WMOMAINGEOM_H

class WmoMainGeom;

#include <vector>
#include "../persistance/PersistentFile.h"
#include "../persistance/helper/ChunkFileReader.h"
#include "../persistance/header/wmoFileHeader.h"
#include "../../gapi/interface/IDevice.h"

class WmoMainGeom : public PersistentFile {
public:
    WmoMainGeom(std::string fileName){};
    WmoMainGeom(int fileDataId){};

    void process(HFileContent wmoMainFile, const std::string &fileName) override;
private:
    static chunkDef<WmoMainGeom> wmoMainTable;
public:
    HFileContent m_wmoMainFile;

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

    PointerChecker<char> skyBoxM2FileName = (skyBoxM2FileNameLen);
    int skyBoxM2FileNameLen = 0;

    PointerChecker<MAVG> mavgs = (mavgsLen);
    int mavgsLen = 0;

    int skyboxM2FileId = 0;
};


#endif //WOWVIEWERLIB_WMOMAINGEOM_H
