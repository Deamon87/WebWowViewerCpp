#include <jni.h>
#include <string>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/native_window_jni.h>

#include "../../../../wowViewerLib/src/engine/ApiContainer.h"
#include "../../../../wowViewerLib/src/gapi/IDeviceFactory.h"
#include "../../../../src/database/CEmptySqliteDB.h"
#include "../../../../wowViewerLib/src/engine/camera/firstPersonCamera.h"
#include "../../../../wowViewerLib/src/engine/SceneComposer.h"
#include "../../../../src/ui/FrontendUI.h"
#include "../../../../src/persistance/HttpRequestProcessor.h"
#include "../../../../wowViewerLib/src/engine/androidLogSupport.h"

HttpRequestProcessor *processor= nullptr;
SceneComposer *sceneComposer= nullptr;
HApiContainer apiContainer= nullptr;
std::shared_ptr<FrontendUI> frontendUI = nullptr;

int canvWidth;
int canvHeight;

AAssetManager * g_assetMgr = nullptr;

extern "C" {
    JNIEXPORT jstring JNICALL Java_com_deamon_wow_viewer_MainActivity_stringFromJNI(
            JNIEnv *env,
            jobject /* this */) {
        std::string hello = "Hello from C++";
        return env->NewStringUTF(hello.c_str());
    }

    JNIEXPORT void JNICALL Java_com_deamon_wow_viewer_MainActivity_createScene(
            JNIEnv *env,
            jobject thiz,
            jint canvWidth,
            jint canvHeight,
            jobject surface,
            jobject assetManager) {
        std::cout.rdbuf(new androidbuf);

        g_assetMgr = AAssetManager_fromJava(env, assetManager);

        void *callback = nullptr;

        //Open Sql storage
        apiContainer = std::make_shared<ApiContainer>();

        //Create device
        std::string rendererName = "ogl3";
        auto hdevice = IDeviceFactory::createDevice(rendererName, &callback);
        apiContainer->databaseHandler = std::make_shared<CEmptySqliteDB>() ;
        apiContainer->hDevice = hdevice;
        apiContainer->camera = std::make_shared<FirstPersonCamera>();

        sceneComposer = new SceneComposer(apiContainer);


        frontendUI = std::make_shared<FrontendUI>(apiContainer, nullptr);
        auto window = ANativeWindow_fromSurface(env, surface);
        frontendUI->initImgui(window);
    }

    JNIEXPORT void JNICALL Java_com_deamon_wow_viewer_MainActivity_gameloop(
            JNIEnv *env,
            jobject thiz,
            jdouble deltaTime) {

        if (sceneComposer == nullptr) return;

        processor->processRequests(false);

        apiContainer->camera->tick(deltaTime*(1000.0f));

        //DrawStage for screenshot
//        needToMakeScreenshot = true;
        if (apiContainer->getConfig()->pauseAnimation) {
            deltaTime = 0.0;
        }
        auto sceneScenario = frontendUI->createFrameScenario(canvWidth, canvHeight, deltaTime);

        sceneComposer->draw(sceneScenario);
    }

}