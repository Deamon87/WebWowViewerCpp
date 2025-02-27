#include "../../wowViewerLib/src/engine/camera/CameraInterface.h"
#include "../../wowViewerLib/src/engine/camera/firstPersonCamera.h"
#include "../../wowViewerLib/src/engine/algorithms/mathHelper.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#define EXPECT_FEQ(a, b) EXPECT_THAT(a, ::testing::AllOf(testing::Ge(b-0.01f), testing::Le(b+0.01f)))

constexpr float DEFAULT_FOV_VALUE = 53.9726294579437f;
constexpr float DEFAULT_NEAR_PLANE = 1.0f;
constexpr float DEFAULT_FAR_PLANE = 1000.0f;

typedef std::shared_ptr<ICamera> HCamera;
auto firstPersonCameraFactory = +[]() -> HCamera {
    return std::make_shared<FirstPersonCamera>();
};

typedef decltype(firstPersonCameraFactory) CameraFactory;

// (factory, isInfiniteZ, isVulkanDepth)
typedef std::tuple<CameraFactory, bool, bool> CamTestParams;

auto firstPersonCameraVulkanFactory = +[]() -> HCamera {
    class FirstPersonCameraVulkan : public FirstPersonCamera {
    public:
        HCameraMatrices getCameraMatrices(float fov, float canvasAspect, float nearPlane, float farPlane) {
            auto matrices = FirstPersonCamera::getCameraMatrices(fov, canvasAspect, nearPlane, farPlane);
            matrices->perspectiveMat = MathHelper::getVulkanMat4Fix() * matrices->perspectiveMat;

            return matrices;
        }
    };
    return std::make_shared<FirstPersonCameraVulkan>();
};
auto firstPersonCameraInvZFactory = +[]() -> HCamera {
    class FirstPersonCameraInvZ : public FirstPersonCamera {
    public:
        HCameraMatrices getCameraMatrices(float fov, float canvasAspect, float nearPlane, float farPlane) {
            auto matrices = FirstPersonCamera::getCameraMatrices(fov, canvasAspect, nearPlane, farPlane);
            matrices->perspectiveMat = MathHelper::getInfZMatrix(
                DEFAULT_FOV_VALUE,
                1.0f
            );

            return matrices;
        }
    };
    return std::make_shared<FirstPersonCameraInvZ>();
};
auto firstPersonCameraInvZVulkanFactory = +[]() -> HCamera {
    class FirstPersonCameraInvZVulkan : public FirstPersonCamera {
    public:
        HCameraMatrices getCameraMatrices(float fov, float canvasAspect, float nearPlane, float farPlane) {
            auto matrices = FirstPersonCamera::getCameraMatrices(fov, canvasAspect, nearPlane, farPlane);
            matrices->perspectiveMat = MathHelper::getVulkanMat4Fix() * MathHelper::getInfZMatrix(
                DEFAULT_FOV_VALUE,
                1.0f
            );

            return matrices;
        }
    };
    return std::make_shared<FirstPersonCameraInvZVulkan>();
};



class CameraTestFixture : public ::testing::TestWithParam<CamTestParams> {
private:
    HCamera m_camera;
    bool m_isInfiniteZ;
    bool m_isVulkanDepth;
public:
    bool isVulkanDepth() { return m_isVulkanDepth; }

    HCameraMatrices getCameraMatrices() {
        return m_camera->getCameraMatrices(DEFAULT_FOV_VALUE, 1.0f, DEFAULT_NEAR_PLANE, DEFAULT_FAR_PLANE);
    }

    void setCamera(const mathfu::vec3 &cameraPos, const mathfu::vec3 &lookAt) {
        auto camera = m_camera;
        camera->setCameraPos(cameraPos.x,cameraPos.y,cameraPos.z);
        camera->setCameraLookAt(lookAt.x,lookAt.y,lookAt.z);
        camera->tick(0);
    }

    //Returns delta of path
    float stepForward() {
        constexpr float deltaTime = 0.5f;
        m_camera->startMovingForward();
        m_camera->tick(deltaTime);
        m_camera->stopMovingForward();

        float pathDelta = m_camera->getMovementSpeed() * deltaTime;
        EXPECT_GE(pathDelta, 0.0f);

        return pathDelta;
    }

    //Returns delta of path
    float stepRight() {
        constexpr float deltaTime = 0.5f;
        m_camera->startStrafingRight();
        m_camera->tick(deltaTime);
        m_camera->stopStrafingRight();

        float pathDelta = m_camera->getMovementSpeed() * deltaTime;
        EXPECT_GE(pathDelta, 0.0f);

        return pathDelta;
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
        m_camera = std::get<0>(GetParam())();
        m_isInfiniteZ = std::get<1>(GetParam());
        m_isVulkanDepth = std::get<2>(GetParam());
    }
};

TEST_P(CameraTestFixture, shouldKeepLookAt) {
    mathfu::vec3 camera = {456.0f, 789.0f, 123.0f};
    mathfu::vec3 lookAt = {123.0f, 456.0f, 789.0f};
    setCamera(camera, lookAt);

    float dist = calcLookAtDist(camera, lookAt);

    EXPECT_LE(dist, 0.0001f);
}

TEST_P(CameraTestFixture, shouldKeepLookAt2) {
    mathfu::vec3 camera = {456.0f, 123.0f, 789.0f};
    mathfu::vec3 lookAt = {123.0f, 789.0f, 456.0f};
    setCamera(camera, lookAt);

    float dist = calcLookAtDist(camera, lookAt);

    EXPECT_LE(dist, 0.0001f);
}

TEST_P(CameraTestFixture, shouldKeepLookAt3) {
    mathfu::vec3 camera = {0, 0, 0};
    mathfu::vec3 lookAt = {1, 0, 0};
    setCamera(camera, lookAt);

    float dist = calcLookAtDist(camera, lookAt);

    EXPECT_LE(dist, 0.0001f);
}

TEST_P(CameraTestFixture, shouldKeepLookAt4) {
    mathfu::vec3 camera = {0, 0, 0};
    mathfu::vec3 lookAt = {0, 1, 0};
    setCamera(camera, lookAt);

    float dist = calcLookAtDist(camera, lookAt);

    EXPECT_LE(dist, 0.0001f);
}
TEST_P(CameraTestFixture, shouldKeepLookAt5) {
    mathfu::vec3 camera = {0, 0, 0};
    mathfu::vec3 lookAt = {-1, 0, 0};
    setCamera(camera, lookAt);

    float dist = calcLookAtDist(camera, lookAt);

    EXPECT_LE(dist, 0.0001f);
}

TEST_P(CameraTestFixture, shouldKeepLookAt6) {
    mathfu::vec3 camera = {0, 0, 0};
    mathfu::vec3 lookAt = {0, -1, 0};
    setCamera(camera, lookAt);

    float dist = calcLookAtDist(camera, lookAt);

    EXPECT_LE(dist, 0.0001f);
}

TEST_P(CameraTestFixture, shouldViewPointsBeOnSameLine) {
    mathfu::vec3 lookAtPos = {1.0f, 0, 0};
    setCamera({0,0,0}, lookAtPos);
    auto cameraMatrices = getCameraMatrices();

    auto lookAtMatInv = cameraMatrices->lookAtMat.Inverse();

    //The Z axis is targeted from LookAt position towards Camera position.
    //Henhe Z coordinate is -1
    auto minusRight = mathfu::vec3(-0.5, 0, 1);
    auto right = mathfu::vec3(0.5, 0, 1);

    auto point1 = (lookAtMatInv * mathfu::vec4(minusRight, 1)).xyz();
    auto point2 = (lookAtMatInv * mathfu::vec4(right, 1)).xyz();

    float cosAlpha = mathfu::vec3::DotProduct(
        (point1 - point2).Normalized(),
        (point1 - lookAtPos).Normalized()
    );

    EXPECT_FLOAT_EQ(cosAlpha, 1.0f);
}

TEST_P(CameraTestFixture, shouldMatchDepthBoundariesViewPersp) {
    setCamera({0,0,0}, {1.0f, 0, 0});
    auto cameraMatrices = getCameraMatrices();

    auto viewPerspective = cameraMatrices->perspectiveMat * cameraMatrices->lookAtMat;

    auto pointNear = viewPerspective * mathfu::vec4(DEFAULT_NEAR_PLANE,0,0,1); pointNear /= pointNear.w;
    auto pointInTheMiddle  = viewPerspective * mathfu::vec4(
        (DEFAULT_NEAR_PLANE + DEFAULT_FAR_PLANE) * 0.5f,0,0,1); pointInTheMiddle /= pointInTheMiddle.w;
    auto pointFar  = viewPerspective * mathfu::vec4(DEFAULT_FAR_PLANE,0,0,1); pointFar /= pointFar.w;

    float lowDepth = -1.0f, highDepth = 1.0f;
    if (isVulkanDepth()) {
        lowDepth = 0.0f, highDepth = 1.0f;
    }

    //WoW uses inverted depth for perspective matrix
    std::swap(lowDepth, highDepth);

    EXPECT_FEQ(pointNear.z, lowDepth);
    EXPECT_FEQ(pointFar.z, highDepth);
    //EXPECT_EQ(pointInTheMiddle.z, highDepth);
}

TEST_P(CameraTestFixture, shouldMatchDepthBoundariesPersp) {
    setCamera({0,0,0}, {1.0f, 0, 0});
    auto cameraMatrices = getCameraMatrices();

    auto perspective = cameraMatrices->perspectiveMat;

    auto pointNear =
        perspective * mathfu::vec4(0,0,DEFAULT_NEAR_PLANE,1); pointNear /= pointNear.w;
    auto pointInTheMiddle  =
        perspective * mathfu::vec4(0,0,(DEFAULT_NEAR_PLANE + DEFAULT_FAR_PLANE) * 0.5f, 1); pointInTheMiddle /= pointInTheMiddle.w;
    auto pointFar  = perspective * mathfu::vec4(0,0,DEFAULT_FAR_PLANE,1); pointFar /= pointFar.w;

    float lowDepth = -1.0f, highDepth = 1.0f;
    if (isVulkanDepth()) {
        lowDepth = 0.0f, highDepth = 1.0f;
    }

    //WoW uses inverted depth for perspective matrix
    std::swap(lowDepth, highDepth);

    EXPECT_FEQ(pointNear.z, lowDepth);
    EXPECT_FEQ(pointFar.z, highDepth);
    //EXPECT_EQ(pointInTheMiddle.z, highDepth);
}

TEST_P(CameraTestFixture, shouldGoForwardToLookAtTarget) {
    mathfu::vec3 cameraPos = {0,0,0};
    mathfu::vec3 lookAt = {0.0f, 1.0f, 0};
    setCamera({0,0,0}, {0.0f, 1.0f, 0});

    auto cameraDataOld = getCameraMatrices();

    auto delta = stepForward();
    auto expectedCamPos = cameraPos + (lookAt - cameraPos).Normalized() * delta;

    auto cameraData = getCameraMatrices();
    float dist = (expectedCamPos - cameraData->cameraPos.xyz()).Length();

    EXPECT_LE(dist, 0.0001f);
}

TEST_P(CameraTestFixture, shouldStrafeRight) {
    mathfu::vec3 cameraPos = {0,0,0};
    mathfu::vec3 lookAt = {0.0f, 1.0f, 0};
    setCamera({0,0,0}, {0.0f, 1.0f, 0});

    auto cameraDataOld = getCameraMatrices();

    auto delta = stepRight();
    auto expectedCamPos = cameraPos + mathfu::vec3(1,0,0) * delta;

    auto cameraData = getCameraMatrices();
    float dist = (expectedCamPos - cameraData->cameraPos.xyz()).Length();

    EXPECT_LE(dist, 0.0001f);
}


INSTANTIATE_TEST_SUITE_P(
    FirstPersonCamera,
    CameraTestFixture,
    testing::Values(std::make_tuple(firstPersonCameraFactory, false, false))
);
INSTANTIATE_TEST_SUITE_P(
    firstPersonCameraVulkan,
    CameraTestFixture,
    testing::Values(std::make_tuple(firstPersonCameraVulkanFactory, false, true))
);
INSTANTIATE_TEST_SUITE_P(
    firstPersonCameraInfZ,
    CameraTestFixture,
    testing::Values(std::make_tuple(firstPersonCameraInvZFactory, true, false))
);
INSTANTIATE_TEST_SUITE_P(
    firstPersonCameraInfZVulkan,
    CameraTestFixture,
    testing::Values(std::make_tuple(firstPersonCameraInvZVulkanFactory, true, true)));