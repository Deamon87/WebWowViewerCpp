//
// Created by deamon on 15.06.17.
//

#ifndef WOWVIEWERLIB_CONFIG_H
#define WOWVIEWERLIB_CONFIG_H


class Config {

private:
    bool renderAdt = true;
    bool renderMd2 = true;
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

    int minBatch = 0;
    int maxBatch = 9999;

    int minParticle = 0;
    int maxParticle = 9999;

    bool useWotlkLogic = false;
public:
    bool getRenderM2 () {
        return renderMd2;
    }
    bool getUseInstancing () {
        return useInstancing;
    }
    void setRenderM2 (bool value) {
        renderMd2 = value;
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


    void setMinBatch(int value) {
        minBatch = value;
    }
    int getMinBatch() {
        return minBatch;
    }
    void setMaxBatch(int value) {
        maxBatch = value;
    }
    int getMaxBatch() {
        return maxBatch;
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
};


#endif //WOWVIEWERLIB_CONFIG_H
