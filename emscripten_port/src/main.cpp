#include "../../wowViewerLib/src/include/wowScene.h"
#include "../../wowViewerLib/src/gapi/IDeviceFactory.h"
#include "RequestProcessor.h"
#include "HttpRequestProcessor.h"
#include <iostream>
#include <emscripten/threading.h>
#include <emscripten.h>
#include <GLFW/glfw3.h>

WoWScene *scene;
HttpRequestProcessor *processor;
Config *testConf;

int canvWidth = 640;
int canvHeight = 480;
bool windowSizeChanged = false;

int mleft_pressed = 0;
int mright_pressed = 0;
double m_x = 0.0;
double m_y = 0.0;

GLFWwindow * window;

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos){
    WoWScene * scene = (WoWScene *)glfwGetWindowUserPointer(window);
    IControllable* controllable = scene->getCurrentCamera();

//    if (!pointerIsLocked) {
    if (mleft_pressed == 1) {
        controllable->addHorizontalViewDir((xpos - m_x) / 4.0f);
        controllable->addVerticalViewDir((ypos - m_y) / 4.0f);

        m_x = xpos;
        m_y = ypos;
    } else if (mright_pressed == 1) {
        controllable->addCameraViewOffset((xpos - m_x) / 64.0f, -(ypos - m_y) / 64.0f);

        m_x = xpos;
        m_y = ypos;
    }

//    } else {
//        var delta_x = event.movementX ||
//                      event.mozMovementX          ||
//                      event.webkitMovementX       ||
//                      0;
//        var delta_y = event.movementY ||
//                      event.mozMovementY      ||
//                      event.webkitMovementY   ||
//                      0;
//
//        camera.addHorizontalViewDir((delta_x) / 4.0);
//        camera.addVerticalViewDir((delta_y) / 4.0);
//    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (action == GLFW_PRESS) {
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            mleft_pressed = 1;
        } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            mright_pressed = 1;
        }

        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        m_x = xpos;
        m_y = ypos;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    if (action == GLFW_RELEASE) {
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            mleft_pressed = 0;
        } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            mright_pressed = 0;
        }
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

static void onKey(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    WoWScene * scene = (WoWScene *)glfwGetWindowUserPointer(window);
    IControllable* controllable = scene->getCurrentCamera();
    if ( action == GLFW_PRESS) {
        switch (key) {
            case 'W' :
                controllable->startMovingForward();
                break;
            case 'S' :
                controllable->startMovingBackwards();
                break;
            case 'A' :
                controllable->startStrafingLeft();
                break;
            case 'D':
                controllable->startStrafingRight();
                break;
            case 'Q':
                controllable->startMovingUp();
                break;
            case 'E':
                controllable->startMovingDown();
                break;
            case GLFW_KEY_LEFT_SHIFT:
                testConf->setMovementSpeed(3.0);
                break;
            default:
                break;
        }
    } else if ( action == GLFW_RELEASE) {
        switch (key) {
            case 'W' :
                controllable->stopMovingForward();
                break;
            case 'S' :
                controllable->stopMovingBackwards();
                break;
            case 'A' :
                controllable->stopStrafingLeft();
                break;
            case 'D':
                controllable->stopStrafingRight();
                break;
            case 'Q':
                controllable->stopMovingUp();
                break;
            case 'E':
                controllable->stopMovingDown();
                break;
            case 'H':
                scene->switchCameras();
                break;
            case 'J':
                testConf->setDoubleCameraDebug(!testConf->getDoubleCameraDebug());
                break;
            case 'K':
                testConf->setRenderPortals(!testConf->getRenderPortals());
                break;

            case 'O':
                scene->setScenePos(0,0,0);
                break;
            case GLFW_KEY_LEFT_SHIFT:
                testConf->setMovementSpeed(1.0);
                break;
            default:
                break;
        }
    }

}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    WoWScene * scene = (WoWScene *)glfwGetWindowUserPointer(window);
    IControllable* controllable = scene->getCurrentCamera();

    controllable->zoomInFromMouseScroll(yoffset/240.0f);
}


void window_size_callback(GLFWwindow* window, int width, int height)
{
    canvWidth = width;
    canvHeight = height;
    windowSizeChanged = true;
}

extern "C" {
    void createWebJsScene(int canvWidth, int canvHeight, char *url, char *urlFileId) {
        if (!glfwInit()) {
            fputs("Failed to initialize GLFW", stderr);
            return;
        }

        emscripten_run_script("Module['hammerJsAssignControl']()");

    //    if (emscripten_has_threading_support()) {
    //        std::cout << "Has support";
    //    } else {
    //        std::cout << "Has no support";
    //    }
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
            glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

        window = glfwCreateWindow(640, 480, "My Title", NULL, NULL);

        if (!window) {
            fputs("Failed to create GLFW window", stderr);
            glfwTerminate();
            return;
        }

        glfwMakeContextCurrent(window);

        processor = new HttpRequestProcessor(url, urlFileId);
        processor->setThreaded(false);

        testConf = new Config();
        testConf->setSunColor(0.0,0.0,0.0,0.0);
        testConf->setAmbientColor(1.0,1.0,1.0,1.0);
        testConf->setRenderM2(true);
        testConf->setRenderWMO(true);

        IDevice * device = IDeviceFactory::createDevice("ogl3", nullptr);

        scene = createWoWScene(testConf, processor, device, canvWidth, canvHeight);
        processor->setFileRequester(scene);

        glfwSetWindowUserPointer(window, scene);
        glfwSetKeyCallback(window, onKey);
        glfwSetScrollCallback(window, scroll_callback);
        glfwSetCursorPosCallback(window, cursor_position_callback);
        glfwSetWindowSizeCallback(window, window_size_callback);
        glfwSetMouseButtonCallback(window, mouse_button_callback);
    }
}

extern "C" {
    void setNewUrls(char *url, char *urlFileId) {
        processor->setUrls(url, urlFileId);
        scene->clearCache();
    }
    void setScene(int sceneType, char *name, int cameraNum) {
        scene->setScene(sceneType, std::string(name), cameraNum);
    }
    EMSCRIPTEN_KEEPALIVE
    void setMap(int mapId, int wdtFileId, float x, float y, float z) {
        scene->setMap(mapId, wdtFileId, x, y, z);
    }
    void setSceneFileDataId(int sceneType, int fileDataId, int cameraNum) {
        scene->setSceneWithFileDataId(sceneType, fileDataId, cameraNum);
    }
    void setSceneSize(int width, int height) {
        canvWidth = width;
        canvHeight = height;
        scene->setScreenSize(canvWidth, canvHeight);
    }
    void addHorizontalViewDir(float val) {
        IControllable* controllable = scene->getCurrentCamera();
        controllable->addHorizontalViewDir(val);
    }
    void addVerticalViewDir(float val ) {
        IControllable* controllable = scene->getCurrentCamera();
        controllable->addVerticalViewDir(val);
    }
    void zoomInFromMouseScroll(float val ) {
        IControllable* controllable = scene->getCurrentCamera();
        controllable->zoomInFromMouseScroll(val);
    }
    void addCameraViewOffset(float val1, float val2 ) {
        IControllable* controllable = scene->getCurrentCamera();
        controllable->addCameraViewOffset(val1, val2);
    }
    void startMovingForward() {
        IControllable* controllable = scene->getCurrentCamera();
        controllable->startMovingForward();
    }
    void startMovingBackwards() {
        IControllable* controllable = scene->getCurrentCamera();
        controllable->startMovingBackwards();
    }
    void stopMovingForward() {
        IControllable* controllable = scene->getCurrentCamera();
        controllable->stopMovingForward();
    }
    void stopMovingBackwards() {
        IControllable* controllable = scene->getCurrentCamera();
        controllable->stopMovingBackwards();
    }

    EMSCRIPTEN_KEEPALIVE
    void setClearColor(float r, float g, float b) {
        testConf->setClearColor(r,g,b,0.0);
    }
    EMSCRIPTEN_KEEPALIVE
    void setFarPlane(float value) {
        testConf->setFarPlane(value);
    }
    EMSCRIPTEN_KEEPALIVE
    void setFarPlaneForCulling(float value) {
        testConf->setFarPlaneForCulling(value);
    }

    EMSCRIPTEN_KEEPALIVE
    void setAnimationId(int animationId) {
        scene->setAnimationId(animationId);
    }

    EMSCRIPTEN_KEEPALIVE
    void setTextures(int *fileDataIdsBuff, int elementsCount) {
        std::vector<int> textures(fileDataIdsBuff, fileDataIdsBuff+elementsCount);

//        std::cout << "called! elementsCount = " << elementsCount << std::endl;

        scene->setReplaceTextureArray(textures);
    }

}

extern "C" {
    void gameloop(double deltaTime) {
        glfwPollEvents();

        processor->processRequests(false);
        processor->processResults(10);

        scene->draw((deltaTime * 1000.0f));
        glfwSwapBuffers(window);


//        std::cout << "testConf->getRenderWMO() " << testConf->getRenderWMO() << std::endl;
    }
}



int main() {

}