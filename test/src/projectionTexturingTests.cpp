#include "gtest/gtest.h"
#include "../../wowViewerLib/src/engine/algorithms/mathHelper.h"

#define ERROR_TOLERANCE 0.0001
#define EXPECT_NEAR_VEC2(x, y) \
    EXPECT_NEAR(x[0], y[0], ERROR_TOLERANCE); \
    EXPECT_NEAR(x[1], y[1], ERROR_TOLERANCE)

// Tests factorial of positive numbers.
TEST(ProjectionTextureTest, testMatrices) {
    mathfu::vec2 corner1 = {-39.18847,   7.79936}; //1.0f, 0f
    mathfu::vec2 corner2 = { 29.82233, -16.62189}; //0.0f, 1.f
    mathfu::vec2 corner3 = {  8.11358, -38.70366}; //0.f, 0.f
    mathfu::vec2 corner4 = {-17.47971,  29.88112}; //1.f, 1.f

    auto projMat = MathHelper::createProjectionalTexture(corner3, corner1, corner2);

    auto p1 = projMat * mathfu::vec4(corner1, mathfu::vec2(0, 1));
    auto p2 = projMat * mathfu::vec4(corner2, mathfu::vec2(0, 1));
    auto p3 = projMat * mathfu::vec4(corner3, mathfu::vec2(0, 1));
    auto p4 = projMat * mathfu::vec4(corner4, mathfu::vec2(0, 1));

    EXPECT_NEAR_VEC2(p3.xy(), mathfu::vec2(0, 0));
    EXPECT_NEAR_VEC2(p1.xy(), mathfu::vec2(1, 0));
    EXPECT_NEAR_VEC2(p2.xy(), mathfu::vec2(0, 1));
    EXPECT_NEAR_VEC2(p4.xy(), mathfu::vec2(1, 1));
}