#include "../../wowViewerLib/src/gapi/IDeviceFactory.h"
#include "RequestProcessor.h"
#include "HttpRequestProcessor.h"
#include "../../wowViewerLib/src/engine/ApiContainer.h"
#include "../../wowViewerLib/src/engine/camera/firstPersonCamera.h"
#include "../../wowViewerLib/src/engine/SceneComposer.h"
#include "../../wowViewerLib/src/engine/objects/scenes/NullScene.h"
#include "../../wowViewerLib/src/engine/camera/planarCamera.h"
#include "../../wowViewerLib/src/engine/objects/scenes/m2Scene.h"
#include "../../wowViewerLib/src/engine/objects/scenes/wmoScene.h"
#include "../../wowViewerLib/src/engine/objects/scenes/map.h"
#include "../../src/exporters/gltfExporter/GLTFExporter.h"
#include "../../3rdparty/filesystem_impl/include/ghc/filesystem.hpp"
#include <iostream>
#include <memory>
#include <emscripten/threading.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/bind.h>
#include <GLFW/glfw3.h>
#include <zipper/zipper.h>

std::shared_ptr<ApiContainer> apiContainer = std::make_shared<ApiContainer>();
HttpRequestProcessor *processor;
SceneComposer *sceneComposer;
std::shared_ptr<IScene> currentScene = std::make_shared<NullScene>();

std::shared_ptr<ICamera> planarCamera = std::make_shared<PlanarCamera>();
std::shared_ptr<ICamera> firstPersonCamera = std::make_shared<FirstPersonCamera>();

std::shared_ptr<IExporter> exporter = nullptr;
int exporterFramesReady = 0;

int canvWidth = 640;
int canvHeight = 480;
bool windowSizeChanged = false;

int mleft_pressed = 0;
int mright_pressed = 0;
double m_x = 0.0;
double m_y = 0.0;

GLFWwindow * window;

template <int T>
float getFloatFromInt(int value) {
    if (T == 0) {
        return (value & 0xFF) / 255.0f;
    }
    if (T == 1) {
        return ((value >> 8) & 0xFF) / 255.0f;
    }
    if (T == 2) {
        return ((value >> 16) & 0xFF) / 255.0f;
    }
    if (T == 3) {
        return ((value >> 24) & 0xFF) / 255.0f;
    }
}
void convertColors(int index, int start, int mid, int end, std::array<std::array<mathfu::vec4, 3>, 3> &particleColorReplacement) {
    particleColorReplacement[index][0] = mathfu::vec4(
        getFloatFromInt<2>(start),
        getFloatFromInt<1>(start),
        getFloatFromInt<0>(start),
        getFloatFromInt<3>(start)
    );
    particleColorReplacement[index][1] = mathfu::vec4(
        getFloatFromInt<2>(mid),
        getFloatFromInt<1>(mid),
        getFloatFromInt<0>(mid),
        getFloatFromInt<3>(mid)
    );
    particleColorReplacement[index][2] = mathfu::vec4(
        getFloatFromInt<2>(end),
        getFloatFromInt<1>(end),
        getFloatFromInt<0>(end),
        getFloatFromInt<3>(end)
    );
}


static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos){
    auto controllable = apiContainer->camera;

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
            case GLFW_KEY_LEFT_SHIFT:
                controllable->setMovementSpeed(3.0f);
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
                break;
            case 'J':
//                testConf->setDoubleCameraDebug(!testConf->getDoubleCameraDebug());
                break;
            case 'K':
//                testConf->setRenderPortals(!testConf->getRenderPortals());
                break;

            case 'O':
                apiContainer->camera->setCameraPos(0,0,0);
                break;
            case GLFW_KEY_LEFT_SHIFT:
                controllable->setMovementSpeed(1.0f);
                break;
            default:
                break;
        }
    }

}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    auto controllable = apiContainer->camera;

    controllable->zoomInFromMouseScroll(yoffset/240.0f);
}


void window_size_callback(GLFWwindow* window, int width, int height)
{
    canvWidth = width;
    canvHeight = height;
    windowSizeChanged = true;
}

void setScene(int sceneType, std::string name, int cameraNum) {
    if (sceneType == -1) {
        currentScene = std::make_shared<NullScene>();
    } else if (sceneType == 0) {
//        m_usePlanarCamera = cameraNum == -1;
        apiContainer->camera = planarCamera;

        currentScene = std::make_shared<M2Scene>(apiContainer, name , cameraNum);
    } else if (sceneType == 1) {
        apiContainer->camera = firstPersonCamera;

        currentScene = std::make_shared<WmoScene>(apiContainer, name);
    } else if (sceneType == 2) {
        apiContainer->camera = firstPersonCamera;

        std::string &adtFileName = name;

        size_t lastSlashPos = adtFileName.find_last_of("/");
        size_t underscorePosFirst = adtFileName.find_last_of("_");
        size_t underscorePosSecond = adtFileName.find_last_of("_", underscorePosFirst-1);
        std::string mapName = adtFileName.substr(lastSlashPos+1, underscorePosSecond-lastSlashPos-1);

        int i = std::stoi(adtFileName.substr(underscorePosSecond+1, underscorePosFirst-underscorePosSecond-1));
        int j = std::stoi(adtFileName.substr(underscorePosFirst+1, adtFileName.size()-underscorePosFirst-5));

        float adt_x_min = AdtIndexToWorldCoordinate(j);
        float adt_x_max = AdtIndexToWorldCoordinate(j+1);

        float adt_y_min = AdtIndexToWorldCoordinate(i);
        float adt_y_max = AdtIndexToWorldCoordinate(i+1);

        apiContainer->camera->setCameraPos(
            (adt_x_min+adt_x_max) / 2.0,
            (adt_y_min+adt_y_max) / 2.0,
            200
        );

        currentScene = std::make_shared<Map>(apiContainer, adtFileName, i, j, mapName);
    }
}

extern "C" {
    EMSCRIPTEN_KEEPALIVE
    void createWebJsScene(int p_canvWidth, int p_canvHeight, char *url, char *urlFileId) {
        auto temp_gc = new tbb::global_control(tbb::global_control::max_allowed_parallelism, 1);

        if (!glfwInit()) {
            fputs("Failed to initialize GLFW", stderr);
            return;
        }
        canvWidth = p_canvWidth;
        canvHeight = p_canvHeight;


        apiContainer->camera = firstPersonCamera;
        apiContainer->getConfig()->globalLighting = EParameterSource::eConfig;

        auto ambient = mathfu::vec4(1.0,1.0,1.0,1.0);

        apiContainer->getConfig()->exteriorAmbientColor = mathfu::vec4(ambient.x, ambient.y, ambient.z, 1.0);
        apiContainer->getConfig()->exteriorHorizontAmbientColor = mathfu::vec4(ambient.x, ambient.y, ambient.z, 1.0);
        apiContainer->getConfig()->exteriorGroundAmbientColor = mathfu::vec4(ambient.x, ambient.y, ambient.z, 1.0);
        apiContainer->getConfig()->exteriorDirectColor = mathfu::vec4(0.3,0.3,0.3,0.3);
        apiContainer->getConfig()->exteriorDirectColorDir = mathfu::vec3(0.0,0.0,0.0);

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
        apiContainer->cacheStorage = std::make_shared<WoWFilesCacheStorage>(processor);
        processor->setFileRequester(apiContainer->cacheStorage.get());

        //        testConf = new Config();
//        testConf->setSunColor(0.0,0.0,0.0,0.0);
//        testConf->setAmbientColor(1.0,1.0,1.0,1.0);
//        testConf->setRenderM2(true);
//        testConf->setRenderWMO(true);

        EmscriptenWebGLContextAttributes contextAttributes;
        emscripten_webgl_get_context_attributes(emscripten_webgl_get_current_context(), &contextAttributes);

        //TODO: Check max uniform vectors
        std::string glVersion = "ogl2";
        GLint uboSize = 0;
        if ((contextAttributes.majorVersion >= 2) ) {
//            this will be 0 for Apples
            glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &uboSize);
            if (uboSize > 0) {
                glVersion = "ogl3";
            }

        }

        auto hdevice = IDeviceFactory::createDevice(glVersion, nullptr);

        apiContainer->databaseHandler = nullptr;
        apiContainer->hDevice = hdevice;
        apiContainer->camera = std::make_shared<FirstPersonCamera>();

        sceneComposer = new SceneComposer(apiContainer);

        glfwSetWindowUserPointer(window, nullptr);
        glfwSetKeyCallback(window, onKey);
        glfwSetScrollCallback(window, scroll_callback);
        glfwSetCursorPosCallback(window, cursor_position_callback);
        glfwSetWindowSizeCallback(window, window_size_callback);
        glfwSetMouseButtonCallback(window, mouse_button_callback);
    }

    EMSCRIPTEN_KEEPALIVE
    void startExport() {
        exporter = std::make_shared<GLTFExporter>("./gltf/");
        currentScene->exportScene(exporter.get());
        exporterFramesReady = 0;
    }

    EMSCRIPTEN_KEEPALIVE
    void setNewUrls(char *url, char *urlFileId) {
        processor->setUrls(url, urlFileId);
        apiContainer->cacheStorage->actuallDropCache();
    }
    EMSCRIPTEN_KEEPALIVE
    void setScene(int sceneType, char *name, int cameraNum) {
        setScene(sceneType, std::string(name), cameraNum);
    }
    EMSCRIPTEN_KEEPALIVE
    void setMap(int mapId, int wdtFileId, float x, float y, float z) {
        apiContainer->camera = firstPersonCamera;

        apiContainer->camera->setCameraPos(
            x,
            y,
            z
        );

        currentScene = std::make_shared<Map>(apiContainer, mapId, wdtFileId);
    }
    EMSCRIPTEN_KEEPALIVE
    void setSceneFileDataId(int sceneType, int fileDataId, int cameraNum) {
        if (sceneType == -1) {
            currentScene = std::make_shared<NullScene>();
        } else if (sceneType == 0) {
//        m_usePlanarCamera = cameraNum == -1;
            apiContainer->camera = planarCamera;

            currentScene = std::make_shared<M2Scene>(apiContainer, fileDataId , cameraNum);
        } else if (sceneType == 1) {
            apiContainer->camera = firstPersonCamera;

            currentScene = std::make_shared<WmoScene>(apiContainer, fileDataId);
        }
    }
    EMSCRIPTEN_KEEPALIVE
    void setSceneSize(int width, int height) {
        canvWidth = width;
        canvHeight = height;
//        scene->setScreenSize(canvWidth, canvHeight);
    }
    EMSCRIPTEN_KEEPALIVE
    void addHorizontalViewDir(float val) {
        auto controllable = apiContainer->camera;
        controllable->addHorizontalViewDir(val);
    }
    EMSCRIPTEN_KEEPALIVE
    void addVerticalViewDir(float val ) {
        auto controllable = apiContainer->camera;
        controllable->addVerticalViewDir(val);
    }
    EMSCRIPTEN_KEEPALIVE
    void zoomInFromMouseScroll(float val ) {
        auto controllable = apiContainer->camera;
        controllable->zoomInFromMouseScroll(val);
    }
    EMSCRIPTEN_KEEPALIVE
    void addCameraViewOffset(float val1, float val2 ) {
        auto controllable = apiContainer->camera;
        controllable->addCameraViewOffset(val1, val2);
    }
    EMSCRIPTEN_KEEPALIVE
    void startMovingForward() {
        auto controllable = apiContainer->camera;
        controllable->startMovingForward();
    }
    EMSCRIPTEN_KEEPALIVE
    void startMovingBackwards() {
        auto controllable = apiContainer->camera;
        controllable->startMovingBackwards();
    }
    EMSCRIPTEN_KEEPALIVE
    void stopMovingForward() {
        auto controllable = apiContainer->camera;
        controllable->stopMovingForward();
    }
    EMSCRIPTEN_KEEPALIVE
    void stopMovingBackwards() {
        auto controllable = apiContainer->camera;
        controllable->stopMovingBackwards();
    }

    EMSCRIPTEN_KEEPALIVE
    void setClearColor(float r, float g, float b) {
        apiContainer->getConfig()->clearColor = mathfu::vec4(r,g,b,0.0);
    }
    EMSCRIPTEN_KEEPALIVE
    void enablePortalCulling(bool value) {
        apiContainer->getConfig()->usePortalCulling = value;
    }
    EMSCRIPTEN_KEEPALIVE
    void setFarPlane(float value) {
        apiContainer->getConfig()->farPlane = value;
    }
    EMSCRIPTEN_KEEPALIVE
    void setFarPlaneForCulling(float value) {
        apiContainer->getConfig()->farPlaneForCulling = value;
    }

    EMSCRIPTEN_KEEPALIVE
    void setAnimationId(int animationId) {
        if (currentScene != nullptr) {
            currentScene->setAnimationId(animationId);
        }
    }

    EMSCRIPTEN_KEEPALIVE
    void setMeshIdArray(uint8_t *meshIds, int length) {
        if (currentScene != nullptr) {
            auto meshIdsVec = std::vector<uint8_t>(meshIds, meshIds+length);
            currentScene->setMeshIds(meshIdsVec);
        }
    }

    EMSCRIPTEN_KEEPALIVE
    void setReplaceParticleColors(
        int start0, int start1, int start2,
        int mid0, int mid1, int mid2,
        int end0, int end1, int end2
    ) {
        std::array<std::array<mathfu::vec4, 3>, 3> particleColorReplacement;
        //0
        convertColors(0, start0, mid0, end0, particleColorReplacement);
        convertColors(1, start1, mid1, end1, particleColorReplacement);
        convertColors(2, start2, mid2, end2, particleColorReplacement);
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                std::cout << "particleColorReplacement["<<i<<"]["<<j<<"] = "
                << "{ "
                << particleColorReplacement[i][j][0] << ", "
                << particleColorReplacement[i][j][1] << ", "
                << particleColorReplacement[i][j][2]
                << "};" << std::endl;
            }
        }

        currentScene->setReplaceParticleColors(particleColorReplacement);
    }
    EMSCRIPTEN_KEEPALIVE
    void resetReplaceParticleColor() {
        currentScene->resetReplaceParticleColor();
    }

    EMSCRIPTEN_KEEPALIVE
    void setTextures(int *fileDataIdsBuff, int elementsCount) {
        std::vector<int> textures(fileDataIdsBuff, fileDataIdsBuff+elementsCount);

//        std::cout << "called! elementsCount = " << elementsCount << std::endl;
        if (currentScene != nullptr) {
            currentScene->setReplaceTextureArray(textures);
        }
    }

    EMSCRIPTEN_KEEPALIVE
    void setScenePos(float x, float y, float z) {
        auto controllable = apiContainer->camera;
        controllable->setCameraPos(x,y,z);
    }
    extern void offerFileAsDownload(const char *filename, int filename_len);

    EMSCRIPTEN_KEEPALIVE
    void gameloop(double deltaTime) {


        if (sceneComposer == nullptr) return;
        if (apiContainer->hDevice == nullptr) return;

        if (exporter != nullptr && processor != nullptr) {
            if (processor->completedAllJobs() && !apiContainer->hDevice->wasTexturesUploaded()) {
                exporterFramesReady++;
            }
            if (exporterFramesReady > 5) {
                std::string exportDir = "./gltf/";
                if (!ghc::filesystem::is_directory(exportDir) || !ghc::filesystem::exists(exportDir)) {
                    ghc::filesystem::create_directory(exportDir); // create src folder
                }
                exporter->saveToFile("model.gltf");
                exporter = nullptr;

                std::error_code errorCode;

                //Add files to archive and download it
                std::string archiveName = "export.zip";
                ghc::filesystem::remove_all(archiveName, errorCode); //Delete file if exists

                zipper::Zipper zipper(archiveName);

                for (const auto& entry : ghc::filesystem::directory_iterator(exportDir)) {
                    const auto filenameStr = entry.path().filename().string();
                    if (entry.status().type() == ghc::filesystem::file_type::regular) {
                        std::ifstream input1(exportDir+filenameStr);


                        zipper.add(input1, filenameStr);
                    }
                }
                zipper.close();
                ghc::filesystem::remove_all(exportDir, errorCode); // Deletes one or more files recursively.

                offerFileAsDownload(archiveName.c_str(), archiveName.size());

            }
        }

        glfwPollEvents();

        processor->processRequests(false);

        apiContainer->camera->tick(deltaTime*(1000.0f));

        float farPlaneRendering = apiContainer->getConfig()->farPlane;
        float farPlaneCulling = apiContainer->getConfig()->farPlaneForCulling;

        float nearPlane = 1.0 ;
        float fov = toRadian(45.0);

        float canvasAspect = (float)canvWidth / (float)canvHeight;

        auto cameraMatricesCulling = apiContainer->camera->getCameraMatrices(fov, canvasAspect, nearPlane, farPlaneCulling);
        auto cameraMatricesRendering = apiContainer->camera->getCameraMatrices(fov, canvasAspect, nearPlane, farPlaneRendering);

        bool isInfZSupported = apiContainer->camera->isCompatibleWithInfiniteZ();
        if (isInfZSupported)
        {
            float f = 1.0f / tan(fov / 2.0f);
            cameraMatricesRendering->perspectiveMat = mathfu::mat4(
                f / canvasAspect, 0.0f,  0.0f,  0.0f,
                0.0f,    f,  0.0f,  0.0f,
                0.0f, 0.0f,  1, -1.0f,
                0.0f, 0.0f, 1,  0.0f);
        }

        HFrameScenario sceneScenario = std::make_shared<FrameScenario>();

        bool clearOnUi = true;
        auto clearColor = apiContainer->getConfig()->clearColor;

        auto cullStage = sceneScenario->addCullStage(cameraMatricesCulling, currentScene);
        auto updateStage = sceneScenario->addUpdateStage(cullStage, deltaTime*(1000.0f), cameraMatricesRendering);
        ViewPortDimensions dimensions = {{0, 0}, {canvWidth, canvHeight}};
        auto sceneDrawStage = sceneScenario->addDrawStage(updateStage, currentScene, cameraMatricesRendering, {}, true,
                                    dimensions, true, isInfZSupported, clearColor, nullptr);




//        try {
            sceneComposer->draw(sceneScenario);
//        } catch (const std::exception &e) {
//                std::cerr << e.what() << std::endl;
//        }
        glfwSwapBuffers(window);


//        std::cout << "testConf->getRenderWMO() " << testConf->getRenderWMO() << std::endl;
    }
}




int main() {

}