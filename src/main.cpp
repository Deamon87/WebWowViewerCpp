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
#include "engine/HeadersGL.h"

#ifdef LINK_VULKAN
#define GLFW_INCLUDE_VULKAN
#include "../wowViewerLib/src/include/vulkancontext.h"
#else
#undef GLFW_INCLUDE_VULKAN
#endif

#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include <cmath>
#include <csignal>
#include <exception>

//#include "persistance/ZipRequestProcessor.h"
#include "persistance/CascRequestProcessor.h"
#include "persistance/HttpZipRequestProcessor.h"
//#include "persistance/MpqRequestProcessor.h"
#include "persistance/HttpRequestProcessor.h"


#include "../wowViewerLib/src/gapi/interface/IDevice.h"
#include "../wowViewerLib/src/gapi/IDeviceFactory.h"
#include "ui/FrontendUI.h"
#include "database/CSqliteDB.h"
#include "../wowViewerLib/src/engine/WowFilesCacheStorage.h"
#include "../wowViewerLib/src/engine/SceneComposer.h"
#include "../wowViewerLib/src/engine/camera/firstPersonCamera.h"
#include "../wowViewerLib/src/engine/objects/scenes/map.h"
#include "../wowViewerLib/src/engine/ApiContainer.h"
#include "../wowViewerLib/src/engine/objects/scenes/wmoScene.h"
#include "../wowViewerLib/src/engine/objects/scenes/m2Scene.h"


int mleft_pressed = 0;
int mright_pressed = 0;
double m_x = 0.0;
double m_y = 0.0;

bool stopMouse = false;
bool stopKeyboard = false;

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos){
    if (stopMouse) return;
    ApiContainer * apiContainer = (ApiContainer *)glfwGetWindowUserPointer(window);
    auto controllable = apiContainer->camera;

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
    if (stopKeyboard) return;
    ApiContainer * apiContainer = (ApiContainer *)glfwGetWindowUserPointer(window);
    auto controllable = apiContainer->camera;

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
//                scene->switchCameras();
//                scene->setScene(0, "trash", 0);
//                scene->setAnimationId(159);
                break;
            case 'J':
//                scene->setAnimationId(0);
//                testConf->setDoubleCameraDebug(!testConf->getDoubleCameraDebug());
                break;
            case 'K':
//                testConf->setRenderPortals(!testConf->getRenderPortals());
                break;
            default:
                break;
        }
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (stopMouse) return;

    ApiContainer * apiContainer = (ApiContainer *)glfwGetWindowUserPointer(window);
    auto controllable = apiContainer->camera;

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

double currentFrame;
double lastFrame;

int main(){
//    std::ofstream out("log.txt");
//    std::streambuf *coutbuf = std::cout.rdbuf(); //save old buf
//    std::cout.rdbuf(out.rdbuf()); //redirect std::cout to out.txt!
//    std::cerr.rdbuf(out.rdbuf()); //redirect std::cout to out.txt!

#ifdef _WIN32
    SetUnhandledExceptionFilter(windows_exception_handler);
    const bool SET_TERMINATE = std::set_terminate(beforeCrash);
    //const bool SET_TERMINATE_UNEXP = std::set_unexpected(beforeCrash);
#endif

    signal(SIGABRT, &my_function_to_handle_aborts);


    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

    glfwInit();

//    std::string rendererName = "ogl2";
    std::string rendererName = "ogl3";
//    std::string rendererName = "vulkan";

    //FOR OGL

    if (rendererName == "ogl3") {
        glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //We don't want the old OpenGL
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE); //We don't want the old OpenGL
    } else if ( rendererName == "ogl2") {
        glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2); // We want OpenGL 3.3
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
//    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE); //We don't want the old OpenGL
    } else if (rendererName == "vulkan"){
        //For Vulkan
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    }

    auto window = glfwCreateWindow(canvWidth, canvHeight, "WowMapViewer", nullptr, nullptr);

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
	
    CSqliteDB *sqliteDB = new CSqliteDB("./export.db3");


    ApiContainer apiContainer;
    RequestProcessor *processor = nullptr;
//    {
        const char * url = "https://wow.tools/casc/file/fname?buildconfig=679386e82870b537ae98416ec461931a&cdnconfig=2aae82f313c31defc6f0df94f3c1ea00&filename=";
        const char * urlFileId = "https://wow.tools/casc/file/fdid?buildconfig=679386e82870b537ae98416ec461931a&cdnconfig=2aae82f313c31defc6f0df94f3c1ea00&filename=data&filedataid=";
//        processor = new HttpZipRequestProcessor(url);
////        processor = new ZipRequestProcessor(filePath);
////        processor = new MpqRequestProcessor(filePath);
        processor = new HttpRequestProcessor(url, urlFileId);
//        processor = new CascRequestProcessor("e:/games/wow beta/World of Warcraft Beta/");
////        processor->setThreaded(false);
////
        processor->setThreaded(true);
        apiContainer.cacheStorage = std::make_shared<WoWFilesCacheStorage>(processor);
        processor->setFileRequester(apiContainer.cacheStorage.get());

//    }
    //Create device
    auto hdevice = IDeviceFactory::createDevice(rendererName, &callback);

    std::shared_ptr<IScene> currentScene = nullptr;

    apiContainer.databaseHandler = sqliteDB;
    apiContainer.hDevice = hdevice;
    apiContainer.camera = std::make_shared<FirstPersonCamera>();

    SceneComposer sceneComposer = SceneComposer(&apiContainer);

    //    WoWScene *scene = createWoWScene(testConf, storage, sqliteDB, device, canvWidth, canvHeight);

    std::shared_ptr<FrontendUI> frontendUI = std::make_shared<FrontendUI>(hdevice);
    frontendUI->overrideCascOpened(true);
    frontendUI->setOpenCascStorageCallback([&processor, &apiContainer, &sceneComposer](std::string cascPath) -> bool {
        CascRequestProcessor *newProcessor = nullptr;
        std::shared_ptr<WoWFilesCacheStorage> newStorage = nullptr;
        try {
            newProcessor = new CascRequestProcessor(cascPath.c_str());
            newStorage = std::make_shared<WoWFilesCacheStorage>(newProcessor);
            newProcessor->setThreaded(true);
            newProcessor->setFileRequester(newStorage.get());
        } catch (...){
            delete newProcessor;
            return false;
        };

        apiContainer.cacheStorage = newStorage;
        processor = newProcessor;

        return true;
    });
    frontendUI->setOpenSceneByfdidCallback([&currentScene, &apiContainer](int mapId, int wdtFileId, float x, float y, float z) {
//        scene->setSceneWithFileDataId(1, 113992, -1); //Ironforge
        if (apiContainer.cacheStorage) {
//            storage->actuallDropCache();
        }

        currentScene = std::make_shared<Map>(&apiContainer, mapId, wdtFileId);
        apiContainer.camera->setCameraPos(x, y, z);
//        scene->setMap(mapId, wdtFileId, x, y, z); //Ironforge
    });
    frontendUI->setOpenWMOSceneByfdidCallback([&currentScene, &apiContainer](int wmoFDid) {
        currentScene = std::make_shared<WmoScene>(&apiContainer, wmoFDid);
        apiContainer.camera->setCameraPos(0, 0, 0);
    });
    frontendUI->setOpenM2SceneByfdidCallback([&currentScene, &apiContainer](int m2FDid, std::vector<int> &replacementTextureIds) {
        currentScene = std::make_shared<M2Scene>(&apiContainer, m2FDid, -1);
        currentScene->setReplaceTextureArray(replacementTextureIds);



//        apiContainer.getConfig()->setBCLightHack(true);
//
        apiContainer.camera->setCameraPos(0, 0, 0);
    });
    frontendUI->setOpenM2SceneByFilenameCallback([&currentScene, &apiContainer](std::string m2FileName, std::vector<int> &replacementTextureIds) {
        currentScene = std::make_shared<M2Scene>(&apiContainer, m2FileName, -1);
        currentScene->setReplaceTextureArray(replacementTextureIds);

        apiContainer.camera->setCameraPos(0, 0, 0);
    });

    frontendUI->setFarPlaneChangeCallback([&apiContainer](float farPlane) -> void {
        auto conf = apiContainer.getConfig();
        conf->setFarPlane(farPlane);
        conf->setFarPlaneForCulling(farPlane+50);
    });
    frontendUI->setSpeedCallback([&apiContainer](float movementSpeed) -> void {
        auto conf = apiContainer.getConfig();
        conf->setMovementSpeed(movementSpeed);

        apiContainer.camera->setMovementSpeed(movementSpeed);
    });
    frontendUI->setThreadCountCallback([&apiContainer](int value) -> void {
        auto conf = apiContainer.getConfig();
        conf->setThreadCount(value);
    });

    frontendUI->setUseGaussBlurCallback([&apiContainer](bool value) -> void {
        auto conf = apiContainer.getConfig();
        conf->setUseGaussBlur(value);
    });

    frontendUI->setQuicksortCutoffCallback([&apiContainer](int value) -> void {
        auto conf = apiContainer.getConfig();
        conf->setQuickSortCutoff(value);
    });
    frontendUI->setGetCurrentAreaName([&apiContainer]()->std::string {
        auto conf = apiContainer.getConfig();
        return conf->getAreaName();
    });
    frontendUI->setUnloadScene([&apiContainer, &currentScene]()->void {
        if (apiContainer.cacheStorage) {
            apiContainer.cacheStorage->actuallDropCache();
        }
        currentScene = nullptr;
        //scene->setSceneWithFileDataId(-1, 0, -1);
    });

    frontendUI->setCurrentTimeChangeCallback([&apiContainer](int value) -> void {
        auto conf = apiContainer.getConfig();
        conf->setCurrentTime(value);
    });

    frontendUI->setGetCameraPos([&apiContainer](float &cameraX,float &cameraY,float &cameraZ) -> void {
        float currentCameraPos[4] = {0,0,0,0};
        apiContainer.camera->getCameraPosition(&currentCameraPos[0]);
        cameraX = currentCameraPos[0];
        cameraY = currentCameraPos[1];
        cameraZ = currentCameraPos[2];
    });
    frontendUI->setGetAdtSelectionMinimap([&frontendUI, &apiContainer](int wdtFileDataId) {
        auto wdtFile = apiContainer.cacheStorage->getWdtFileCache()->getFileId(wdtFileDataId);


        frontendUI->setFillAdtSelectionMinimap([&frontendUI, wdtFile, &apiContainer](std::array<std::array<HGTexture, 64>, 64> &minimap, bool &isWMOMap, bool &wdtFileExists) -> bool {
            if (wdtFile->getStatus() == FileStatus::FSRejected) {
                wdtFileExists = false;
                isWMOMap = false;
                return false;
            }

            if (wdtFile->getStatus() != FileStatus::FSLoaded) return false;

            isWMOMap = wdtFile->mphd->flags.wdt_uses_global_map_obj != 0;

            for (int i = 0; i < 64; i++) {
                for (int j = 0; j < 64; j++) {
                    if (wdtFile->mapFileDataIDs[i*64 + j].minimapTexture > 0) {
                        auto texture = apiContainer.cacheStorage->getTextureCache()->getFileId(wdtFile->mapFileDataIDs[i*64 + j].minimapTexture);
                        minimap[i][j] = apiContainer.hDevice->createBlpTexture(texture, false, false);
                    } else {
                        minimap[i][j] = nullptr;
                    }
                }
            }
            return true;
        });
    });
    frontendUI->setGetMapList([&sqliteDB](std::vector<MapRecord> &mapList) -> void {
        if (sqliteDB == nullptr)  return;

        sqliteDB->getMapArray(mapList);
    });





    glfwSetWindowUserPointer(window, &apiContainer);
    glfwSetKeyCallback(window, onKey);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetCursorPosCallback( window, cursor_position_callback);
    glfwSetWindowSizeCallback( window, window_size_callback);
    glfwSetWindowSizeLimits( window, canvWidth, canvHeight, GLFW_DONT_CARE, GLFW_DONT_CARE);
    glfwSetMouseButtonCallback( window, mouse_button_callback);

    //This has to be called after setting all callbacks specific to this app.
    //ImGUI takes care of previous callbacks and calls them before applying it's own logic over data
    //Otherwise keys like backspace, delete etc wont work

    frontendUI->initImgui(window);
    glfwSwapInterval(0);

//try {
    while (!glfwWindowShouldClose(window)) {
        frontendUI->newFrame();
        stopMouse = frontendUI->getStopMouse();
        stopKeyboard = frontendUI->getStopKeyboard();
        glfwPollEvents();

        frontendUI->composeUI();

        // Render scene
        currentFrame = glfwGetTime(); // seconds
        double deltaTime = currentFrame - lastFrame;

        if (processor) {
            if (!processor->getThreaded()) {
                processor->processRequests(false);
            }
            processor->processResults(10);
        }
//        if (windowSizeChanged) {
//            scene->setScreenSize(canvWidth, canvHeight);
//            windowSizeChanged = false;
//        }

//        scene->draw((deltaTime*(1000.0f))); //miliseconds

        apiContainer.camera->tick(deltaTime*(1000.0f));
        float farPlaneRendering = apiContainer.getConfig()->getFarPlane();
        float farPlaneCulling = apiContainer.getConfig()->getFarPlaneForCulling();

        float nearPlane = 1.0;
        float fov = toRadian(45.0);

        float canvasAspect = (float)canvWidth / (float)canvHeight;


        auto cameraMatricesCulling = apiContainer.camera->getCameraMatrices(fov, canvasAspect, nearPlane, farPlaneCulling);
        auto cameraMatricesRendering = apiContainer.camera->getCameraMatrices(fov, canvasAspect, nearPlane, farPlaneRendering);
        //Frustum matrix with reversed Z
        {
            float f = 1.0f / tan(fov / 2.0f);
            cameraMatricesRendering->perspectiveMat = mathfu::mat4(
                f / canvasAspect, 0.0f,  0.0f,  0.0f,
                0.0f,    f,  0.0f,  0.0f,
                0.0f, 0.0f,  1, -1.0f,
                0.0f, 0.0f, 1,  0.0f);
        }

        if (hdevice->getIsVulkanAxisSystem() ) {
            auto &perspectiveMatrix = cameraMatricesRendering->perspectiveMat;

            static const mathfu::mat4 vulkanMatrixFix2 = mathfu::mat4(1, 0, 0, 0,
                                                                      0, -1, 0, 0,
                                                                      0, 0, 1.0/2.0, 1/2.0,
                                                                      0, 0, 0, 1).Transpose();

            perspectiveMatrix = vulkanMatrixFix2 * perspectiveMatrix;
        }


        HFrameScenario sceneScenario = std::make_shared<FrameScenario>();

        std::vector<HDrawStage> uiDependecies = {};
        bool clearOnUi = true;
        auto clearColor = apiContainer.getConfig()->getClearColor();

        if (currentScene != nullptr) {
            auto cullStage = sceneScenario->addCullStage(cameraMatricesCulling, currentScene);
            auto updateStage = sceneScenario->addUpdateStage(cullStage, deltaTime*(1000.0f), cameraMatricesRendering);
            auto sceneDrawStage = sceneScenario->addDrawStage(updateStage, currentScene, cameraMatricesRendering, {}, true,
                {{0, 0}, {canvWidth, canvHeight}},
                true, clearColor);

            clearOnUi = false;
            uiDependecies.push_back(sceneDrawStage);
        }

        auto uiCullStage = sceneScenario->addCullStage(nullptr, frontendUI);
        auto uiUpdateStage = sceneScenario->addUpdateStage(uiCullStage, deltaTime*(1000.0f), nullptr);
        auto frontUIDrawStage = sceneScenario->addDrawStage(uiUpdateStage, frontendUI, nullptr, uiDependecies, true, {
            {0,0}, {canvWidth, canvHeight}
        }, clearOnUi, clearColor);


//        auto updateResult = scene->cull(camera)->update(camera);
//        SceneComposer::All({
//            updateResult->render(camera)->toFB(frameBuffer, viewPortDims),
//            updateResult->render(cameraDebug)->toFB(frameBuffer, viewPortDims);
//        }).then([]{
//            frontendUI->bind("", frameBuffer.getTexture())
//            SceneComposer.renderToScreen()
//        });

        sceneComposer.draw(sceneScenario);

        double currentDeltaAfterDraw = (glfwGetTime() - lastFrame)*(1000.0f);
        lastFrame = currentFrame;
        if (currentDeltaAfterDraw < 5.0) {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(std::chrono::milliseconds((int)(5.0 - currentDeltaAfterDraw)));
        }

        if (rendererName == "ogl3" || rendererName == "ogl2") {
            glfwSwapBuffers(window);
        }
    }
//} catch(const std::exception &e){
//    std::cerr << e.what() << std::endl;
//    throw;
//} catch(...) {
//    std::cout << "something happened" << std::endl;
//}

    std::cout << "program ended" << std::endl;
    //        while (1) {
    //            mainLoop(&myapp);
    //        }

    return 0;
}
