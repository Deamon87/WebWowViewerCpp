#ifndef WOWMAPVIEWERREVIVED_WOWSCENE_H_H
#define WOWMAPVIEWERREVIVED_WOWSCENE_H_H

#include <string>
#include "config.h"

class WoWScene {

public:
    virtual void draw(int deltaTime) = 0;
    virtual void provideFile(int requestId, char* fileName, unsigned char* data, int fileLength) = 0;
};

WoWScene * createWoWScene(Config *config);

#endif //WOWMAPVIEWERREVIVED_WOWSCENE_H_H
