//
// Created by deamon on 19.06.17.
//

#ifndef WOWVIEWERLIB_M2GEOM_H
#define WOWVIEWERLIB_M2GEOM_H

#include <vector>
#include "../persistance/M2File.h"

class M2Geom {
public:
    void initM2File(void *m2File, int fileLength);
    void loadTextures();
    void createVBO();
    void setupAttributes(void *skinObject);
private:
    std::vector<uint8_t> m2File;
    M2Data *m_m2Data;
};


#endif //WOWVIEWERLIB_M2GEOM_H
