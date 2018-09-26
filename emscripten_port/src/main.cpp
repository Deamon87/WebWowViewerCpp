#include "../../wowViewerLib/src/include/wowScene.h"
#include "RequestProcessor.h"
#include "HttpRequestProcessor.h"
#include <iostream>
#include <emscripten/threading.h>

WoWScene *scene;
RequestProcessor *processor;
Config *testConf;

extern "C" {
void createWebJsScene(int canvWidth, int canvHeight) {
    const char *url = "http://178.165.92.24:40001/get/";
    const char *urlFileId = "http://178.165.92.24:40001/get_file_id/";

    if (emscripten_has_threading_support()) {
        std::cout << "Has support";
    } else {
        std::cout << "Has no support";
    }

    processor = new HttpRequestProcessor(url, urlFileId);
    processor->setThreaded(false);

    testConf = new Config();

    scene = createWoWScene(testConf, processor, canvWidth, canvHeight);
}
}


extern "C" {
void gameloop(double deltaTime) {
    processor->processRequests(false);
    processor->processResults(10);

//    if (windowSizeChanged) {
//        myapp->scene->setScreenSize(canvWidth, canvHeight);
//        windowSizeChanged = false;
//    }

    scene->draw((deltaTime * 1000));
}
}

int main() {

}