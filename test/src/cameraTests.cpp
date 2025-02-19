#include "../../wowViewerLib/src/engine/camera/CameraInterface.h"
#include "../../wowViewerLib/src/engine/camera/firstPersonCamera.h"
#include "gtest/gtest.h"

constexpr float DEFAULT_FOV_VALUE = 53.9726294579437f;

typedef std::shared_ptr<ICamera> HCamera;
auto firstPersonCameraFactory = +[]() -> HCamera {
    return std::make_shared<FirstPersonCamera>();
};

typedef decltype(firstPersonCameraFactory) CameraFactory;

class CameraTestFixture : public ::testing::TestWithParam<CameraFactory> {
private:
    HCamera m_camera;
public:
    HCameraMatrices getCameraMatrices() {
        return m_camera->getCameraMatrices(DEFAULT_FOV_VALUE, 1.0f, 1.0f, 1000.f);
    }

    void setCamera(const mathfu::vec3 &cameraPos, const mathfu::vec3 &lookAt) {
        auto camera = m_camera;
        camera->setCameraPos(cameraPos.x,cameraPos.y,cameraPos.z);
        camera->setCameraLookAt(lookAt.x,lookAt.y,lookAt.z);
        camera->tick(0);
    }

    float calcLookAtDist(const mathfu::vec3 &camera, const mathfu::vec3 &lookAt) {
        auto cameraMatrices = getCameraMatrices();
        cameraMatrices->lookAt.xyz();

        auto normDir = (lookAt - camera).Normalized();

        auto normLookAt = camera + normDir;
        auto dist = mathfu::vec3::Distance(normLookAt, cameraMatrices->lookAt.xyz());

        return dist;
    }

    void SetUp() override {
        m_camera = GetParam()();
    }
};

TEST_P(CameraTestFixture, shouldKeepLookAt) {
    mathfu::vec3 camera = {456.0f, 789.0f, 123.0f};
    mathfu::vec3 lookAt = {123.0f, 456.0f, 789.0f};
    setCamera(camera, lookAt);

    float dist = calcLookAtDist(camera, lookAt);

    EXPECT_TRUE(dist < 0.0001f);
}

TEST_P(CameraTestFixture, shouldKeepLookAt2) {
    mathfu::vec3 camera = {456.0f, 123.0f, 789.0f};
    mathfu::vec3 lookAt = {123.0f, 789.0f, 456.0f};
    setCamera(camera, lookAt);

    float dist = calcLookAtDist(camera, lookAt);

    EXPECT_TRUE(dist < 0.0001f);
}

TEST_P(CameraTestFixture, shouldIncreaseDepthWithDistance) {
    setCamera({0,0,0}, {1.0f, 0, 0});
    auto cameraMatrices = getCameraMatrices();

    auto MVP = cameraMatrices->perspectiveMat * cameraMatrices->lookAtMat;

    auto pointNear = MVP * mathfu::vec4(1,0,0,1); pointNear /= pointNear.w;
    auto pointFar  = MVP * mathfu::vec4(2,0,0,1); pointFar /= pointFar.w;

    EXPECT_TRUE(pointNear.z < pointFar.z);
}

TEST_P(CameraTestFixture, shouldIncreaseDepthWithDistance2) {
    setCamera({0,0,0}, {0.0f, 1.0f, 0});
    auto cameraMatrices = getCameraMatrices();

    auto MVP = cameraMatrices->perspectiveMat * cameraMatrices->lookAtMat;

    auto pointNear = MVP * mathfu::vec4(0, 1.0f, 0, 1); pointNear /= pointNear.w;
    auto pointFar  = MVP * mathfu::vec4(0, 2.0f, 0, 1); pointFar /= pointFar.w;

    EXPECT_TRUE(pointNear.z < pointFar.z);
}

INSTANTIATE_TEST_SUITE_P(
    FirstPersonCamera,
    CameraTestFixture,
    testing::Values(firstPersonCameraFactory)
);
