//
// Created by deamon on 15.06.17.
//

#ifndef WOWVIEWERLIB_CONFIG_H
#define WOWVIEWERLIB_CONFIG_H

#include <string>

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

    float clearColor[4] = {0.117647, 0.207843, 0.392157, 0};
    float ambientColor[4];
    float sunColor[4];
    float closeRiverColor[4] = {1,1,1,1};
    float fogColor[4];

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

    void getClearColor(float *clearC){
        clearC[0] = clearColor[0];
        clearC[1] = clearColor[1];
        clearC[2] = clearColor[2];
        clearC[3] = clearColor[3];
    }
    void setAmbientColor(float r, float g, float b, float a) {
        ambientColor[0] = r;
        ambientColor[1] = g;
        ambientColor[2] = b;
        ambientColor[3] = a;
    }

    void getAmbientColor(float *ambient){
        ambient[0] = ambientColor[0];
        ambient[1] = ambientColor[1];
        ambient[2] = ambientColor[2];
        ambient[3] = ambientColor[3];
    }

    void setSunColor(float r, float g, float b, float a) {
        sunColor[0] = r;
        sunColor[1] = g;
        sunColor[2] = b;
        sunColor[3] = a;
    }

    void getSunColor(float *aSunColor){
        aSunColor[0] = sunColor[0];
        aSunColor[1] = sunColor[1];
        aSunColor[2] = sunColor[2];
        aSunColor[3] = sunColor[3];
    }

    void setCloseRiverColor(float r, float g, float b, float a) {
        closeRiverColor[0] = r;
        closeRiverColor[1] = g;
        closeRiverColor[2] = b;
        closeRiverColor[3] = a;
    }

    void getCloseRiverColor(float *aCloseRiverColor){
        aCloseRiverColor[0] = closeRiverColor[0];
        aCloseRiverColor[1] = closeRiverColor[1];
        aCloseRiverColor[2] = closeRiverColor[2];
        aCloseRiverColor[3] = closeRiverColor[3];
    }

    void setFogColor(float r, float g, float b, float a) {
        fogColor[0] = r;
        fogColor[1] = g;
        fogColor[2] = b;
        fogColor[3] = a;
    }

    void getFogColor(float *aFogColor){
        aFogColor[0] = fogColor[0];
        aFogColor[1] = fogColor[1];
        aFogColor[2] = fogColor[2];
        aFogColor[3] = fogColor[3];
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
};


#endif //WOWVIEWERLIB_CONFIG_H
