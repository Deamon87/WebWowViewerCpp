#ifndef WOWMAPVIEWERREVIVED_WOWSCENE_H_H
#define WOWMAPVIEWERREVIVED_WOWSCENE_H_H

#include <cstdint>
#include <vector>
#ifdef WITH_GLESv2
typedef float animTime_t;
#else
typedef double animTime_t;
#endif

#include <string>
#include "config.h"
#include "controllable.h"
#include "sharedFile.h"


enum class CacheHolderType {
    CACHE_M2,
    CACHE_SKIN,
    CACHE_ANIM,
    CACHE_BONE,
    CACHE_SKEL,
    CACHE_MAIN_WMO,
    CACHE_GROUP_WMO,
    CACHE_ADT,
    CACHE_WDT,
    CACHE_WDL,
    CACHE_BLP,
    CACHE_DB2,
};

class IFileRequest {
public:
    virtual void requestFile(const char* fileName, CacheHolderType holderType) = 0;
};

class IFileRequester {
public:
    virtual void setFileRequestProcessor(IFileRequest * requestProcessor) = 0;
    virtual void provideFile(CacheHolderType holderType, const char* fileName, HFileContent data) = 0;
    virtual void rejectFile(CacheHolderType holderType, const char* fileName) = 0;
};

class WoWScene : public IFileRequester {

public:
    virtual ~WoWScene() = default;
    virtual void draw(animTime_t deltaTime) = 0;
    virtual void setScreenSize(int canvWidth, int canvHeight) = 0;
    virtual void switchCameras() = 0;

    virtual void clearCache() = 0;

    virtual void setAnimationId(int animationId) = 0;
    virtual void setReplaceTextureArray(std::vector<int> &replaceTextureArray) = 0;
    virtual void setScene(int sceneType, std::string fileName, int cameraNum) = 0;
    virtual void setSceneWithFileDataId(int sceneType, int fileDataId, int cameraNum) = 0;
    virtual void setScenePos(float x, float y, float z) = 0;

    virtual IControllable* getCurrentCamera() = 0;
};



class IDevice;
extern "C" { WoWScene * createWoWScene(Config *config, IFileRequest * requestProcessor, IDevice * device, int canvWidth, int canvHeight); };

#endif //WOWMAPVIEWERREVIVED_WOWSCENE_H_H
