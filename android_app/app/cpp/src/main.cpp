#include <jni.h>
#include <string>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/native_window_jni.h>
#include <android/input.h>

#include "../../../../wowViewerLib/src/engine/ApiContainer.h"
#include "../../../../wowViewerLib/src/gapi/IDeviceFactory.h"
#include "../../../../src/database/CEmptySqliteDB.h"
#include "../../../../wowViewerLib/src/engine/camera/firstPersonCamera.h"
#include "../../../../wowViewerLib/src/engine/SceneComposer.h"
#include "../../../../src/ui/FrontendUI.h"
#include "../../../../src/persistance/HttpRequestProcessor.h"
#include "../../../../wowViewerLib/src/engine/androidLogSupport.h"

SceneComposer *sceneComposer= nullptr;
HApiContainer apiContainer= nullptr;
std::shared_ptr<FrontendUI> frontendUI = nullptr;

int canvWidth;
int canvHeight;

AAssetManager * g_assetMgr = nullptr;
static JavaVM *gJavaVM;

#define logExecution { \
    std::cout << "Passed "<<__FUNCTION__<<" line " << __LINE__ << std::endl;\
}

extern "C" {
    JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved)
    {
        gJavaVM = vm;
        std::cout.rdbuf(new androidbuf);
        return JNI_VERSION_1_6;
    }

    int cdToExtStorage(void) {

        // Make JNI calls to get the external storage directory, and cd to it.

        // To begin, get a reference to the env and attach to it.
        JNIEnv *env;
        int isAttached = 0;
        int ret = 0;
        jthrowable exception;
        if ((gJavaVM->GetEnv((void**)&env, JNI_VERSION_1_6)) < 0) {
            // Couldn't get JNI environment, so this thread is native.
            if ((gJavaVM->AttachCurrentThread(&env, NULL)) < 0) {
                fprintf(stderr, "Error: Couldn't attach to Java VM.\n");
                return (-1);
            }
            isAttached = 1;
        }

        // Get File object for the external storage directory.
        jclass classEnvironment = env->FindClass("android/os/Environment");
        if (!classEnvironment) {
            std::cout << "Error: JNI call failure.\n" << std::endl;
        };
        jmethodID methodIDgetExternalStorageDirectory = env->GetStaticMethodID(classEnvironment, "getExternalStorageDirectory", "()Ljava/io/File;"); // public static File getExternalStorageDirectory ()
        if (!methodIDgetExternalStorageDirectory)  {
            std::cout << "Error: JNI call failure.\n" << std::endl;
        };
        jobject objectFile = env->CallStaticObjectMethod(classEnvironment, methodIDgetExternalStorageDirectory);
        exception = env->ExceptionOccurred();
        if (exception) {
            env->ExceptionDescribe();
            env->ExceptionClear();
        }

        // Call method on File object to retrieve String object.
        jclass classFile = env->GetObjectClass(objectFile);
        if (!classFile) {
            std::cout << "Error: JNI call failure.\n" << std::endl;
        }
        jmethodID methodIDgetAbsolutePath = env->GetMethodID(classFile, "getAbsolutePath", "()Ljava/lang/String;");
        if (!methodIDgetAbsolutePath) {
            std::cout << "Error: JNI call failure.\n" << std::endl;
        }

        jstring stringPath = static_cast<jstring>(env->CallObjectMethod(objectFile,
                                                                        methodIDgetAbsolutePath));
        exception = env->ExceptionOccurred();
        if (exception) {
            env->ExceptionDescribe();
            env->ExceptionClear();
        }

        // Extract a C string from the String object, and chdir() to it.
        const char *wpath3 = env->GetStringUTFChars(stringPath, NULL);
        if (chdir(wpath3) != 0) {
            fprintf(stderr, "Error: Unable to change working directory to %s.\n", wpath3);
            perror(NULL);
        }

        env->ReleaseStringUTFChars(stringPath, wpath3);

        if (isAttached) gJavaVM->DetachCurrentThread(); // Clean up.
        return (ret);
    }

    JNIEXPORT void JNICALL
    Java_com_deamon_wow_viewer_MainActivity_onTouch(JNIEnv *jenv, jobject thiz,jobject motionEvent) {
        const AInputEvent *inputEvent = AMotionEvent_fromJava(jenv, motionEvent);

        AKeyEvent_getAction(inputEvent);
        AInputEvent_release(inputEvent);
    }

    JNIEXPORT jstring JNICALL Java_com_deamon_wow_viewer_MainActivity_stringFromJNI(
            JNIEnv *env,
            jobject /* this */) {
        std::string hello = "Hello from C++";
        return env->NewStringUTF(hello.c_str());
    }

    JNIEXPORT void JNICALL Java_com_deamon_wow_viewer_MainActivity_createScene(
            JNIEnv *env,
            jobject thiz,
            jint lcanvWidth,
            jint lcanvHeight,
            jobject surface,
            jobject assetManager) {

        canvHeight = lcanvHeight;
        canvWidth = lcanvWidth;

        cdToExtStorage();

        g_assetMgr = AAssetManager_fromJava(env, assetManager);

        void *callback = nullptr;

        //Open Sql storage
        apiContainer = std::make_shared<ApiContainer>();

        //Create device
        std::string rendererName = "ogl3";
        logExecution
        auto hdevice = IDeviceFactory::createDevice(rendererName, &callback);
        logExecution
        apiContainer->databaseHandler = std::make_shared<CEmptySqliteDB>() ;
        logExecution
        apiContainer->hDevice = hdevice;
        logExecution
        apiContainer->camera = std::make_shared<FirstPersonCamera>();
        logExecution

        sceneComposer = new SceneComposer(apiContainer);
        logExecution


        logExecution
        frontendUI = std::make_shared<FrontendUI>(apiContainer, nullptr);
        logExecution
        auto window = ANativeWindow_fromSurface(env, surface);
        logExecution
        frontendUI->initImgui(window);
        logExecution
    }

    JNIEXPORT void JNICALL Java_com_deamon_wow_viewer_MainActivity_gameloop(
            JNIEnv *env,
            jobject thiz,
            jdouble deltaTime) {
        logExecution
        if (sceneComposer == nullptr) return;
        logExecution
        frontendUI->newFrame();
        frontendUI->composeUI();

        logExecution
        {
            auto processor = frontendUI->getProcessor();
            if (frontendUI->getProcessor()) {

                if (!processor->getThreaded()) {
                    processor->processRequests(false);
                }
            }
        }
        logExecution
        apiContainer->camera->tick(deltaTime*(1000.0f));
        logExecution
        //DrawStage for screenshot
//        needToMakeScreenshot = true;
        if (apiContainer->getConfig()->pauseAnimation) {
            deltaTime = 0.0;
        }
        logExecution
        auto sceneScenario = frontendUI->createFrameScenario(canvWidth, canvHeight, deltaTime);
        logExecution
        sceneComposer->draw(sceneScenario);
        logExecution
    }

}