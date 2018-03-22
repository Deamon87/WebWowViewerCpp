#include <jni.h>
#include "../wowViewerLib/src/include/config.h"
#include "persistance/HttpRequestProcessor.h"

static Config *testConf;
static RequestProcessor *processor;
static WoWScene *scene;
int canvWidth;
int canvHeight;
bool windowSizeChanged = false;

extern "C" {
JNIEXPORT void JNICALL
Java_livingwallpaper_wow_deamon87_wowlivingwallpaper_NativeWoWLib_createScene(JNIEnv *env,
                                                                              jobject thiz,
                                                                              jint canvWidth,
                                                                              jint canvHeight) {
    //    const char *url = "http://deamon87.github.io/WoWFiles/shattrath.zip\0";
//    const char *url = "http://deamon87.github.io/WoWFiles/ironforge.zip\0";
//    const char *filePath = "D:\\shattrath (1).zip\0";
//    const char *filePath = "D:\\ironforge.zip\0";
    const char *url = "http://178.165.92.24:40001/get/";
    const char *urlFileId = "http://178.165.92.24:40001/get_file_id/";
//    const char *filePath = "d:\\Games\\WoW_3.3.5._uwow.biz_EU\\Data\\\0";
    const char *filePath = "d:\\Games\\WoWLimitedUS\\World of Warcraft\\\0";
//     const char *url = "http://localhost:8084/get/";


    testConf = new Config();

//    HttpZipRequestProcessor *processor = new HttpZipRequestProcessor(url);
//    ZipRequestProcessor *processor = new ZipRequestProcessor(filePath);
//    MpqRequestProcessor *processor = new MpqRequestProcessor(filePath);
    processor = new HttpRequestProcessor(url, urlFileId);
//        CascRequestProcessor *processor = new CascRequestProcessor(filePath);
    processor->setThreaded(false);

    scene = createWoWScene(testConf, processor, canvWidth, canvHeight);
    processor->setFileRequester(scene);
    testConf->setDrawM2BB(false);
//    testConf->setUsePortalCulling(false);
}
JNIEXPORT void JNICALL
Java_livingwallpaper_wow_deamon87_wowlivingwallpaper_NativeWoWLib_draw(JNIEnv *env, jobject thiz,
                                                                       jdouble deltaTime) {
    processor->processRequests(false);
    processor->processResults(10);

    if (windowSizeChanged) {
        scene->setScreenSize(canvWidth, canvHeight);
        windowSizeChanged = false;
    }

    scene->draw((deltaTime * 1000));
}

JNIEXPORT void JNICALL
        Java_livingwallpaper_wow_deamon87_wowlivingwallpaper_NativeWoWLib_changeDimensions(JNIEnv *env,
                                                                                           jobject thiz,
                                                                                           jint a_canvWidth,
                                                                                           jint a_canvHeight) {
    canvWidth = a_canvWidth;
    canvHeight = a_canvHeight;

    windowSizeChanged = true;
}
}