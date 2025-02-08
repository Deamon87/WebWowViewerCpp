#include "gtest/gtest.h"
#include <memory>
#include "../../src/persistance/CascRequestProcessor.h"
#include "../../wowViewerLib/src/engine/WowFilesCacheStorage.h"
#include "../../src/database/buildInfoParser/buildDefinition.h"

struct GAPI_config {
    std::string gapiName = "";
    bool supportsBindless = false;
    bool enableValidation = false;
};

std::string CASC_PATH = "";

class RenderScenesTestFixture : public ::testing::TestWithParam<GAPI_config> {

    virtual void SetUp() override {
        //1. Create filesystem (CASC for now)
        BuildDefinition buildDef;
        auto newProcessor = std::make_shared<CascRequestProcessor>(CASC_PATH, buildDef);
        auto newStorage = std::make_shared<WoWFilesCacheStorage>(newProcessor.get());
    }
};

GTEST_API_ int _tmain(int argc, char *argv[]) {


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