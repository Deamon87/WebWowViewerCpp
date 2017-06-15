#ifndef WOWMAPVIEWERREVIVED_WOWSCENE_H_H
#define WOWMAPVIEWERREVIVED_WOWSCENE_H_H

#include <string>

class WoWScene {

public:
    virtual void draw(int deltaTime) = 0;
    virtual void provideFile(int requestId, char* fileName, unsigned char* data, int fileLength) = 0;
};

WoWScene * createWoWScene();

#endif //WOWMAPVIEWERREVIVED_WOWSCENE_H_H
