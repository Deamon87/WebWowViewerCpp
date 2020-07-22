//
// Created by deamon on 15.06.17.
//

#ifndef WOWVIEWERLIB_CONFIG_H
#define WOWVIEWERLIB_CONFIG_H

#include <string>
#include <mathfu/glsl_mappings.h>

class Config {

private:
    bool renderAdt = true;
    bool renderWMO = true;
    bool renderM2 = true;
    bool renderBSP = false;
    bool renderPortals = false;
    bool usePortalCulling = true;
    bool useInstancing = false;

    bool drawWmoBB = false;
    bool drawM2BB = true;
    bool secondCamera = false;
    bool doubleCameraDebug = false;

    bool BCLightHack = false;

    bool drawDepthBuffer = false;
    int cameraM2 = -1; // this will be sceneNumber of object

    int wmoMinBatch = 0;
    int wmoMaxBatch = 9999;

    int m2MinBatch = 0;
    int m2MaxBatch = 9999;

    int minParticle = 0;
    int maxParticle = 9999;

    int threadCount = 4;
    int quickSortCutoff = 100;

    int currentTime = 0;

    bool useWotlkLogic = false;
    float movementSpeed = 1.0;

    float nearPlane = 1;
    float farPlane = 1000;
    float farPlaneForCulling = 400;

    float fogStart;
    float fogEnd;

    bool useGaussBlur = false;

    mathfu::vec4 clearColor = {0.117647, 0.207843, 0.392157, 0};

    mathfu::vec4 exteriorAmbientColor = {1, 1, 1, 1};
    mathfu::vec4 exteriorHorizontAmbientColor = {1, 1, 1, 1};
    mathfu::vec4 exteriorGroundAmbientColor = {1, 1, 1, 1};
    mathfu::vec4 exteriorDirectColor = {0.3,0.3,0.3, 0.3};
    mathfu::vec3 exteriorDirectColorDir;

    mathfu::vec4 interiorAmbientColor;
    mathfu::vec4 interiorSunColor;
    mathfu::vec3 interiorSunDir;

    mathfu::vec4 closeRiverColor = {1,1,1,1};
    mathfu::vec4 fogColor;

    mathfu::vec4 SkyTopColor;
    mathfu::vec4 SkyMiddleColor;
    mathfu::vec4 SkyBand1Color;
    mathfu::vec4 SkyBand2Color;
    mathfu::vec4 SkySmogColor;
    mathfu::vec4 SkyFogColor;


    std::string areaName;
public:
    float getFarPlane() {
        return farPlane;
    }
    void setFarPlane(float value) {
        farPlane = value;
    }
    float getFarPlaneForCulling() {
        return farPlaneForCulling;
    }
    void setFarPlaneForCulling(float value) {
        farPlaneForCulling = value;
    }

    bool getRenderWMO () {
        return renderWMO;
    }

    void setRenderWMO(bool value) {
        renderWMO = value;
    }

    bool getRenderM2 () {
        return renderM2;
    }
    bool getUseInstancing () {
        return useInstancing;
    }
    void setRenderM2 (bool value) {
        renderM2 = value;
    }
    bool getRenderAdt() {
        return renderAdt;
    }
    void setRenderAdt(bool value) {
        renderAdt = value;
    }
    bool getRenderBSP() {
        return renderBSP;
    }
    void setRenderBSP (bool value) {
        renderBSP = value;
    }
    bool getRenderPortals() {
        return renderPortals;
    }
    void setRenderPortals(bool value) {
        renderPortals = value;
    }
    bool getUsePortalCulling () {
        return usePortalCulling;
    }
    void setUsePortalCulling(bool value) {
        usePortalCulling = value;
    }

    bool getDrawWmoBB (){
        return drawWmoBB;
    }
    void setDrawWmoBB (bool value) {
        drawWmoBB = value;
    }
    bool getDrawM2BB (){
        return drawM2BB;
    }
    void setDrawM2BB( bool value) {
        drawM2BB = value;
    }
    bool getUseSecondCamera () {
        return secondCamera;
    }
    void setUseSecondCamera (bool value) {
        secondCamera = value;
    }
    bool getDoubleCameraDebug () {
        return doubleCameraDebug;
    }
    void setDoubleCameraDebug (bool value) {
        doubleCameraDebug = value;
    }
    bool getDrawDepthBuffer () {
        return drawDepthBuffer;
    }
    void setDrawDepthBuffer (bool value) {
        drawDepthBuffer = value;
    }
    int getCameraM2 () {
        return cameraM2;
    }
    void setCameraM2 (bool value) {
        cameraM2 = value;
    }

    bool getBCLightHack () {
        return BCLightHack;
    }
    void setBCLightHack (bool value) {
        BCLightHack = value;
    }

    bool getUseWotlkLogic() {
         return useWotlkLogic;
    };

    void setWmoMinBatch(int value) {
        wmoMinBatch = value;
    }
    int getWmoMinBatch() {
        return wmoMinBatch;
    }
    void setWmoMaxBatch(int value) {
        wmoMaxBatch = value;
    }
    int getWmoMaxBatch() {
        return wmoMaxBatch;
    }

    void setM2MinBatch(int value) {
        m2MinBatch = value;
    }
    int getM2MinBatch() {
        return m2MinBatch;
    }
    void setM2MaxBatch(int value) {
        m2MaxBatch = value;
    }
    int getM2MaxBatch() {
        return m2MaxBatch;
    }

    void setMinParticle(int value) {
        minParticle = value;
    }
    int getMinParticle() {
        return minParticle;
    }
    void setMaxParticle(int value) {
        maxParticle = value;
    }
    int getMaxParticle() {
        return maxParticle;
    }

    void setMovementSpeed(float value) {
        movementSpeed = value;
    }
    float getMovementSpeed() {
        return movementSpeed;
    }

    void setClearColor(float r, float g, float b, float a) {
        clearColor[0] = r;
        clearColor[1] = g;
        clearColor[2] = b;
        clearColor[3] = a;
    }

    mathfu::vec4 getClearColor(){
        return clearColor;
    }
    void setExteriorAmbientColor(float r, float g, float b, float a) {
        exteriorAmbientColor[0] = r;
        exteriorAmbientColor[1] = g;
        exteriorAmbientColor[2] = b;
        exteriorAmbientColor[3] = a;
    }

    mathfu::vec4 getExteriorAmbientColor(){
        return exteriorAmbientColor;
    }

    mathfu::vec4 getExteriorHorizontAmbientColor(){
        return exteriorHorizontAmbientColor;
    }

    void setExteriorHorizontAmbientColor(float r, float g, float b, float a) {
        exteriorHorizontAmbientColor[0] = r;
        exteriorHorizontAmbientColor[1] = g;
        exteriorHorizontAmbientColor[2] = b;
        exteriorHorizontAmbientColor[3] = a;
    }
    mathfu::vec4 getExteriorGroundAmbientColor(){
        return exteriorGroundAmbientColor;
    }

    void setExteriorGroundAmbientColor(float r, float g, float b, float a) {
        exteriorGroundAmbientColor[0] = r;
        exteriorGroundAmbientColor[1] = g;
        exteriorGroundAmbientColor[2] = b;
        exteriorGroundAmbientColor[3] = a;
    }


    void setExteriorDirectColor(float r, float g, float b, float a) {
        exteriorDirectColor[0] = r;
        exteriorDirectColor[1] = g;
        exteriorDirectColor[2] = b;
        exteriorDirectColor[3] = a;
    }

    mathfu::vec4 getExteriorDirectColor(){
        return exteriorDirectColor;
    }

    void setExteriorDirectColorDir(float x, float y, float z) {
        exteriorDirectColorDir[0] = x;
        exteriorDirectColorDir[1] = y;
        exteriorDirectColorDir[2] = z;
    }

    mathfu::vec4 getSkyTopColor(){
        return SkyTopColor;
    }

    void setSkyTopColor(float x, float y, float z) {
        SkyTopColor[0] = x;
        SkyTopColor[1] = y;
        SkyTopColor[2] = z;
    }

    mathfu::vec4 getSkyBand1Color(){
        return SkyBand1Color;
    }

    void setSkyBand1Color(float x, float y, float z) {
        SkyBand1Color[0] = x;
        SkyBand1Color[1] = y;
        SkyBand1Color[2] = z;
    }

    mathfu::vec4 getSkyMiddleColor(){
        return SkyMiddleColor;
    }

    void setSkyMiddleColor(float x, float y, float z) {
        SkyMiddleColor[0] = x;
        SkyMiddleColor[1] = y;
        SkyMiddleColor[2] = z;
    }

    mathfu::vec4 getSkyBand2Color(){
        return SkyBand2Color;
    }

    void setSkyBand2Color(float x, float y, float z) {
        SkyBand2Color[0] = x;
        SkyBand2Color[1] = y;
        SkyBand2Color[2] = z;
    }

    mathfu::vec4 getSkySmogColor(){
        return SkySmogColor;
    }

    void setSkySmogColor(float x, float y, float z) {
        SkySmogColor[0] = x;
        SkySmogColor[1] = y;
        SkySmogColor[2] = z;
    }

    mathfu::vec4 getSkyFogColor(){
        return SkyFogColor;
    }

    void setSkyFogColor(float x, float y, float z) {
        SkyFogColor[0] = x;
        SkyFogColor[1] = y;
        SkyFogColor[2] = z;
    }

    mathfu::vec3 getExteriorDirectColorDir(){
        return exteriorDirectColorDir;
    }

    void setCloseRiverColor(float r, float g, float b, float a) {
        closeRiverColor[0] = r;
        closeRiverColor[1] = g;
        closeRiverColor[2] = b;
        closeRiverColor[3] = a;
    }

    mathfu::vec4 getCloseRiverColor(){
        return closeRiverColor;
    }

    void setFogStart(float value) {
        fogStart = value;
    }
    float getFogStart() {
        return fogStart;
    };

    void setFogEnd(float value) {
        fogEnd = value;
    }
    float getFogEnd() {
        return fogEnd;
    }

    void setFogColor(float r, float g, float b, float a) {
        fogColor[0] = r;
        fogColor[1] = g;
        fogColor[2] = b;
        fogColor[3] = a;
    }

    mathfu::vec4 getFogColor(){
        return fogColor;
    }

    void setAreaName(std::string a) {
        areaName = a;
    }

    std::string getAreaName() {
        return areaName;
    }

    int getThreadCount() {
        return threadCount;
    }
    void setThreadCount(int value) {
        threadCount = value;
    }

    int getQuickSortCutoff() {
        return quickSortCutoff;
    }
    void setQuickSortCutoff(int value) {
        quickSortCutoff = value;
    }

    void setCurrentTime(int value) {
        currentTime = value;
    }
    int getCurrentTime() {
        return currentTime;
    }

    void setUseGaussBlur(bool value) {
        useGaussBlur = value;
    }
    bool getUseGaussBlur() {
        return useGaussBlur;
    }
};


#endif //WOWVIEWERLIB_CONFIG_H
