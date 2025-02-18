#define _AMD64_ 1

//#define NOWINBASEINTERLOCK

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#ifdef _WIN32
 #include <errhandlingapi.h>
#endif

// Include GLEW. Always include it before gl.h and glfw.h, since it's a bit magic.
#define GLFW_INCLUDE_GLCOREARB
 
//#define __EMSCRIPTEN__

#ifdef LINK_VULKAN
#define GLFW_INCLUDE_VULKAN
#define VK_NO_PROTOTYPES
#else
#undef GLFW_INCLUDE_VULKAN
#endif


#include <cmath>
#include <vector>
#include <string>
#include <iostream>
#include <csignal>
#include <exception>
#include <algorithm>
#include <GLFW/glfw3.h>

#include "../wowViewerLib/src/gapi/interface/IDevice.h"
#include "../wowViewerLib/src/gapi/IDeviceFactory.h"
#include "ui/FrontendUI.h"
#include "../wowViewerLib/src/renderer/frame/SceneComposer.h"
#include "../wowViewerLib/src/engine/camera/firstPersonCamera.h"
#include "../wowViewerLib/src/engine/objects/scenes/map.h"
#include "screenshots/screenshotMaker.h"
#include "database/CEmptySqliteDB.h"
#include <exception>
#ifdef WIN32
#include <timeapi.h>
#endif
#ifdef LINK_TRACY
#include "Tracy.hpp"
#endif

int mleft_pressed = 0;
int mright_pressed = 0;
double m_x = 0.0;
double m_y = 0.0;

bool stopMouse = false;
bool stopKeyboard = false;

std::shared_ptr<FrontendUI> frontendUI = nullptr;

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos){
    if (stopMouse) {
        mleft_pressed = 0;
        mright_pressed = 0;
        return;
    };
    HApiContainer apiContainer = *(HApiContainer *)glfwGetWindowUserPointer(window);
    auto currentActiveScene = frontendUI->getCurrentActiveScene();
    auto controllable = currentActiveScene ? currentActiveScene->getCamera() : nullptr;

    if (!controllable) {
        mleft_pressed = 0;
        mright_pressed = 0;
        return;
    };

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
    if (stopMouse) return;

    HApiContainer apiContainer = *(HApiContainer *)glfwGetWindowUserPointer(window);
    auto currentActiveScene = frontendUI->getCurrentActiveScene();
    auto controllable = currentActiveScene ? currentActiveScene->getCamera() : nullptr;

    if (!controllable) {
        mleft_pressed = 0;
        mright_pressed = 0;
        return;
    };

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
//        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
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
    HApiContainer apiContainer = *(HApiContainer *)glfwGetWindowUserPointer(window);
    auto currentActiveScene = frontendUI->getCurrentActiveScene();
    auto controllable = currentActiveScene ? currentActiveScene->getCamera() : nullptr;

    if (!controllable) return;

    if ( action == GLFW_PRESS) {
        if (stopKeyboard) return;
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
            default:
                break;
        }
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    auto currentActiveScene = frontendUI->getCurrentActiveScene();
    auto controllable = currentActiveScene ? currentActiveScene->getCamera() : nullptr;

    if (!controllable) return;

    HApiContainer apiContainer = *(HApiContainer *)glfwGetWindowUserPointer(window);

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

int windowWidth = canvWidth;
int windowHeight = canvHeight;
bool windowSizeChanged = false;

void window_size_callback(GLFWwindow* window, int width, int height)
{
    windowWidth = width;
    windowHeight = height;

    glfwGetFramebufferSize(window, &canvWidth, &canvHeight);

    if (frontendUI != nullptr) {
        frontendUI->setWindowSize(windowWidth, windowHeight);
    }

    windowSizeChanged = true;
}

void beforeCrash(void);




extern "C" void my_function_to_handle_aborts(int signal_number)
{
    /*Your code goes here. You can output debugging info.
      If you return from this function, and it was called
      because abort() was called, your program will exit or crash anyway
      (with a dialog box on Windows).
     */

    std::cout << "HELLO" << std::endl;
}

/*Do this early in your program's initialization */


#ifdef _WIN32
void beforeCrash() {
    std::cout << "HELLO" << std::endl;
    __debugbreak();
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


double currentFrame;
double lastFrame;

int main(int argc, char *argv[]) {
//    std::ofstream out("log.txt");
//    std::streambuf *coutbuf = std::cout.rdbuf(); //save old buf
//    std::cout.rdbuf(out.rdbuf()); //redirect std::cout to out.txt!
//    std::cerr.rdbuf(out.rdbuf()); //redirect std::cout to out.txt!

#ifdef _WIN32
    SetUnhandledExceptionFilter(windows_exception_handler);
    const bool SET_TERMINATE = std::set_terminate(beforeCrash);
#if !defined(_MSC_VER) && !defined(_LIBCPP_VERSION)
    const bool SET_TERMINATE_UNEXP = std::set_unexpected(beforeCrash);
#endif
#endif
    signal(SIGABRT, &my_function_to_handle_aborts);
    signal(SIGSEGV, &my_function_to_handle_aborts);


    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

    glfwInit();

    bool egl = false;
#ifdef WITH_GLESv2
    egl = true;
#endif

//    std::string rendererName = "ogl2";
//      std::string rendererName = "ogl3";
    std::string rendererName = "vulkan";

//    if (argc > 1 && std::string(argv[1]) == "-vulkan") {
//        rendererName = "vulkan";
//    }

    //FOR OGL

    if (rendererName == "ogl3") {
        glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
        if (egl) {
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL ES 3.1
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
            glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
            glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
        } else {
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //We don't want the old OpenGL

        }
//    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE); //We don't want the old OpenGL
    } else if ( rendererName == "ogl2") {
        glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2); // We want OpenGL 2.0
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
//    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE); //We don't want the old OpenGL

        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
        glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
    } else if (rendererName == "vulkan"){
        //For Vulkan
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    }

    auto window = glfwCreateWindow(windowWidth, windowHeight, "WowMapViewer", nullptr, nullptr);

#ifdef LINK_VULKAN
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
#else
    void *callback = nullptr;
#endif

    //For OGL
    if (rendererName == "ogl3" || rendererName == "ogl2")
    {
        glfwMakeContextCurrent(window);
    }

    //Open Sql storage
    HApiContainer apiContainer = std::make_shared<ApiContainer>();

    //Create device
    auto hdevice = IDeviceFactory::createDevice(rendererName, &callback);
    apiContainer->databaseHandler = std::make_shared<CEmptySqliteDB>() ;
    apiContainer->hDevice = hdevice;

    SceneComposer sceneComposer = SceneComposer(apiContainer);

    frontendUI = std::make_shared<FrontendUI>(apiContainer);

    glfwSetWindowUserPointer(window, &apiContainer);
    glfwSetKeyCallback(window, onKey);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetCursorPosCallback( window, cursor_position_callback);
    glfwSetWindowSizeCallback( window, window_size_callback);
    glfwSetWindowSizeLimits( window, canvWidth, canvHeight, GLFW_DONT_CARE, GLFW_DONT_CARE);
    glfwSetMouseButtonCallback( window, mouse_button_callback);

    GLFWmonitor* windowMonitor = glfwGetPrimaryMonitor();
    float xscale, yscale;
    glfwGetMonitorContentScale(windowMonitor, &xscale, &yscale);

    float uiScale = std::max<float>(xscale, yscale);
    std::cout << "uiScale = " << uiScale << std::endl;

    int width_mm, height_mm;
    glfwGetMonitorPhysicalSize(windowMonitor, &width_mm, &height_mm);

    if (width_mm > 0 && height_mm > 0) {
        std::cout << "monitor width = " << width_mm << "mm, monitor height = " << height_mm << "mm" << std::endl;
        float monitorDiag = sqrt((width_mm * width_mm) + (height_mm * height_mm)) * 3.0f / 64.0f;
        std::cout << "monitor diag " << monitorDiag << "''" << std::endl;

        if (monitorDiag > 20) {
            uiScale = std::min<float>(uiScale, 2.0f);
        }
        if (monitorDiag > 25) {
            uiScale = std::min<float>(uiScale, 1.5f);
        }

        std::cout << "corrected uiScale for monitor dimensions = " << uiScale << std::endl;
    }

    uiScale = 1.0;
    frontendUI->setUIScale(uiScale);

//    auto native_me = std::this_thread::get_id().native_handle();
#ifdef WIN32
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
    timeBeginPeriod(3);
#endif
    //This has to be called after setting all callbacks specific to this app.
    //ImGUI takes care of previous callbacks and calls them before applying it's own logic over data
    //Otherwise keys like backspace, delete etc wont work
    frontendUI->initImgui(window);
    {
        int width = frontendUI->getWindowWidth();
        int height = frontendUI->getWindowHeight();

        glfwSetWindowSize(window, width, height);
        glfwGetFramebufferSize(window, &canvWidth, &canvHeight);
    }
    glfwSwapInterval(0);
    tbb::global_control global_limit(oneapi::tbb::global_control::max_allowed_parallelism,
                                     2*apiContainer->getConfig()->hardwareThreadCount());

//try {
    while (!glfwWindowShouldClose(window)) {
        frontendUI->newFrame();
        stopMouse = frontendUI->getStopMouse();
        stopKeyboard = frontendUI->getStopKeyboard();

        glfwPollEvents();

        // Render scene
        currentFrame = glfwGetTime(); // seconds
        double deltaTime = currentFrame - lastFrame;

        frontendUI->composeUI();
        auto sceneScenario = frontendUI->createFrameScenario(canvWidth, canvHeight, deltaTime);

        sceneComposer.draw(sceneScenario, windowSizeChanged);
        windowSizeChanged = false;

        double currentDeltaAfterDraw = (glfwGetTime() - currentFrame)*(1000.0f);
        lastFrame = currentFrame;
        if (currentDeltaAfterDraw < 5.0) {
            using namespace std::chrono_literals;
#ifdef WIN32
            sqlite3_sleep(5.0f-currentDeltaAfterDraw);
#else
            std::this_thread::sleep_for(std::chrono::milliseconds((int)(5.0-currentDeltaAfterDraw)));
#endif
        }

        if (rendererName == "ogl3" || rendererName == "ogl2") {
            glfwSwapBuffers(window);
        }

#ifdef LINK_TRACY
        FrameMark;
#endif
    }
//} catch(const std::exception &e){
//    std::cerr << e.what() << std::endl;
//    throw;
//} catch(...) {
//    std::cout << "something happened" << std::endl;
//}

    hdevice->waitForAllWorkToComplete();
    frontendUI->shutDown();

    // std::cout << "program ended" << std::endl;
#ifdef WIN32
    timeEndPeriod(3);
#endif
    //        while (1) {
    //            mainLoop(&myapp);
    //        }

    return 0;
}


