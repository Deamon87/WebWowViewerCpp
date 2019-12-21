#ifndef WOWMAPVIEWERREVIVED_WOWSCENE_H_H
#define WOWMAPVIEWERREVIVED_WOWSCENE_H_H


class IDevice;
class WoWFilesCacheStorage;

#include <string>
#include "config.h"
#include "controllable.h"
#include "sharedFile.h"
#include "iostuff.h"




class WoWScene {

public:
    virtual ~WoWScene() = default;
    virtual void draw(animTime_t deltaTime) = 0;
    virtual void setScreenSize(int canvWidth, int canvHeight) = 0;
    virtual void switchCameras() = 0;

    virtual void clearCache() = 0;

    virtual void setCacheStorage(WoWFilesCacheStorage * cacheStorage) = 0;
    virtual void setAnimationId(int animationId) = 0;
    virtual void setReplaceTextureArray(std::vector<int> &replaceTextureArray) = 0;
    virtual void setScene(int sceneType, std::string fileName, int cameraNum) = 0;
    virtual void setSceneWithFileDataId(int sceneType, int fileDataId, int cameraNum) = 0;
    virtual void setScenePos(float x, float y, float z) = 0;
    virtual void setMap(int mapId, int wdtFileId, float x, float y, float z) = 0;

    virtual IControllable* getCurrentCamera() = 0;
};



#include "../engine/WowFilesCacheStorage.h"
extern "C" { WoWScene * createWoWScene(Config *config, WoWFilesCacheStorage * cacheStorage, IDevice * device, int canvWidth, int canvHeight); };

#endif //WOWMAPVIEWERREVIVED_WOWSCENE_H_H
