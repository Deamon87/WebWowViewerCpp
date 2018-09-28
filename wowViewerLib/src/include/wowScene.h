#ifndef WOWMAPVIEWERREVIVED_WOWSCENE_H_H
#define WOWMAPVIEWERREVIVED_WOWSCENE_H_H

#include <cstdint>
#ifdef WITH_GLESv2
typedef float animTime_t;
#else
typedef double animTime_t;
#endif

#include <string>
#include "config.h"
#include "controllable.h"

class IFileRequest {
public:
    virtual void requestFile(const char* fileName) = 0;
};

class IFileRequester {
public:
    virtual void setFileRequestProcessor(IFileRequest * requestProcessor) = 0;
    virtual void provideFile(const char* fileName, unsigned char* data, int fileLength) = 0;
    virtual void rejectFile(const char* fileName) = 0;
};

class WoWScene : public IFileRequester {

public:
    virtual ~WoWScene(){};
    virtual void draw(animTime_t deltaTime) = 0;
    virtual void setScreenSize(int canvWidth, int canvHeight) = 0;
    virtual void switchCameras() = 0;

    virtual void setScene(int sceneType, std::string fileName, int cameraNum) = 0;
    virtual void setSceneWithFileDataId(int sceneType, int fileDataId, int cameraNum) = 0;

    virtual IControllable* getCurrentCamera() = 0;
};


extern "C" { WoWScene * createWoWScene(Config *config, IFileRequest * requestProcessor, int canvWidth, int canvHeight); };

#endif //WOWMAPVIEWERREVIVED_WOWSCENE_H_H
