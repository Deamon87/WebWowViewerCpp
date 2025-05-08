#include "gtest/gtest.h"
#include <memory>
#include <filesystem>

#include "../../../src/persistance/CascRequestProcessor.h"
#include "../../../src/database/CEmptySqliteDB.h"
#include "../../../src/database/CSqliteDB.h"
#include "../../../wowViewerLib/src/engine/WowFilesCacheStorage.h"
#include "../../../src/database/buildInfoParser/buildDefinition.h"
#include "../../../src/ui/childWindow//sceneWindow/SceneWindow.h"
#include "../../../src/ui/childWindow/cascStorageDialog/CascStorageEntry.h"
#include "../../../wowViewerLib/src/gapi/IDeviceFactory.h"
#include "../../../wowViewerLib/src/renderer/frame/SceneComposer.h"
#include "../../../wowViewerLib/src/engine/camera/firstPersonCamera.h"
#include "../../../wowViewerLib/src/gapi/vulkan/GDeviceVulkan.h"

#include "../../../wowViewerLib/src/gapi/renderdoc_app.h"
#include "renderSceneAbstract.h"

#ifdef _WIN32
    #include <errhandlingapi.h>
#endif

#if INCLUDE_EXTERNAL==1
#include "./externalRenderScene.h"
#endif

#include "../../../wowViewerLib/src/gapi/interface/FrameContext.h"

#include <csignal>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <set>
#include <chrono>
#include <iomanip>
#ifdef _WIN32
    #include <stdlib.h>  // for _putenv_s
#endif

// Custom XML writer that writes separate report files per test suite
class PerSuiteXmlWriter : public ::testing::EmptyTestEventListener {
public:
    explicit PerSuiteXmlWriter(const std::string& outputDir) : m_outputDir(outputDir) {}

    void OnTestSuiteStart(const ::testing::TestSuite& test_suite) override {
        m_currentSuite = test_suite.name();
        m_suiteStartTime = std::chrono::steady_clock::now();
        m_testResults.clear();
    }

    void OnTestStart(const ::testing::TestInfo& test_info) override {
        m_testStartTime = std::chrono::steady_clock::now();
    }

    void OnTestEnd(const ::testing::TestInfo& test_info) override {
        auto endTime = std::chrono::steady_clock::now();
        double durationSec = std::chrono::duration<double>(endTime - m_testStartTime).count();

        TestResult result;
        result.name = test_info.name();
        result.durationSec = durationSec;
        result.passed = test_info.result()->Passed();
        result.skipped = test_info.result()->Skipped();

        // Collect failure messages
        for (int i = 0; i < test_info.result()->total_part_count(); ++i) {
            const auto& part = test_info.result()->GetTestPartResult(i);
            if (part.failed()) {
                result.failureMessage += part.message();
                result.failureMessage += "\n";
            }
        }

        // Collect recorded properties (like pngFile)
        for (int i = 0; i < test_info.result()->test_property_count(); ++i) {
            const auto& prop = test_info.result()->GetTestProperty(i);
            result.properties[prop.key()] = prop.value();
        }

        m_testResults.push_back(result);
    }

    void OnTestSuiteEnd(const ::testing::TestSuite& test_suite) override {
        auto endTime = std::chrono::steady_clock::now();
        double suiteDurationSec = std::chrono::duration<double>(endTime - m_suiteStartTime).count();

        // Write XML report for this suite
        std::filesystem::path reportPath = std::filesystem::path(m_outputDir) / (m_currentSuite + "_report.xml");
        std::filesystem::create_directories(reportPath.parent_path());

        std::ofstream out(reportPath);
        if (!out.is_open()) {
            std::cerr << "Failed to write report to: " << reportPath << std::endl;
            return;
        }

        int failures = 0, skipped = 0;
        for (const auto& r : m_testResults) {
            if (!r.passed && !r.skipped) failures++;
            if (r.skipped) skipped++;
        }

        out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        out << "<testsuites tests=\"" << m_testResults.size()
            << "\" failures=\"" << failures
            << "\" disabled=\"" << skipped
            << "\" time=\"" << std::fixed << std::setprecision(3) << suiteDurationSec << "\">\n";

        out << "  <testsuite name=\"" << escapeXml(m_currentSuite)
            << "\" tests=\"" << m_testResults.size()
            << "\" failures=\"" << failures
            << "\" disabled=\"" << skipped
            << "\" time=\"" << std::fixed << std::setprecision(3) << suiteDurationSec << "\">\n";

        for (const auto& r : m_testResults) {
            out << "    <testcase name=\"" << escapeXml(r.name)
                << "\" classname=\"" << escapeXml(m_currentSuite)
                << "\" time=\"" << std::fixed << std::setprecision(3) << r.durationSec << "\"";

            bool hasContent = !r.passed || !r.properties.empty();
            if (!hasContent) {
                out << " />\n";
            } else {
                out << ">\n";

                // Write properties
                if (!r.properties.empty()) {
                    out << "      <properties>\n";
                    for (const auto& [key, value] : r.properties) {
                        out << "        <property name=\"" << escapeXml(key)
                            << "\" value=\"" << escapeXml(value) << "\" />\n";
                    }
                    out << "      </properties>\n";
                }

                // Write failure info
                if (!r.passed && !r.skipped) {
                    out << "      <failure message=\"Test failed\">"
                        << escapeXml(r.failureMessage) << "</failure>\n";
                } else if (r.skipped) {
                    out << "      <skipped />\n";
                }

                out << "    </testcase>\n";
            }
        }

        out << "  </testsuite>\n";
        out << "</testsuites>\n";

        out.close();
        std::cout << "Wrote suite report: " << reportPath << std::endl;
    }

private:
    struct TestResult {
        std::string name;
        double durationSec = 0;
        bool passed = true;
        bool skipped = false;
        std::string failureMessage;
        std::map<std::string, std::string> properties;
    };

    static std::string escapeXml(const std::string& str) {
        std::string result;
        result.reserve(str.size());
        for (char c : str) {
            switch (c) {
                case '&': result += "&amp;"; break;
                case '<': result += "&lt;"; break;
                case '>': result += "&gt;"; break;
                case '"': result += "&quot;"; break;
                case '\'': result += "&apos;"; break;
                default: result += c;
            }
        }
        return result;
    }

    std::string m_outputDir;
    std::string m_currentSuite;
    std::chrono::steady_clock::time_point m_suiteStartTime;
    std::chrono::steady_clock::time_point m_testStartTime;
    std::vector<TestResult> m_testResults;
};

extern "C" void my_function_to_handle_aborts(int signal_number)
{
    /*Your code goes here. You can output debugging info.
      If you return from this function, and it was called
      because abort() was called, your program will exit or crash anyway
      (with a dialog box on Windows).
     */

    std::cout << "HELLO" << std::endl;
}


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

void initRenderDoc() {
#ifdef _WIN32
    // At init, on windows
    if (false)
    {
        rdoc_api = nullptr;
        HMODULE mod = LoadLibraryA("renderdoc.dll");
        if (mod)
        {
            pRENDERDOC_GetAPI RENDERDOC_GetAPI =
                (pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");
            int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_4_0, (void **)&rdoc_api);
            if (ret == 1) {
                std::cout << "Acquired renderdoc API" << std::endl;
            }

        }
    }
    if (rdoc_api && false) {
        int secondsToWaitMax = 30;
        std::cout << "Waiting for RenderDoc to within 30 sec" << std::endl;
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

struct GAPI_config {
    std::string gapiName = "";
    bool supportsBindless = false;
    bool enableValidation = false;
    bool forceForward = false;
};

// Default CASC_PATH from command-line or environment (not stored in git)
static std::string& getDefaultCascPath() {
    static std::string defaultPath = "";
    static bool initialized = false;

    if (!initialized) {
        // Fall back to environment variable if not set via command-line
        const char* envPath = std::getenv("CASC_PATH");
        if (envPath) {
            defaultPath = envPath;
        }
        initialized = true;
    }
    return defaultPath;
}

// Read CASC config from JSON file (same format as CascStorageDialog's cascStorages.json entries).
// Expected structure:
// {
//   "default":   { "cascLocalFolder": "...", "buildDef": { ... } },  // optional, applied on top of -casc/CASC_PATH
//   "overrides": { "TestName": { "cascLocalFolder": "...", "buildDef": { ... } } }  // optional, per-test partial overrides
// }
// Only fields present in JSON are overwritten, so an override may specify just the fields it changes.
static nlohmann::json loadCascConfigJson() {
    static nlohmann::json config;
    static bool loaded = false;

    if (!loaded) {
        // Look for config file in current directory or test directory
        std::vector<std::string> configPaths = {
            "casc_config.json",
            "test/casc_config.json",
            "test/src/casc_config.json"
        };

        for (const auto& configPath : configPaths) {
            std::ifstream file(configPath);
            if (file.is_open()) {
                try {
                    file >> config;
                    std::cout << "Loaded CASC config from " << configPath << std::endl;
                } catch (const std::exception& e) {
                    std::cerr << "Error parsing " << configPath << ": " << e.what() << std::endl;
                }
                break; // Use first found config file
            }
        }
        loaded = true;
    }

    return config;
}

// Get CASC storage entry (path + build definition) with per-test override support from config file
static CascStorageEntry getCascStorageEntry(const std::string& testName) {
    // 1. Base defaults: path from command-line/environment, default-constructed BuildDefinition
    CascStorageEntry entry;
    entry.cascLocalFolder = getDefaultCascPath();

    const auto& config = loadCascConfigJson();

    // 2. Apply "default" section from config file (if any)
    if (config.is_object() && config.contains("default")) {
        from_json(config["default"], entry);
    }

    // 3. Apply per-test override from config file (if test name is available)
    if (!testName.empty() && config.is_object() && config.contains("overrides")) {
        auto& overrides = config["overrides"];
        if (overrides.is_object() && overrides.contains(testName)) {
            from_json(overrides[testName], entry);
        } else {
            // Log that no override was found
            std::cout << "No CASC override found for " << testName << ". Using default one" << std::endl;
        }
    }

    return entry;
}


extern bool forceDisableBindlessSupport;

class RenderSceneGAPI : public RenderSceneAbstract {
private:
    std::unique_ptr<SceneComposer> sceneComposer;
    HApiContainer apiContainer = nullptr;
    std::shared_ptr<SceneWindow> sceneWindow = nullptr;

    std::stringstream output_buffer;

    GAPI_config m_param;

public:
    RenderSceneGAPI(GAPI_config param) : m_param(param) {};
    ~RenderSceneGAPI() override {

    };

    void tearDown() override {
        auto device = apiContainer ? apiContainer->hDevice : nullptr;;

        sceneComposer = nullptr;
        apiContainer = nullptr;
        sceneWindow = nullptr;

        output_buffer.clear();

        if (device) {
            device->clear();
        }
    };

    void setup() override {
        initRenderDoc();

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

//        vulkanEnableValidationLayers = m_param.enableValidation;
        vulkanEnableValidationLayers = false;
        forceDisableBindlessSupport = !m_param.supportsBindless;
        bool forceForwardRendering = m_param.forceForward;

        apiContainer = std::make_shared<ApiContainer>();

        // output_buffer.str(std::string());
        // std::cout.rdbuf(output_buffer.rdbuf());
        // std::cerr.rdbuf(output_buffer.rdbuf());

        //1. Create filesystem (CASC for now)
        // Get test name for config file lookup
        std::string testName = "";
        const testing::TestInfo* const test_info = testing::UnitTest::GetInstance()->current_test_info();
        if (test_info) {
            // Format: "TestName"
            testName = std::string(test_info->name());
        }

        CascStorageEntry cascEntry = getCascStorageEntry(testName);
        apiContainer->requestProcessor = std::make_shared<CascRequestProcessor>(cascEntry.cascLocalFolder, cascEntry.buildDef);
        apiContainer->cacheStorage = std::make_shared<WoWFilesCacheStorage>(apiContainer->requestProcessor.get());

        //2. Create database
        // apiContainer->databaseHandler = std::make_shared<CEmptySqliteDB>();
        apiContainer->databaseHandler = std::make_shared<CSqliteDB>("./export.db3");

        //3. Create device
        apiContainer->hDevice = IDeviceFactory::createDevice(m_param.gapiName, &callback);

        //4. Create composer
        sceneComposer = std::make_unique<SceneComposer>(apiContainer, false);

        //5. Create sceneWindow
        sceneWindow = std::make_shared<SceneWindow>(apiContainer, false, nullptr, forceForwardRendering);
        sceneWindow->setViewPortDimensions({{0,0}, {1024,1024}});
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

    void openMap(int mapId, int wdtFileId, const mathfu::vec3 &cameraPos, const mathfu::vec3 &lookAt, int timeOverride) override {
        sceneWindow->openMapByIdAndWDTId(mapId, wdtFileId, cameraPos.x, cameraPos.y, cameraPos.z, timeOverride);
        sceneWindow->getCamera()->setCameraLookAt(lookAt.x, lookAt.y, lookAt.z);
    }
    void openWMO(int wmoFileId, const mathfu::vec3 &cameraPos, const mathfu::vec3 &lookAt, int timeOverride) override {
        sceneWindow->openWMOSceneByfdid(wmoFileId);
        sceneWindow->getCamera()->setCameraPos(cameraPos.x, cameraPos.y, cameraPos.z);
        sceneWindow->getCamera()->setCameraLookAt(lookAt.x, lookAt.y, lookAt.z);
    }
    void process(testing::Test *test) override {
        auto l_device = apiContainer->hDevice;

        int framesToContinue = IDevice::MAX_FRAMES_IN_FLIGHT+1;
        bool makeScreenshot = false;
        std::shared_ptr<IRenderDocCaptureHandler> captureHandler = nullptr;

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
                    const testing::TestInfo* const test_info =
                          testing::UnitTest::GetInstance()->current_test_info();

                    std::filesystem::path pngFilePath =
                        std::string(test_info->test_suite_name()) +
                        "/" +
                        test_info->name() +
                        ".png";

                    test->RecordProperty("pngFile", pngFilePath.string());

                    pngFilePath = std::filesystem::absolute(std::filesystem::path(testResultFolder) / pngFilePath);


                    auto folderPathForPng = pngFilePath.parent_path();
                    std::filesystem::create_directories(folderPathForPng);

                    std::string pngFileName = pngFilePath.string();

                    sceneWindow->makeScreenshot(DEFAULT_FOV_VALUE,
                        1024, 1024,
                        pngFileName,
                        scenario, updateFrameNumberLambda
                    );
                    framesToContinue = IDevice::MAX_FRAMES_IN_FLIGHT+1;
                    makeScreenshot = true;
                    captureWithRenderDoc = true;
                } else {
                    sceneWindow->render(0, DEFAULT_FOV_VALUE, scenario, nullptr, updateFrameNumberLambda);
                }
            }

            if (captureWithRenderDoc && rdoc_api)
                captureHandler = apiContainer->hDevice->getRenderDocHelper();

            sceneComposer->draw(scenario, false);

            auto lastPlan = sceneWindow->getLastPlan();
            if (!isFrameLoaded(lastPlan)) {
                framesToContinue = IDevice::MAX_FRAMES_IN_FLIGHT+1;
            }

            captureHandler = nullptr;
        }

        l_device->waitForAllWorkToComplete();
    }
};


class RenderScenesTestFixture : public ::testing::TestWithParam<std::function<HRenderSceneInstance()>> {
private:
    HRenderSceneInstance m_renderSceneInst;

    void SetUp() override {
        m_renderSceneInst = this->GetParam()();
        m_renderSceneInst->setup();
    }

    void TearDown() override {
        m_renderSceneInst->tearDown();
    }

public:
    const HRenderSceneInstance getInstance() {
        return m_renderSceneInst;
    };
};

TEST_P(RenderScenesTestFixture, NzothMap) {
    auto &renderSceneInst = this->getInstance();
this;
    renderSceneInst->openMap(2217, 2842322, {-11595, 9280, 260}, {0,0,0}, -1);
    renderSceneInst->process(this);
    EXPECT_TRUE(true);
}
TEST_P(RenderScenesTestFixture, EversongTest) {
    auto &renderSceneInst = this->getInstance();

    renderSceneInst->openMap(530, 828395, {9305.81, -7442.45, 236.1}, {9304.83, -7442.5, 235.909}, 1398);
    renderSceneInst->process(this);
    EXPECT_TRUE(true);
}
TEST_P(RenderScenesTestFixture, ArathorBuildingTest) {
    auto &renderSceneInst = this->getInstance();

    renderSceneInst->openMap(2601, 4914790, {2633.42, 2052.34, 712.67}, {2634.21, 2051.75, 712.548}, 1398);
    renderSceneInst->process(this);
    EXPECT_TRUE(true);
}
TEST_P(RenderScenesTestFixture, InteriorFloatingGarrison) {
    auto &renderSceneInst = this->getInstance();

    renderSceneInst->openWMO(1576792, {-14.3089, 1.57191, 3.31259}, {-13.3302, 1.47767, 3.13035}, 1398);
    renderSceneInst->process(this);
    EXPECT_TRUE(true);
}

TEST_P(RenderScenesTestFixture, DeadMinesTest) {
    auto &renderSceneInst = this->getInstance();

    renderSceneInst->openMap(36, 780605, {-238.819, -573.68, 61.4023}, {-237.933, -573.304, 61.1308}, 0);
    renderSceneInst->process(this);
    EXPECT_TRUE(true);
}
TEST_P(RenderScenesTestFixture, AhnKahetTest) {
    auto &renderSceneInst = this->getInstance();

    renderSceneInst->openMap(619, 818056, {541.886, -947.982, 162.23}, {542.37, -948.847, 162.365}, 0);
    renderSceneInst->process(this);
    EXPECT_TRUE(true);
}
TEST_P(RenderScenesTestFixture, DraenorHeightTextTest) {
    auto &renderSceneInst = this->getInstance();

    renderSceneInst->openMap(1116, 842315, {4084.86, -2089.91, 49.4204}, {4084.7, -2089.18, 48.7513}, 0);
    renderSceneInst->process(this);
    EXPECT_TRUE(true);
}
TEST_P(RenderScenesTestFixture, IronforgeWagonTest) {
    auto &renderSceneInst = this->getInstance();

    renderSceneInst->openMap(0, 775971, {-4933.28, -950.835, 512.3}, {-4932.4, -951.284, 512.11}, 0);
    renderSceneInst->process(this);
    EXPECT_TRUE(true);
}
TEST_P(RenderScenesTestFixture, IronforgeGryphonsTest) {
    auto &renderSceneInst = this->getInstance();

    renderSceneInst->openMap(0, 775971, {-4825.39, -1170.09, 508.631}, {-4825.02, -1169.17, 508.522}, 0);
    renderSceneInst->process(this);
    EXPECT_TRUE(true);
}
TEST_P(RenderScenesTestFixture, GnomereganTest) {
    auto &renderSceneInst = this->getInstance();

    renderSceneInst->openMap(90, 782773, {-527.852, 414.312, -202.29}, {-528.096, 415.253, -202.524}, 0);
    renderSceneInst->process(this);
    EXPECT_TRUE(true);
}
TEST_P(RenderScenesTestFixture, AwakeningMachineTest) {
    auto &renderSceneInst = this->getInstance();

    renderSceneInst->openMap(2710, 5525210, {1802.74, 336.837, 311.313}, {1803.55, 336.253, 311.357}, 0);
    renderSceneInst->process(this);
    EXPECT_TRUE(true);
}
TEST_P(RenderScenesTestFixture, SotaLiquidTest) {
    auto &renderSceneInst = this->getInstance();

    renderSceneInst->openMap(607, 789579, {1549.47, -182.156, 37.5648}, {1550.15, -181.443, 37.404}, 0);
    renderSceneInst->process(this);
    EXPECT_TRUE(true);
}
TEST_P(RenderScenesTestFixture, NewSilvermoonEntranceTest) {
    auto &renderSceneInst = this->getInstance();

    renderSceneInst->openMap(0, 775971, {7930.67, -4616.36, 200}, {7930.69, -4616.36, 199}, 0);
    renderSceneInst->process(this);
    EXPECT_TRUE(true);
}
TEST_P(RenderScenesTestFixture, CavernOfTimeRibbonTest) {
    auto &renderSceneInst = this->getInstance();

    renderSceneInst->openMap(1, 782779, {-8467.03, -4778.31, -155.158}, {-8467.13, -4777.32, -155.284}, 0);
    renderSceneInst->process(this);
    EXPECT_TRUE(true);
}
TEST_P(RenderScenesTestFixture, ArgusDungeonProjectionTest) {
    auto &renderSceneInst = this->getInstance();

    renderSceneInst->openMap(1753, 1618081, {5862.56, 10771.1, 90.3372}, {5862.96, 10770.5, 89.6746}, 0);
    renderSceneInst->process(this);
    EXPECT_TRUE(true);
}

TEST_P(RenderScenesTestFixture, ArgusDungeonProjectionAnimTest) {
    auto &renderSceneInst = this->getInstance();

    renderSceneInst->openMap(1753, 1618081, {6049.45, 10555.7, 106.512}, {6048.77, 10555.4, 105.811}, 0);
    renderSceneInst->process(this);
    EXPECT_TRUE(true);
}
TEST_P(RenderScenesTestFixture, ArgusDungeonDarknessLevelTest) {
    auto &renderSceneInst = this->getInstance();

    renderSceneInst->openMap(1753, 1618081, {5967.91, 10649.3, 50.0779}, {5968.55, 10649.6, 49.3991}, 0);
    renderSceneInst->process(this);
    EXPECT_TRUE(true);
}
TEST_P(RenderScenesTestFixture, BrokenIslesProjectedTextureTest) {
    auto &renderSceneInst = this->getInstance();

    renderSceneInst->openMap(1220, 957721, {-1026.63, 3290.91, 110.458}, {-1026.11, 3290.44, 109.742}, 0);
    renderSceneInst->process(this);
    EXPECT_TRUE(true);
}
TEST_P(RenderScenesTestFixture, BrokenIslesBalackTowerTest) {
    auto &renderSceneInst = this->getInstance();

    renderSceneInst->openMap(1220, 957721, {-743.287, 2710.8, 105.703}, {-742.763, 2710.05, 106.098}, 0);
    renderSceneInst->process(this);
    EXPECT_TRUE(true);
}

TEST_P(RenderScenesTestFixture, NightFallLightsTest) {
    auto &renderSceneInst = this->getInstance();

    renderSceneInst->openMap(2686, 5363692, {-805.694, 54.6233, 148.302}, {-804.866, 55.1766, 148.21}, 0);
    renderSceneInst->process(this);
    EXPECT_TRUE(true);
}

INSTANTIATE_TEST_SUITE_P(
    VulkanWithBindless,
    RenderScenesTestFixture,
    testing::Values(
        [] { return std::make_shared<RenderSceneGAPI>(GAPI_config{"vulkan", true, true }); }
    ));

INSTANTIATE_TEST_SUITE_P(
    VulkanForward,
    RenderScenesTestFixture,
    testing::Values(
        [] { return std::make_shared<RenderSceneGAPI>(GAPI_config{"vulkan", false, true, true }); }
    )
);

INSTANTIATE_TEST_SUITE_P(
    VulkanDeferred,
    RenderScenesTestFixture,
    testing::Values(
        [] { return std::make_shared<RenderSceneGAPI>(GAPI_config{"vulkan", false, true }); }
    )
);

#if INCLUDE_EXTERNAL==1
INSTANTIATE_TEST_SUITE_P(
    External,
    RenderScenesTestFixture,
    testing::Values(
         [] { return std::make_shared<RenderSceneExternal>(); }
    ));
#endif

GTEST_API_ int main(int argc, char *argv[]) {
#ifdef _WIN32
    SetUnhandledExceptionFilter(windows_exception_handler);
    const bool SET_TERMINATE = std::set_terminate(beforeCrash);
#if !defined(_MSC_VER) && !defined(_LIBCPP_VERSION)
    const bool SET_TERMINATE_UNEXP = std::set_unexpected(beforeCrash);
#endif
#endif
    signal(SIGABRT, &my_function_to_handle_aborts);
    signal(SIGSEGV, &my_function_to_handle_aborts);


    std::ios_base::Init init_streams;

    auto argIdx = 1;
    while ((argc - argIdx) >= 2) {
        auto mainOption = std::string(argv[argIdx++]);
        auto secondOption = std::string(argv[argIdx++]);
        if (mainOption == "-casc") {
            // Set default CASC_PATH from command-line (overrides environment variable)
            getDefaultCascPath() = secondOption;
        }
    }
    printf("Initializing gtest \n");

    ::testing::InitGoogleTest(&argc, argv);

    // Register per-suite XML writer that creates separate report files
    // Each test suite (VulkanWithBindless, VulkanForward, VulkanDeferred, etc.) gets its own XML file
    ::testing::TestEventListeners& listeners = ::testing::UnitTest::GetInstance()->listeners();
    listeners.Append(new PerSuiteXmlWriter(testResultFolder));

    // Also keep the combined report for backwards compatibility
    ::testing::GTEST_FLAG(output) = "xml:" + testResultFolder + "/report.xml";

    return RUN_ALL_TESTS();
}