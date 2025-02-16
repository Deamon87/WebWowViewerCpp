#include "gtest/gtest.h"
#include <memory>
#include "../../src/persistance/CascRequestProcessor.h"
#include "../../src/database/CEmptySqliteDB.h"
#include "../../wowViewerLib/src/engine/WowFilesCacheStorage.h"
#include "../../src/database/buildInfoParser/buildDefinition.h"
#include "../../src/ui/childWindow//sceneWindow/SceneWindow.h"
#include "../../wowViewerLib/src/gapi/IDeviceFactory.h"
#include "../../wowViewerLib/src/renderer/frame/SceneComposer.h"
#include "../../wowViewerLib/src/engine/camera/firstPersonCamera.h"
#include "../../wowViewerLib/src/gapi/vulkan/GDeviceVulkan.h"

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



struct GAPI_config {
    std::string gapiName = "";
    bool supportsBindless = false;
    bool enableValidation = false;
};

std::string CASC_PATH = "";

extern bool forceDisableBindlessSupport;

class RenderScenesTestFixture : public ::testing::TestWithParam<GAPI_config> {

    std::unique_ptr<SceneComposer> sceneComposer;
    HApiContainer apiContainer = nullptr;
    std::shared_ptr<SceneWindow> sceneWindow = nullptr;

    void SetUp() override {

        const auto CASC_PATH = std::getenv("CASC_PATH");

#ifdef LINK_VULKAN
        vkCallInitCallback callback;
        callback.createSurface = [&](VkInstance vkInstance) -> VkSurfaceKHR {
            // VkSurfaceKHR surface;

            // if (glfwCreateWindowSurface(vkInstance, window, nullptr, &surface) != VK_SUCCESS) {
            // throw std::runtime_error("failed to create window surface!");
            // }

            return VK_NULL_HANDLE;
        };
        callback.getRequiredExtensions = [](char** &extensionNames, int &extensionCnt) {
            extensionNames = nullptr;
            extensionCnt = 0;
        };
#else
        void *callback = nullptr;
#endif

        auto &param = this->GetParam();

        vulkanEnableValidationLayers = param.enableValidation;
        forceDisableBindlessSupport = !param.supportsBindless;

        apiContainer = std::make_shared<ApiContainer>();

        //1. Create filesystem (CASC for now)
        BuildDefinition buildDef;
        apiContainer->requestProcessor = std::make_shared<CascRequestProcessor>(CASC_PATH, buildDef);
        apiContainer->cacheStorage = std::make_shared<WoWFilesCacheStorage>(apiContainer->requestProcessor.get());

        //2. Create database
        apiContainer->databaseHandler = std::make_shared<CEmptySqliteDB>();

        //3. Create device
        apiContainer->hDevice = IDeviceFactory::createDevice(param.gapiName, &callback);

        //4. Create composer
        sceneComposer = std::make_unique<SceneComposer>(apiContainer, false);

        //5. Create sceneWindow
        sceneWindow = std::make_shared<SceneWindow>(apiContainer, false, nullptr);
        sceneWindow->setViewPortDimensions({{0,0}, {1024,1024}});
    }

public:
    void create() {
        sceneWindow->openMapByIdAndWDTId(2217, 2842322, -11595, 9280, 260, -1);
        sceneWindow->getCamera()
    }

    void process() {
        auto l_device = apiContainer->hDevice;

        int framesToContinue = IDevice::MAX_FRAMES_IN_FLIGHT+1;
        while (framesToContinue-- > 0) {
            auto processingFrame = l_device->getFrameNumber();
            std::function<uint32_t()> updateFrameNumberLambda = [l_device, frame = processingFrame]() -> uint32_t {
                l_device->setCurrentProcessingFrameNumber(frame);
                return frame;
            };

            HFrameScenario scenario = std::make_shared<HFrameScenario::element_type>();
            sceneWindow->render(0, 60*0.5f/M_PI, scenario, nullptr, updateFrameNumberLambda);
            sceneComposer->draw(scenario, false);

            auto lastPlan = sceneWindow->getLastPlan();
            if (!apiContainer->requestProcessor->completedAllJobs() ||
                !lastPlan ||
                !lastPlan->wmoArray.getToLoad().empty() ||
                !lastPlan->wmoGroupArray.getToLoad().empty() ||
                !lastPlan->m2Array.getToLoadGeom().empty() ||
                !lastPlan->m2Array.getToLoadMain().empty()
            ) {
                framesToContinue = IDevice::MAX_FRAMES_IN_FLIGHT+1;
            }
        }
    }
};

TEST_P(RenderScenesTestFixture, fooTest) {
    create();
    process();
}

INSTANTIATE_TEST_SUITE_P(
    MeenyMinyMoe,
    RenderScenesTestFixture,
    testing::Values(GAPI_config{"vulkan", false, false }));

GTEST_API_ int main(int argc, char *argv[]) {
#ifdef _WIN32
    SetUnhandledExceptionFilter(windows_exception_handler);
    const bool SET_TERMINATE = std::set_terminate(beforeCrash);
#if !defined(_MSC_VER) && !defined(_LIBCPP_VERSION)
    const bool SET_TERMINATE_UNEXP = std::set_unexpected(beforeCrash);
#endif
#endif

    auto argIdx = 1;
    while ((argc - argIdx) >= 2) {
        auto mainOption = std::string(argv[argIdx++]);
        auto secondOption = std::string(argv[argIdx++]);
        if (mainOption == "-casc") {
            CASC_PATH = secondOption;
        }
    }
    printf("Initializing gtest \n");
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}