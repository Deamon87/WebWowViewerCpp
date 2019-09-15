#define _X86_ 1


#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#ifdef _WIN32
#include <errhandlingapi.h>
#endif

// Include GLEW. Always include it before gl.h and glfw.h, since it's a bit magic.
#define GLFW_INCLUDE_GLCOREARB

//#define __EMSCRIPTEN__
#include "engine/HeadersGL.h"

#ifdef __EMSCRIPTEN__
#undef GLFW_INCLUDE_VULKAN
#else
#define GLFW_INCLUDE_VULKAN
#endif

#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include <cmath>

#include "../wowViewerLib/src/include/wowScene.h"
//#include "persistance/ZipRequestProcessor.h"
#include "persistance/CascRequestProcessor.h"
#include "persistance/HttpZipRequestProcessor.h"
#include "persistance/MpqRequestProcessor.h"
#include "persistance/HttpRequestProcessor.h"
#include "../wowViewerLib/src/include/vulkancontext.h"

#include "../wowViewerLib/src/gapi/interface/IDevice.h"
#include "../wowViewerLib/src/gapi/IDeviceFactory.h"


int mleft_pressed = 0;
int mright_pressed = 0;
double m_x = 0.0;
double m_y = 0.0;

bool stopInputs = false;

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos){
    if (stopInputs) return;
    WoWScene * scene = (WoWScene *)glfwGetWindowUserPointer(window);
    IControllable* controllable = scene->getCurrentCamera();

//    if (!pointerIsLocked) {
        if (mleft_pressed == 1) {
            controllable->addHorizontalViewDir((xpos - m_x) / 4.0f);
            controllable->addVerticalViewDir((ypos - m_y) / 4.0f);

            m_x = xpos;
            m_y = ypos;
        } else if (mright_pressed == 1) {
            controllable->addCameraViewOffset((xpos - m_x) / 8.0f, -(ypos - m_y) / 8.0f);

            m_x = xpos;
            m_y = ypos;
        }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
//    addCameraViewOffset
    if (stopInputs) return;
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

Config *testConf;
static void onKey(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (stopInputs) return;
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

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    WoWScene * scene = (WoWScene *)glfwGetWindowUserPointer(window);
    IControllable* controllable = scene->getCurrentCamera();

    controllable->zoomInFromMouseScroll(-yoffset/2.0f);
}

double calcFPS(GLFWwindow* window, double timeInterval = 1.0, std::string windowTitle = "NONE")
{
    // Static values which only get initialised the first time the function runs
    static double startTime  =  glfwGetTime(); // Set the initial time to now
    static double fps        =  0.0;           // Set the initial FPS value to 0.0

    // Set the initial frame count to -1.0 (it gets set to 0.0 on the next line). Because
    // we don't have a start time we simply cannot get an accurate FPS value on our very
    // first read if the time interval is zero, so we'll settle for an FPS value of zero instead.
    static double frameCount =  -1.0;

    // Here again? Increment the frame count
    frameCount++;

    // Ensure the time interval between FPS checks is sane (low cap = 0.0 i.e. every frame, high cap = 10.0s)
    if (timeInterval < 0.0)
    {
        timeInterval = 0.0;
    }
    else if (timeInterval > 10.0)
    {
        timeInterval = 10.0;
    }

    // Get the duration in seconds since the last FPS reporting interval elapsed
    // as the current time minus the interval start time
    double duration = glfwGetTime() - startTime;

    // If the time interval has elapsed...
    if (duration > timeInterval)
    {
        // Calculate the FPS as the number of frames divided by the duration in seconds
        fps = frameCount / duration;
        std::cout << "fps = " << fps << std::endl;

        // If the user specified a window title to append the FPS value to...
        if (windowTitle != "NONE")
        {
            // Convert the fps value into a string using an output stringstream
            std::string fpsString = std::to_string(fps);

            // Append the FPS value to the window title details
            windowTitle += " | FPS: " + fpsString;

            // Convert the new window title to a c_str and set it
            const char* pszConstString = windowTitle.c_str();
            glfwSetWindowTitle(window, pszConstString);
        }
        else // If the user didn't specify a window to append the FPS to then output the FPS to the console
        {
//            std::cout << "FPS: " << fps << std::endl;
        }

        // Reset the frame count to zero and set the initial time to be now
        frameCount        = 0.0;
        startTime = glfwGetTime();
    }

    // Return the current FPS - doesn't have to be used if you don't want it!
    return fps;
}

int canvWidth = 640;
int canvHeight = 480;
bool windowSizeChanged = false;

void window_size_callback(GLFWwindow* window, int width, int height)
{
    canvWidth = width;
    canvHeight = height;
    windowSizeChanged = true;
}

void beforeCrash(void);

#ifdef _WIN32
static const bool SET_TERMINATE = std::set_terminate(beforeCrash);

void beforeCrash() {
    //__asm("int3");
}

static LONG WINAPI windows_exception_handler(EXCEPTION_POINTERS * ExceptionInfo)
{
    switch(ExceptionInfo->ExceptionRecord->ExceptionCode)
    {
        case EXCEPTION_ACCESS_VIOLATION:
            fputs("Error: EXCEPTION_ACCESS_VIOLATION\n", stderr);
            break;
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
            fputs("Error: EXCEPTION_ARRAY_BOUNDS_EXCEEDED\n", stderr);
            break;
        case EXCEPTION_BREAKPOINT:
            fputs("Error: EXCEPTION_BREAKPOINT\n", stderr);
            break;
    }
	return 0;
}
#endif


enum radioOptions {
    EASY,
    HARD
};

struct my_nkc_app {
    /* some user data */
    float movementSpeed = 1.0;
    bool drawM2AABB = false;
    bool drawWMOAABB = false;

    int wmoMinBatch = 0;
    int wmoMaxBatch = 9999;

    int m2MinBatch = 0;
    int m2MaxBatch = 9999;

    int minParticle = 0;
    int maxParticle = 9999;

    WoWScene *scene;
    RequestProcessor *processor;

    double currentFrame;
    double lastFrame;

    float ambientColor[4] = {255,255,255,255};
    float ambientIntensity = 1.0;
    float sunColor[4];
};

inline void CopyAndNullTerminate( const std::string& source,
                                  char* dest,
                                  size_t dest_size )
{
    dest[source.copy(dest, dest_size-1)] = 0;
}

void mainLoop(void* loopArg){
    struct my_nkc_app* myapp = (struct my_nkc_app*)loopArg;

    // Render scene
    myapp->currentFrame = glfwGetTime(); // seconds
    double deltaTime = myapp->currentFrame - myapp->lastFrame;
    myapp->lastFrame = myapp->currentFrame;

    double fps = calcFPS(nullptr, 2.0);

    myapp->processor->processRequests(false);
    myapp->processor->processResults(10);

    if (windowSizeChanged) {
        myapp->scene->setScreenSize(canvWidth, canvHeight);
        windowSizeChanged = false;
    }

    myapp->scene->draw((deltaTime*(1000.0f))); //miliseconds

    stopInputs = false;
}



int main(){
    struct my_nkc_app myapp;


#ifdef _WIN32
    SetUnhandledExceptionFilter(windows_exception_handler);
#endif


//    const char *url = "http://deamon87.github.io/WoWFiles/shattrath.zip\0";
//    const char *url = "http://deamon87.github.io/WoWFiles/ironforge.zip\0";
//    const char *filePath = "D:\\shattrath (1).zip\0";
//    const char *filePath = "D:\\ironforge.zip\0";
    const char * url = "https://wow.tools/casc/file/fname?buildconfig=081460ab8f317f8273068b29288f2af7&cdnconfig=a00c4591f68ddf55d8baa84db360a8d0&filename=";
    const char * urlFileId = "https://wow.tools/casc/file/fdid?buildconfig=081460ab8f317f8273068b29288f2af7&cdnconfig=a00c4591f68ddf55d8baa84db360a8d0&filename=data&filedataid=";
//1.13.0
//    const char * url = "https://bnet.marlam.in/casc/file/fname?buildconfig=db00c310c6ba0215be3f386264402d56&cdnconfig=1e32d08ef668e70aac36a516bd43dff1&filename=";
//    const char * urlFileId = "https://bnet.marlam.in/casc/file/fdid?buildconfig=db00c310c6ba0215be3f386264402d56&cdnconfig=1e32d08ef668e70aac36a516bd43dff1&filename=data&filedataid=";

//    const char * url = "http://178.165.92.24:40001/get/";
//    const char * urlFileId = "http://178.165.92.24:40001/get_file_id/";

//    const char *filePath = "d:\\Games\\WoW_3.3.5._uwow.biz_EU\\Data\\\0";
    const char *filePath = "d:\\Games\\WoWLimitedUS\\World of Warcraft\\\0";
//     const char *url = "http://localhost:8084/get/";

    testConf = new Config();
    testConf->setAmbientColor(1,1,1,1);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);



    printf("Successfull init. Starting 'infinite' main loop...\n");

    //    HttpZipRequestProcessor *processor = new HttpZipRequestProcessor(url);
    //    ZipRequestProcessor *processor = new ZipRequestProcessor(filePath);
    //    MpqRequestProcessor *processor = new MpqRequestProcessor(filePath);
    HttpRequestProcessor *processor = new HttpRequestProcessor(url, urlFileId);
//        CascRequestProcessor *processor = new CascRequestProcessor(filePath);
    processor->setThreaded(true);


    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    auto window = glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);

    vkCallInitCallback callback;
    callback.createSurface = [&](VkInstance vkInstance) {
        VkSurfaceKHR surface;

        if (glfwCreateWindowSurface(vkInstance, window, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }

        return surface;
    };
    callback.getRequiredExtensions = [](char** &extensionNames, int &extensionCnt) {
        uint32_t count;
        extensionNames = const_cast<char **>(glfwGetRequiredInstanceExtensions(&count));
        extensionCnt = count;
    };


    //Create device
    IDevice * device = IDeviceFactory::createDevice("vulkan", &callback);
    WoWScene *scene = createWoWScene(testConf, processor, device, canvWidth, canvHeight);
    processor->setFileRequester(scene);
    testConf->setDrawM2BB(false);
    //testConf->setUsePortalCulling(false);

    myapp.scene = scene;
    myapp.processor = processor;
    myapp.currentFrame = glfwGetTime();
    myapp.lastFrame = myapp.currentFrame;


    glfwSetWindowUserPointer(window, scene);
    glfwSetKeyCallback(window, onKey);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetCursorPosCallback( window, cursor_position_callback);
    glfwSetWindowSizeCallback( window, window_size_callback);
    glfwSetWindowSizeLimits( window, canvWidth, canvHeight, GLFW_DONT_CARE, GLFW_DONT_CARE);
    glfwSetMouseButtonCallback( window, mouse_button_callback);
//        glfwSwapInterval(0);

try {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Render scene
        myapp.currentFrame = glfwGetTime(); // seconds
        double deltaTime = myapp.currentFrame - myapp.lastFrame;
        myapp.lastFrame = myapp.currentFrame;

        double fps = calcFPS(nullptr, 2.0);


        processor->processRequests(false);
        processor->processResults(10);

        if (windowSizeChanged) {
            scene->setScreenSize(canvWidth, canvHeight);
            windowSizeChanged = false;
        }

        scene->draw((deltaTime*(1000.0f))); //miliseconds
    }
} catch(const std::exception &e){
    std::cerr << e.what() << std::endl;
    throw;
} catch(...) {
    std::cout << "something happened" << std::endl;
}

std::cout << "program ended" << std::endl;
//        while (1) {
//            mainLoop(&myapp);
//        }



    delete myapp.scene;

    return 0;
}
