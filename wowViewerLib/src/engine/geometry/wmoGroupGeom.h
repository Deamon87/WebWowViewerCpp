//
// Created by deamon on 03.07.17.
//

#ifndef WOWVIEWERLIB_WMOGROUPGEOM_H
#define WOWVIEWERLIB_WMOGROUPGEOM_H


#include <vector>
#include "../persistance/ChunkFileReader.h"
//#include "../persistance/wmoFile.h"

class WmoGroupGeom {
public:
    void process(std::vector<unsigned char> &wmoGroupFile);

    static chunkDef<WmoGroupGeom> wmoGroupTable;
private:
//    MOGP *mogp;
};




#endif //WOWVIEWERLIB_WMOGROUPGEOM_H
