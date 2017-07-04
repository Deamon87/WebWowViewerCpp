//
// Created by deamon on 03.07.17.
//

#ifndef WOWVIEWERLIB_WMOMAINGEOM_H
#define WOWVIEWERLIB_WMOMAINGEOM_H

#include <vector>
#include "../persistance/ChunkFileReader.h"

class WmoMainGeom {
public:
    void process(std::vector<unsigned char> &wmoMainFile);

    static chunkDef<WmoMainGeom> wmoMainTable;
};


#endif //WOWVIEWERLIB_WMOMAINGEOM_H
