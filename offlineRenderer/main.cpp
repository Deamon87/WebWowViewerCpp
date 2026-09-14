#include <memory>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cassert>
#include <thread>
#include <chrono>

#include "../src/persistance/CascRequestProcessor.h"
#include "../src/database/CSqliteDB.h"
#include "../wowViewerLib/src/engine/WowFilesCacheStorage.h"
#include "../src/database/buildInfoParser/buildDefinition.h"
#include "../src/ui/childWindow/sceneWindow/SceneWindow.h"
#include "../wowViewerLib/src/gapi/IDeviceFactory.h"
#include "../wowViewerLib/src/renderer/frame/SceneComposer.h"
#include "../wowViewerLib/src/gapi/vulkan/GDeviceVulkan.h"
#include "../wowViewerLib/src/gapi/renderdoc_app.h"
#include "../wowViewerLib/src/include/config.h"
#include "../wowViewerLib/src/gapi/interface/IDevice.h"
#include "gapi/interface/FrameContext.h"

#ifdef _WIN32
    #include <errhandlingapi.h>
    #include <windows.h>
#endif

extern bool forceDisableBindlessSupport;

struct RenderConfig {
    std::string cascPath = "";
    std::string outputPath = "output.png";
    std::string gapiName = "vulkan";
    bool supportsBindless = false;
    bool enableValidation = false;
    int mapId = -1;
    int wdtFileId = -1;
    int wmoFileId = -1;
    int m2FileId = -1;
    mathfu::vec3 cameraPos = {0, 0, 0};
    mathfu::vec3 lookAt = {0, 0, 0};
    int timeOverride = 0;
    unsigned int width = 1024;
    unsigned int height = 1024;
    
    // Video recording options
    bool recordVideo = false;
    std::string videoOutputPath = "output.mp4";
    int fps = 30;
    float videoDuration = 5.0f; // seconds
    int cameraIndex = -1; // -1 means use manual camera
    unsigned int videoFramebufferWidth = 0;  // 0 means use video output width
    unsigned int videoFramebufferHeight = 0; // 0 means use video output height
};

void initRenderDoc() {
#ifdef _WIN32
    // At init, on windows
    {
        rdoc_api = nullptr;
        HMODULE mod = LoadLibraryA("renderdoc.dll");
        if (mod)
        {
            pRENDERDOC_GetAPI RENDERDOC_GetAPI =
                (pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");
            int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_2, (void **)&rdoc_api);
            assert(ret == 1);
        }
    }
    if (rdoc_api && false) {
        int secondsToWaitMax = 15;
        std::cout << "Waiting for RenderDoc to within 15 sec" << std::endl;
        while (!rdoc_api->IsRemoteAccessConnected() && secondsToWaitMax > 0) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            secondsToWaitMax--;
        }

        if (!rdoc_api->IsRemoteAccessConnected()) {
            std::cout << "RenderDoc failed to Connect within timeout" << std::endl;
        } else {
            std::cout << "RenderDoc connected!" << std::endl;
        }
    }
#endif
}

class OfflineRenderer {
private:
    std::unique_ptr<SceneComposer> sceneComposer;
    HApiContainer apiContainer = nullptr;
    std::shared_ptr<SceneWindow> sceneWindow = nullptr;
    RenderConfig config;

public:
    OfflineRenderer(RenderConfig cfg) : config(cfg) {}

    ~OfflineRenderer() {
        tearDown();
    }

    void tearDown() {
        auto device = apiContainer ? apiContainer->hDevice : nullptr;

        sceneComposer = nullptr;
        apiContainer = nullptr;
        sceneWindow = nullptr;

        if (device) {
            device->clear();
        }
    }

    void setup() {
        initRenderDoc();

#ifdef LINK_VULKAN
        vkCallInitCallback callback;
        callback.createSurface = [&](VkInstance vkInstance) -> VkSurfaceKHR {
            return VK_NULL_HANDLE;
        };
        callback.getRequiredExtensions = [](char** &extensionNames, int &extensionCnt) {
            extensionNames = nullptr;
            extensionCnt = 0;
        };
#else
        void *callback = nullptr;
#endif

        vulkanEnableValidationLayers = config.enableValidation;
        forceDisableBindlessSupport = !config.supportsBindless;

        apiContainer = std::make_shared<ApiContainer>();

        //1. Create filesystem (CASC for now)
        BuildDefinition buildDef;
        
        std::string cascPath = config.cascPath;
        if (cascPath.empty()) {
            const char* envPath = std::getenv("CASC_PATH");
            if (envPath) {
                cascPath = envPath;
            } else {
                std::cerr << "Error: CASC path not provided. Use -casc <path> or set CASC_PATH environment variable." << std::endl;
                exit(1);
            }
        }
        
        apiContainer->requestProcessor = std::make_shared<CascRequestProcessor>(cascPath, buildDef);
        apiContainer->cacheStorage = std::make_shared<WoWFilesCacheStorage>(apiContainer->requestProcessor.get());

        //2. Create database
        apiContainer->databaseHandler = std::make_shared<CSqliteDB>("./export.db3");

        //3. Create device
        apiContainer->hDevice = IDeviceFactory::createDevice(config.gapiName, &callback);

        //4. Create composer
        sceneComposer = std::make_unique<SceneComposer>(apiContainer, false);

        //5. Create sceneWindow
        sceneWindow = std::make_shared<SceneWindow>(apiContainer, false, nullptr);
        sceneWindow->setViewPortDimensions({{0,0}, {config.width, config.height}});
    }

    bool isFrameLoaded(std::shared_ptr<MapRenderPlan> lastPlan) {
        return !(
            !apiContainer->requestProcessor->completedAllJobs() ||
            !lastPlan ||
            !lastPlan->wmoArray.getToLoad().empty() ||
            !lastPlan->wmoGroupArray.getToLoad().empty() ||
            !lastPlan->m2Array.getToLoadGeom().empty() ||
            !lastPlan->m2Array.getToLoadMain().empty()
        );
    }

    void openScene() {
        if (config.mapId >= 0 && config.wdtFileId >= 0) {
            sceneWindow->openMapByIdAndWDTId(config.mapId, config.wdtFileId, 
                config.cameraPos.x, config.cameraPos.y, config.cameraPos.z, config.timeOverride);
            sceneWindow->getCamera()->setCameraLookAt(config.lookAt.x, config.lookAt.y, config.lookAt.z);
        } else if (config.wmoFileId > 0) {
            sceneWindow->openWMOSceneByfdid(config.wmoFileId);
            sceneWindow->getCamera()->setCameraPos(config.cameraPos.x, config.cameraPos.y, config.cameraPos.z);
            sceneWindow->getCamera()->setCameraLookAt(config.lookAt.x, config.lookAt.y, config.lookAt.z);
        } else if (config.m2FileId > 0) {
            sceneWindow->openM2SceneByfdid(config.m2FileId, {});
            sceneWindow->getCamera()->setCameraPos(config.cameraPos.x, config.cameraPos.y, config.cameraPos.z);
            sceneWindow->getCamera()->setCameraLookAt(config.lookAt.x, config.lookAt.y, config.lookAt.z);
        } else {
            std::cerr << "Error: Either map (mapId and wdtFileId) or wmoFileId or m2FileId must be specified." << std::endl;
            exit(1);
        }
    }
    
    void setupCamera() {
        // Wait for scene to load before setting camera
        auto plan = sceneWindow->getLastPlan();
        int waitFrames = 100;
        while (!isFrameLoaded(plan) && waitFrames-- > 0) {
            auto processingFrame = apiContainer->hDevice->getFrameNumber();
            std::function<uint32_t()> updateFrameNumberLambda = [this, frame = processingFrame]() -> uint32_t {
                FrameContext::setCurrentProcessingFrameNumber(frame);
                return frame;
            };
            HFrameScenario scenario = std::make_shared<HFrameScenario::element_type>();
            sceneWindow->render(0, DEFAULT_FOV_VALUE, scenario, nullptr, updateFrameNumberLambda);
            sceneComposer->draw(scenario, false);
            plan = sceneWindow->getLastPlan();
        }
        
        if (config.cameraIndex >= 0) {
            int cameraCount = sceneWindow->getCurrentCameraCount();
            std::cout << "Available cameras: " << cameraCount << std::endl;
            
            if (config.cameraIndex < cameraCount) {
                std::cout << "Setting camera index to: " << config.cameraIndex << std::endl;
                sceneWindow->setCurrentCameraIndex(config.cameraIndex);
            } else {
                std::cerr << "Warning: Camera index " << config.cameraIndex 
                          << " out of range (0-" << (cameraCount - 1) << "). Using default camera." << std::endl;
            }
        }
    }

    void processScreenshot() {
        // Create output directory if needed
        auto outputPath = std::filesystem::path(config.outputPath);
        auto folderPath = outputPath.parent_path();
        if (!folderPath.empty()) {
            std::filesystem::create_directories(folderPath);
        }

        std::string pngFileName = std::filesystem::absolute(outputPath).string();

        auto l_device = apiContainer->hDevice;


        int framesToContinue = IDevice::MAX_FRAMES_IN_FLIGHT + 1;
        bool makeScreenshot = false;
        
        while (framesToContinue-- > 0) {
            bool captureWithRenderDoc = false;
            auto processingFrame = l_device->getFrameNumber();
            std::function<uint32_t()> updateFrameNumberLambda = [l_device, frame = processingFrame]() -> uint32_t {
                FrameContext::setCurrentProcessingFrameNumber(frame);
                return frame;
            };

            HFrameScenario scenario = std::make_shared<HFrameScenario::element_type>();

            {
                auto plan = sceneWindow->getLastPlan();
                if (framesToContinue == 0 && !makeScreenshot && isFrameLoaded(plan)) {
                    std::cout << "Capturing screenshot to: " << pngFileName << std::endl;

                    sceneWindow->makeScreenshot(
                        DEFAULT_FOV_VALUE,
                        config.width, config.height,
                        pngFileName,
                        scenario, updateFrameNumberLambda
                    );
                    framesToContinue = IDevice::MAX_FRAMES_IN_FLIGHT + 1;
                    makeScreenshot = true;
                    captureWithRenderDoc = true;
                } else {
                    sceneWindow->render(0, DEFAULT_FOV_VALUE, scenario, nullptr, updateFrameNumberLambda);
                }
            }

            std::shared_ptr<IRenderDocCaptureHandler> captureHandler = nullptr;
            if (captureWithRenderDoc && rdoc_api)
                captureHandler = apiContainer->hDevice->getRenderDocHelper();

            sceneComposer->draw(scenario, false);

            auto lastPlan = sceneWindow->getLastPlan();
            if (!isFrameLoaded(lastPlan)) {
                framesToContinue = IDevice::MAX_FRAMES_IN_FLIGHT + 1;
            }

            captureHandler = nullptr;
        }

        l_device->waitForAllWorkToComplete();
        std::cout << "Rendering complete!" << std::endl;
    }
    
    void processVideoRecording() {
        auto l_device = apiContainer->hDevice;
        
        // Create output directory if needed
        auto outputPath = std::filesystem::path(config.videoOutputPath);
        auto folderPath = outputPath.parent_path();
        if (!folderPath.empty()) {
            std::filesystem::create_directories(folderPath);
        }
        
        std::string videoFileName = std::filesystem::absolute(outputPath).string();
        std::cout << "Starting video recording to: " << videoFileName << std::endl;
        
        // Determine framebuffer size (default to video output size if not specified)
        uint32_t framebufferWidth = config.videoFramebufferWidth > 0 ? config.videoFramebufferWidth : config.width;
        uint32_t framebufferHeight = config.videoFramebufferHeight > 0 ? config.videoFramebufferHeight : config.height;
        
        if (framebufferWidth != config.width || framebufferHeight != config.height) {
            std::cout << "Using framebuffer size: " << framebufferWidth << "x" << framebufferHeight 
                      << " -> output size: " << config.width << "x" << config.height << std::endl;
        }
        
        // Start video recording with separate framebuffer and output sizes
        auto videoContext = sceneWindow->startVideoRecording(framebufferWidth, framebufferHeight, config.width, config.height, videoFileName);
        
        float deltaTime = 1.0f / config.fps; // seconds per frame
        int totalFrames = static_cast<int>(config.videoDuration * config.fps);
        int currentFrame = 0;
        
        std::cout << "Recording " << totalFrames << " frames at " << config.fps << " FPS..." << std::endl;
        
        auto renderDummyFrame = [&](animTime_t deltaTime) {
            auto scenario = std::make_shared<HFrameScenario::element_type>();
            auto processingFrame = l_device->getFrameNumber();
            auto updateFrameNumberLambda = [l_device, frame = processingFrame]() -> uint32_t {
                FrameContext::setCurrentProcessingFrameNumber(frame);
                return frame;
            };
            sceneWindow->render(deltaTime, DEFAULT_FOV_VALUE, scenario, nullptr, updateFrameNumberLambda);
            sceneComposer->draw(scenario, false);
        };

        //To skip first frame, where picture may not be what we want
        renderDummyFrame(0);
        
        while (currentFrame < totalFrames) {
            auto processingFrame = l_device->getFrameNumber();
            std::function<uint32_t()> updateFrameNumberLambda = [l_device, frame = processingFrame]() -> uint32_t {
                FrameContext::setCurrentProcessingFrameNumber(frame);
                return frame;
            };
            
            HFrameScenario scenario = std::make_shared<HFrameScenario::element_type>();
            
            auto plan = sceneWindow->getLastPlan();
            
            // Only process video frame when scene is fully loaded
            if (isFrameLoaded(plan)) {
                std::cout << "Recording frame " << (currentFrame + 1) << "/" << totalFrames << std::endl;
                
                // Process video recording for this frame
                sceneWindow->processVideoRecording(videoContext, scenario, DEFAULT_FOV_VALUE, updateFrameNumberLambda);
                sceneComposer->draw(scenario, false);
                
                // Advance to next frame
                currentFrame++;
                
                // Render next frame with time advancement
                if (currentFrame < totalFrames) {
                    renderDummyFrame(deltaTime);
                }
            } else {
                sceneComposer->draw(scenario, false);
            }
        }
        
        l_device->waitForAllWorkToComplete();
        std::cout << "Video recording complete!" << std::endl;
    }
    
    void process() {
        if (config.recordVideo) {
            processVideoRecording();
        } else {
            processScreenshot();
        }
    }
};

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [options]\n"
              << "Options:\n"
              << "  -casc <path>               CASC filesystem path (or set CASC_PATH env var)\n"
              << "                             Paths with spaces should be quoted: -casc \"C:\\Program Files\\World of Warcraft\"\n"
              << "  -output <path>             Output PNG file path (default: output.png)\n"
              << "  -map <mapId> <wdtFileId>   Render a map with given mapId and wdtFileId\n"
              << "  -wmo <wmoFileId>           Render a WMO with given fileId\n"
              << "  -m2 <m2FileId>             Render a M2 with given fileId\n"
              << "  -camera <x> <y> <z>        Camera position (default: 0 0 0)\n"
              << "  -lookat <x> <y> <z>        Camera look-at point (default: 0 0 0)\n"
              << "  -cameraIndex <index>       Use predefined camera by index (m2 scenes)\n"
              << "  -time <time>               Time override (default: 0)\n"
              << "  -size <width> <height>     Output image size (default: 1024 1024)\n"
              << "  -gapi <name>               Graphics API name (default: vulkan)\n"
              << "  -bindless                  Enable bindless support\n"
              << "  -validation                Enable validation layers\n"
              << "\n"
              << "Video Recording Options:\n"
              << "  -video                     Enable video recording mode\n"
              << "  -videoOutput <path>        Output video file path (default: output.mp4)\n"
              << "  -fps <fps>                 Video framerate (default: 30)\n"
              << "  -duration <seconds>        Video duration in seconds (default: 5.0)\n"
              << "  -videoFramebuffer <w> <h>  Framebuffer size for video (default: same as output size)\n"
              << "\n"
              << "  -h, --help                 Show this help message\n"
              << "\n"
              << "Examples:\n"
              << "  Screenshot:\n"
              << "    " << programName << " -casc /path/to/wow -map 530 828395 -camera 9305.81 -7442.45 236.1 -lookat 9304.83 -7442.5 235.909 -output result.png\n"
              << "  Video recording:\n"
              << "    " << programName << " -casc /path/to/wow -wmo 123456 -video -videoOutput video.mp4 -fps 60 -duration 10 -cameraIndex 0\n"
              << "  Video recording with custom framebuffer size (render at 4K, output at 1080p):\n"
              << "    " << programName << " -casc /path/to/wow -wmo 123456 -video -size 1920 1080 -videoFramebuffer 3840 2160 -videoOutput video.mp4 -fps 30 -duration 5\n"
              << std::endl;
}

RenderConfig parseArguments(int argc, char* argv[]) {
    RenderConfig config;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "-casc" && i + 1 < argc) {
            config.cascPath = argv[++i];
        } else if (arg == "-output" && i + 1 < argc) {
            config.outputPath = argv[++i];
        } else if (arg == "-map" && i + 2 < argc) {
            config.mapId = std::stoi(argv[++i]);
            config.wdtFileId = std::stoi(argv[++i]);
        } else if (arg == "-wmo" && i + 1 < argc) {
            config.wmoFileId = std::stoi(argv[++i]);
        } else if (arg == "-m2" && i + 1 < argc) {
            config.m2FileId = std::stoi(argv[++i]);
        } else if (arg == "-camera" && i + 3 < argc) {
            config.cameraPos.x = std::stof(argv[++i]);
            config.cameraPos.y = std::stof(argv[++i]);
            config.cameraPos.z = std::stof(argv[++i]);
        } else if (arg == "-lookat" && i + 3 < argc) {
            config.lookAt.x = std::stof(argv[++i]);
            config.lookAt.y = std::stof(argv[++i]);
            config.lookAt.z = std::stof(argv[++i]);
        } else if (arg == "-cameraIndex" && i + 1 < argc) {
            config.cameraIndex = std::stoi(argv[++i]);
        } else if (arg == "-time" && i + 1 < argc) {
            config.timeOverride = std::stoi(argv[++i]);
        } else if (arg == "-size" && i + 2 < argc) {
            config.width = std::stoi(argv[++i]);
            config.height = std::stoi(argv[++i]);
        } else if (arg == "-gapi" && i + 1 < argc) {
            config.gapiName = argv[++i];
        } else if (arg == "-bindless") {
            config.supportsBindless = true;
        } else if (arg == "-validation") {
            config.enableValidation = true;
        } else if (arg == "-video") {
            config.recordVideo = true;
        } else if (arg == "-videoOutput" && i + 1 < argc) {
            config.videoOutputPath = argv[++i];
        } else if (arg == "-fps" && i + 1 < argc) {
            config.fps = std::stoi(argv[++i]);
        } else if (arg == "-duration" && i + 1 < argc) {
            config.videoDuration = std::stof(argv[++i]);
        } else if (arg == "-videoFramebuffer" && i + 2 < argc) {
            config.videoFramebufferWidth = std::stoi(argv[++i]);
            config.videoFramebufferHeight = std::stoi(argv[++i]);
        } else if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            exit(0);
        } else {
            std::cerr << "Unknown option: " << arg << std::endl;
            printUsage(argv[0]);
            exit(1);
        }
    }

    return config;
}

int main(int argc, char* argv[]) {
#ifdef _WIN32
    // On Windows, use CommandLineToArgvW to properly handle quoted paths with spaces
    LPWSTR* szArglist;
    int nArgs;
    
    szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
    if (szArglist == NULL) {
        std::cerr << "CommandLineToArgvW failed" << std::endl;
        return 1;
    }
    
    // Convert wide strings to UTF-8
    std::vector<std::string> argStrings;
    argStrings.reserve(nArgs); // Reserve space to prevent reallocation
    
    for (int i = 0; i < nArgs; i++) {
        int size = WideCharToMultiByte(CP_UTF8, 0, szArglist[i], -1, NULL, 0, NULL, NULL);
        std::string str(size - 1, '\0');
        WideCharToMultiByte(CP_UTF8, 0, szArglist[i], -1, &str[0], size, NULL, NULL);
        argStrings.push_back(str);
    }
    
    LocalFree(szArglist);
    
    // Build pointer array AFTER all strings are added (to ensure pointers remain valid)
    std::vector<char*> argPointers;
    argPointers.reserve(nArgs);
    for (auto& str : argStrings) {
        argPointers.push_back(&str[0]);
    }
    
    argc = nArgs;
    argv = argPointers.data();
    
    // Debug: Print all arguments
    std::cout << "Total arguments: " << argc << std::endl;
    for (int i = 0; i < argc; i++) {
        std::cout << "  argv[" << i << "] = \"" << argv[i] << "\"" << std::endl;
    }
    std::cout << std::endl;
#else
    // Debug: Print all arguments on non-Windows
    std::cout << "Total arguments: " << argc << std::endl;
    for (int i = 0; i < argc; i++) {
        std::cout << "  argv[" << i << "] = \"" << argv[i] << "\"" << std::endl;
    }
    std::cout << std::endl;
#endif

    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    RenderConfig config = parseArguments(argc, argv);

    try {
        OfflineRenderer renderer(config);
        
        std::cout << "Setting up renderer..." << std::endl;
        renderer.setup();
        
        std::cout << "Opening scene..." << std::endl;
        renderer.openScene();
        
        std::cout << "Setting up camera..." << std::endl;
        if (config.cameraIndex >= 0)
            renderer.setupCamera();
        
        if (config.recordVideo) {
            std::cout << "Starting video recording..." << std::endl;
        } else {
            std::cout << "Processing frames (waiting for all data to load)..." << std::endl;
        }
        renderer.process();
        
        std::cout << "Done!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

