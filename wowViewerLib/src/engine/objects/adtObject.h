//
// Created by deamon on 10.07.17.
//

#ifndef WEBWOWVIEWERCPP_ADTOBJECT_H
#define WEBWOWVIEWERCPP_ADTOBJECT_H


#include <vector>
#include "../persistance/ChunkFileReader.h"

class AdtObject {
public:
    AdtObject(){}
    void process(std::vector<unsigned char> &adtFile);
private:
    static chunkDef<AdtObject> adtObjectTable;
};


#endif //WEBWOWVIEWERCPP_ADTOBJECT_H
