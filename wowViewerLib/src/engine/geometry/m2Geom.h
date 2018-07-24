//
// Created by deamon on 19.06.17.
//

#ifndef WOWVIEWERLIB_M2GEOM_H
#define WOWVIEWERLIB_M2GEOM_H

#include <vector>
#include "../wowInnerApi.h"
#include "../persistance/header/M2FileHeader.h"
#include "../objects/m2/m2Helpers/M2MaterialInst.h"
#include "../../gapi/GDevice.h"
//#include "../objects/m2Object.h"


class M2Geom {
public:
    void process(std::vector<unsigned char> &m2File, std::string &fileName);
    HGVertexBuffer getVBO(GDevice &device);

    bool isLoaded() { return m_loaded; };

    M2Data * getM2Data(){ if (m_loaded) {return m_m2Data;} else {return nullptr;}};

    M2Data *m_m2Data;
    std::vector<uint32_t> skinFileDataIDs;
    std::vector<uint32_t> textureFileDataIDs;
private:
    std::vector<uint8_t> m2File;

    bool m_loaded = false;
    HGVertexBuffer vertexVbo = HGVertexBuffer(nullptr);

    static chunkDef<M2Geom> m2FileTable;

};
typedef std::shared_ptr<M2Geom> HM2Geom;

#endif //WOWVIEWERLIB_M2GEOM_H
