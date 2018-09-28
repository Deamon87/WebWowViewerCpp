#include "../../wowViewerLib/src/include/wowScene.h"
#include "RequestProcessor.h"
#include "HttpRequestProcessor.h"
#include <iostream>
#include <emscripten/threading.h>
#include <GLFW/glfw3.h>

WoWScene *scene;
RequestProcessor *processor;
Config *testConf;

int canvWidth = 640;
int canvHeight = 480;
bool windowSizeChanged = false;

int mleft_pressed = 0;
double m_x = 0.0;
double m_y = 0.0;

GLFWwindow * window;

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos){
    WoWScene * scene = (WoWScene *)glfwGetWindowUserPointer(window);
    IControllable* controllable = scene->getCurrentCamera();

//    if (!pointerIsLocked) {
    if (mleft_pressed == 1) {
        controllable->addHorizontalViewDir((xpos - m_x) / 4.0);
        controllable->addVerticalViewDir((ypos - m_y) / 4.0);

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
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        mleft_pressed = 1;
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        m_x = xpos;
        m_y = ypos;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        mleft_pressed = 0;
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
            default:
                break;
        }
    }

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

        scene = createWoWScene(testConf, processor, canvWidth, canvHeight);
        processor->setFileRequester(scene);

        glfwSetWindowUserPointer(window, scene);
        glfwSetKeyCallback(window, onKey);
        glfwSetCursorPosCallback(window, cursor_position_callback);
        glfwSetWindowSizeCallback(window, window_size_callback);
        glfwSetMouseButtonCallback(window, mouse_button_callback);
    }
}

extern "C" {
    void setScene(int sceneType, char *name, int cameraNum) {
        scene->setScene(sceneType, std::string(name), cameraNum);
    }
    void setSceneFileDataId(int sceneType, int fileDataId, int cameraNum) {
        scene->setSceneWithFileDataId(sceneType, fileDataId, cameraNum);
    }
}

extern "C" {
    void gameloop(double deltaTime) {
        processor->processRequests(false);
        processor->processResults(10);

        if (windowSizeChanged) {
            scene->setScreenSize(canvWidth, canvHeight);
            windowSizeChanged = false;
        }

        scene->draw((deltaTime * 1000));
    }
}



int main() {

}