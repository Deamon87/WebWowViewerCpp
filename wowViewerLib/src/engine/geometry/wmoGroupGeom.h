//
// Created by deamon on 03.07.17.
//

#ifndef WOWVIEWERLIB_WMOGROUPGEOM_H
#define WOWVIEWERLIB_WMOGROUPGEOM_H


#include <vector>
#include "../persistance/ChunkFileReader.h"

class WmoGroupGeom {
public:
    void process(std::vector<unsigned char> &wmoGroupFile);
};

chunkDef<WmoGroupGeom> a = {
        subChunks: {
                {'MOPY',
                    [](WmoGroupGeom b, chunkData_t &c)->{

                    }
                }
        }
};


#endif //WOWVIEWERLIB_WMOGROUPGEOM_H
