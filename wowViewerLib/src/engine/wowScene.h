#ifndef WOWMAPVIEWERREVIVED_WOWSCENEIMPL_H_H
#define WOWMAPVIEWERREVIVED_WOWSCENEIMPL_H_H

#include <string>
#include <functional>
#include <algorithm>
#include <cctype>
#include <locale>
#include <mutex>
#include <future>
#include "wowInnerApi.h"

#include "../include/wowScene.h"
#include "../include/config.h"
#include "mathfu/glsl_mappings.h"
#include "camera/firstPersonOrthoCamera.h"
#include "camera/firstPersonCamera.h"
#include "texture/BlpTexture.h"
#include "geometry/skinGeom.h"
#include "geometry/m2Geom.h"
#include "cache/cache.h"
#include "stringTrim.h"
#include "geometry/wmoGroupGeom.h"
#include "geometry/wmoMainGeom.h"
#include "objects/scenes/map.h"
#include "persistance/wdtFile.h"
#include "persistance/wdlFile.h"
#include "../gapi/interface/IDevice.h"
#include "objects/wowFrameData.h"
#include "camera/planarCamera.h"
#include "persistance/animFile.h"
#include "persistance/skelFile.h"
#include "objects/scenes/NullScene.h"
#include "WowFilesCacheStorage.h"

class WoWSceneImpl: public WoWScene, public IWoWInnerApi {
private:
    WoWFilesCacheStorage * cacheStorage;
public:
    WoWSceneImpl(Config *config, WoWFilesCacheStorage * cacheStorage, IDevice * device, int canvWidth, int canvHeight);
    ~WoWSceneImpl() override;

    void draw(animTime_t deltaTime) override;
    void setScreenSize(int canvWidth, int canvHeight) override;

    IControllable* controllable = &m_firstCamera;

    IControllable* getCurrentCamera() override {
        return this->controllable;
    };

    void switchCameras() override {
        if (controllable == &m_firstCamera) {
            this->m_config->setUseSecondCamera(true);
            controllable = &m_secondCamera;
            mathfu::vec3 camPos = m_firstCamera.getCameraPosition();
            m_secondCamera.setCameraPos(camPos.x, camPos.y, camPos.z);
        } else {
            this->m_config->setUseSecondCamera(false);
            controllable = &m_firstCamera;
        }
    }
public:
    virtual IDevice * getDevice() override {
        return m_gdevice.get();
    }

    virtual HGUniformBufferChunk getSceneWideUniformBuffer() override {
        return m_sceneWideUniformBuffer;
    }

    virtual Cache<AdtFile> *getAdtGeomCache() override {
        return cacheStorage->getAdtGeomCache();
    }
    virtual Cache<M2Geom> *getM2GeomCache() override {
        return cacheStorage->getM2GeomCache();
    };
    virtual Cache<SkinGeom>* getSkinGeomCache() override {
        return cacheStorage->getSkinGeomCache();
    };
    virtual Cache<AnimFile>* getAnimCache() override {
        return cacheStorage->getAnimCache();
    };
    virtual Cache<SkelFile>* getSkelCache() override {
        return cacheStorage->getSkelCache();
    };
    virtual Cache<BlpTexture> *getTextureCache() override {
        return cacheStorage->getTextureCache();
    };
    virtual Cache<WmoMainGeom>* getWmoMainCache() override {
        return cacheStorage->getWmoMainCache();
    };
    virtual Cache<WmoGroupGeom>* getWmoGroupGeomCache() override {
        return cacheStorage->getWmoGroupGeomCache();
    };
    virtual Cache<WdtFile>* getWdtFileCache() override {
        return cacheStorage->getWdtFileCache();
    };

    virtual Cache<WdlFile>* getWdlFileCache() override {
        return cacheStorage->getWdlFileCache();
    };

    virtual mathfu::mat4& getViewMat() override {
        return m_FrameParams[updateFrameIndex].m_lookAtMat4;
    }
    virtual mathfu::vec4 getGlobalAmbientColor() override {
        return m_FrameParams[updateFrameIndex].m_globalAmbientColor ;
    }
    virtual mathfu::vec3 getGlobalSunDir() override {
        return m_FrameParams[updateFrameIndex].m_sunDir;
    };
    virtual mathfu::vec4 getGlobalSunColor() override {
        return m_FrameParams[updateFrameIndex].m_globalSunColor;
    }
    virtual mathfu::vec4 getGlobalFogColor() override {
        return m_FrameParams[updateFrameIndex].m_fogColor;
    }
    virtual float getGlobalFogStart() override { return m_FrameParams[updateFrameIndex].uFogStart; };
    virtual float getGlobalFogEnd() override { return m_FrameParams[updateFrameIndex].uFogEnd; };
    virtual mathfu::vec3 getViewUp() override { return m_FrameParams[updateFrameIndex].m_upVector; };

    int updateFrameIndex = 0;

    virtual Config *getConfig() override {
        return m_config;
    }
    void setScenePos(float x, float y, float z) override {
        m_firstCamera.setCameraPos(x,y,z);
    };
    void setCameraPosition(float x, float y, float z) override {
        m_planarCamera.setCameraPos(x,y,z);
    }
    void setCameraOffset(float x, float y, float z) override {
        m_planarCamera.setCameraOffset(x,y,z);
    };
    void clearCache() override;

    void setAnimationId(int animationId) override;
    void setReplaceTextureArray(std::vector<int> &replaceTextureArray) override;

    void setScene(int sceneType, std::string fileName, int cameraNum) override;
    void setSceneWithFileDataId(int sceneType, int fileDataId, int cameraNum) override;
    void setMap(int mapId, int wdtFileId, float x, float y, float z) override;
private:
    void DoCulling();
    void DoUpdate();
private:
    bool m_enable;

    bool m_isTerminating = false;
    Config * m_config;
    std::unique_ptr<IDevice> m_gdevice;
    HGUniformBufferChunk m_sceneWideUniformBuffer;

    WoWFrameData m_FrameParams[4];

    bool m_usePlanarCamera = false;

    FirstPersonCamera m_firstCamera;
    PlanarCamera m_planarCamera;
    FirstPersonCamera m_secondCamera;

    int canvWidth;
    int canvHeight;
    float canvAspect;


    bool m_isDebugCamera = false;


    iInnerSceneApi *currentScene = new NullScene();
    iInnerSceneApi *newScene = nullptr;

    bool needToDropCache = false;
    std::promise<float> nextDeltaTime = std::promise<float>();
    std::promise<float> nextDeltaTimeForUpdate;
    std::promise<bool> cullingFinished;
    std::promise<bool> updateFinished;

//    void drawTexturedQuad(GLuint texture, float x, float y, float width, float height, float canv_width, float canv_height,
//                          bool drawDepth);

    void drawCamera() override ;
    bool getIsDebugCamera() override {
        return m_isDebugCamera;
    }

    bool m_supportThreads = true;

    void SetDirection(WoWFrameData &frameParamHolder);

    void processCaches(int limit);
};



#endif //WOWMAPVIEWERREVIVED_WOWSCENEIMPL_H_H
