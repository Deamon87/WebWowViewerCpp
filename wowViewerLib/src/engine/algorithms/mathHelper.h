//
// Created by deamon on 22.06.17.
//

#ifndef WOWVIEWERLIB_MATHHELPER_H
#define WOWVIEWERLIB_MATHHELPER_H

#include "../persistance/header/commonFileStructs.h"
#include "mathfu/glsl_mappings.h"
#include <vector>
#include "../custom_allocators/FrameBasedStackAllocator.h"
#include <thread>

template<typename T>
inline constexpr T toRadian(T x) {
    return (x * M_PI) / 180.0;
}

template<typename T>
inline constexpr T fromRadian(T x) {
    return (x * 180.0) / M_PI;
}

template<typename T>
inline const CAaBox &retrieveAABB(T &object);


class MathHelper {
public:
    static const constexpr float TILESIZE =  1600.0f/3.0f ;
    static const constexpr float CHUNKSIZE = TILESIZE / 16.0f;
    static const constexpr float UNITSIZE =  CHUNKSIZE / 8.0f;

    static const constexpr float INFZ_MAT4_HANDNESS = 1.0f;
    static const constexpr float LOOKAT_MAT_HANDNESS = -1.0f;

    struct PlanesUndPoints {
        framebased::vector<mathfu::vec4> planes;
        framebased::vector<mathfu::vec3> points;
        framebased::vector<mathfu::vec3> hullLines;
    };

    typedef struct {
        mathfu::mat4 perspectiveMat;
        mathfu::mat4 viewMat;
        mathfu::vec4 farPlane;
        mathfu::vec3 cameraPos;
        std::vector<PlanesUndPoints> frustums;
    } FrustumCullingData;

    typedef struct {
        double h;       // angle in degrees
        double s;       // a fraction between 0 and 1
        double v;       // a fraction between 0 and 1
    } hsv;

    static hsv rgb2hsv(const mathfu::vec3 &in);
    static mathfu::vec3 hsv2rgb(const hsv &in);

    static float fp69ToFloat(uint16_t x);

    static mathfu::mat4 getVulkanMat4Fix();
    static mathfu::mat4 createPerspectiveMat(float fovy, float aspect, float zNear, float zFar);
    static mathfu::mat4 createLookAtMat(mathfu::vec3 cameraPos, mathfu::vec3 cameraTarget, mathfu::vec3 cameraUp);
    static mathfu::vec2 convertV69ToV2(vector_2fp_6_9 &fp69);

    static CAaBox transformAABBWithMat4(const mathfu::mat4 &matrix, const mathfu::vec4 &min, const mathfu::vec4 &max);
    static framebased::vector<mathfu::vec4> transformPlanesWithMat(const framebased::vector<mathfu::vec4> &planes, const mathfu::mat4 &mat);
    static framebased::vector<mathfu::vec3> getIntersectionPointsFromPlanes(const framebased::vector<mathfu::vec4> &planes);
    static framebased::vector<mathfu::vec4> getFrustumClipsFromMatrix(const mathfu::mat4 &mat);
    static void fixNearPlane(framebased::vector<mathfu::vec4> &planes, mathfu::vec4 &camera) {
        mathfu::vec4 &nearPlane = planes[5];
        mathfu::vec4 cameraVec4 = mathfu::vec4(camera[0], camera[1], camera[2], 1);
        float dist = mathfu::dot(nearPlane, cameraVec4);
        nearPlane[3] -= dist;
    }
    static framebased::vector<mathfu::vec3> calculateFrustumPointsFromMat(mathfu::mat4 &perspectiveViewMat);

    static bool checkFrustum(const MathHelper::FrustumCullingData &frustumData, const CAaBox &box);
    static bool checkFrustum(const std::vector<PlanesUndPoints> &frustums, const CAaBox &box);
    static bool checkFrustum2D(const std::vector<PlanesUndPoints> &frustums, const CAaBox &box);
    static framebased::vector<mathfu::vec3> getHullPoints(framebased::vector<mathfu::vec3> &points);
    static framebased::vector<mathfu::vec3> getHullLines(framebased::vector<mathfu::vec3> &points);

    //Handness = -1 or 1
    static inline mathfu::mat4 getInfZMatrix(float fovR, float aspect) {
        float f = 1.0f / tan(fovR / 2.0f);
        return mathfu::mat4(
            f / aspect, 0.0f,  0.0f,  0.0f,
            0.0f,    f,  0.0f,  0.0f,
            0.0f, 0.0f,  -1, 1.0f,
            0.0f, 0.0f, 2.0f,  0.0f);
    }

    static inline mathfu::mat4 RotationX(float angle) {
        return mathfu::quat::FromAngleAxis(angle, mathfu::vec3(1,0,0)).ToMatrix4();
    };

    static inline mathfu::mat4 RotationY(float angle) {
        return mathfu::quat::FromAngleAxis(angle, mathfu::vec3(0,1,0)).ToMatrix4();
    };

    static inline mathfu::mat4 RotationZ(float angle) {
        return mathfu::quat::FromAngleAxis(angle, mathfu::vec3(0,0,1)).ToMatrix4();
    };

    static inline mathfu::mat3 mat3RotationX(float angle) {
        return mathfu::quat::FromAngleAxis(angle, mathfu::vec3(1,0,0)).ToMatrix();
    };

    static inline mathfu::mat3 mat3RotationY(float angle) {
        return mathfu::quat::FromAngleAxis(angle, mathfu::vec3(0,1,0)).ToMatrix();
    };

    static inline mathfu::mat3 mat3RotationZ(float angle) {
        return mathfu::quat::FromAngleAxis(angle, mathfu::vec3(0,0,1)).ToMatrix();
    };

    static inline const mathfu::mat4 &getAdtToWorldMat4() {
        static const mathfu::mat4 adtToWorldMat4 = {
                {0, -1, 0, 0},
                {0, 0, 1, 0},
                {-1, 0, 0, 0},
                {32*MathHelper::TILESIZE, 32*MathHelper::TILESIZE, 0, 1}};

        return adtToWorldMat4;
    }
//

    static bool planeCull(framebased::vector<mathfu::vec3> &points, framebased::vector<mathfu::vec4> &planes);

    static void sortVec3ArrayAgainstPlane(framebased::vector<mathfu::vec3> &thisPortalVertices, const mathfu::vec4 &plane);
    static mathfu::vec4 createPlaneFromEyeAndVertexes(
        const mathfu::vec3 &eye,
        const mathfu::vec3 &vertex1,
        const mathfu::vec3 &vertex2);

    static float distanceFromAABBToPoint(const CAaBox &aabb, mathfu::vec3 &p);

    static bool isAabbIntersect2d(CAaBox a, CAaBox b);

    static bool isPointInsideAABB(const CAaBox &aabb, const mathfu::vec3 &p);
    static bool isPointInsideAABB(const mathfu::vec2 aabb[2], const mathfu::vec2 &p);

    static bool isPointInsideNonConvex(const mathfu::vec3 &p, const CAaBox &aabb, const std::vector<mathfu::vec2> &points);
    static float findLeastDistanceToBorder(const mathfu::vec3 &p, const std::vector<mathfu::vec2> &points, bool &isInsideConvex);

    static mathfu::vec3 getBarycentric(mathfu::vec3 &p, mathfu::vec3 &a, mathfu::vec3 &b, mathfu::vec3 &c);

    static mathfu::vec4 planeLineIntersection(mathfu::vec4 &plane, mathfu::vec4 &p1, mathfu::vec4 &p2);
    static mathfu::vec3 getIntersection( mathfu::vec3 &tail1, mathfu::vec3 &head1, mathfu::vec3 &tail2, mathfu::vec3 &head2);

    static float distance_aux(float p, float lower, float upper);

    static float distanceFromAABBToPoint2DSquared(const mathfu::vec2 aabb[2], mathfu::vec2 &p);
    static mathfu::vec3 calcExteriorColorDir(const mathfu::mat4 &lookAtMat, int time);
    static mathfu::vec3 calcSunPlanetPos(const mathfu::mat4 &lookAtMat, int time);
    static mathfu::mat4 createProjectionalTexture(mathfu::vec2 p_t00, mathfu::vec2 p_t10, mathfu::vec2 p_t01);
};

const float ROUNDING_ERROR_f32 = 0.001f;
inline bool feq(const float a, const float b, const float tolerance = ROUNDING_ERROR_f32)
{
    return (a + tolerance >= b) && (a - tolerance <= b);
}

inline float worldCoordinateToAdtIndexF(float x) {
    return (32.0f - (x / MathHelper::TILESIZE));
}

inline int worldCoordinateToAdtIndex(float x) {
    return floor(worldCoordinateToAdtIndexF(x));
}

inline int worldCoordinateToGlobalAdtChunk(float x) {
    return floor(( (32.0f*16.0f) - (x / (MathHelper::CHUNKSIZE)   )));
}

inline float AdtIndexToWorldCoordinate(float x) {
    return (32.0f - x) * MathHelper::TILESIZE;
}

#endif //WOWVIEWERLIB_MATHHELPER_H
