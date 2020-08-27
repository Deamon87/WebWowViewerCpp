//
// Created by deamon on 22.06.17.
//

#ifndef WOWVIEWERLIB_MATHHELPER_H
#define WOWVIEWERLIB_MATHHELPER_H


#include "../persistance/header/commonFileStructs.h"
#include "mathfu/glsl_mappings.h"
#include <vector>
#include <thread>

#define toRadian(x) (float) ((float) (x) * ((float)M_PI/ (float)180.0))
const float ROUNDING_ERROR_f32 = 0.001f;
inline bool feq(const float a, const float b, const float tolerance = ROUNDING_ERROR_f32)
{
    return (a + tolerance >= b) && (a - tolerance <= b);
}


inline int worldCoordinateToAdtIndex(float x) {
    return floor((32.0f - (x / 533.33333f)));
}

inline int worldCoordinateToGlobalAdtChunk(float x) {
    return floor(( (32.0f*16.0f) - (x / (533.33333f / 16.0f)   )));
}

inline float AdtIndexToWorldCoordinate(int x) {
    return (32.0f - x) * 533.33333f;
}

class MathHelper {
public:

    static float fp69ToFloat(uint16_t x);

    static mathfu::vec2 convertV69ToV2(vector_2fp_6_9 &fp69);

    static CAaBox transformAABBWithMat4(mathfu::mat4 matrix, mathfu::vec4 min, mathfu::vec4 max);
    static std::vector<mathfu::vec4> getFrustumClipsFromMatrix(mathfu::mat4 &mat);
    static void fixNearPlane(std::vector<mathfu::vec4> &planes, mathfu::vec4 &camera) {
        mathfu::vec4 &nearPlane = planes[5];
        mathfu::vec4 cameraVec4 = mathfu::vec4(camera[0], camera[1], camera[2], 1);
        float dist = mathfu::dot(nearPlane, cameraVec4);
        nearPlane[3] -= dist;
    }
    static std::vector<mathfu::vec3> calculateFrustumPointsFromMat(mathfu::mat4 &perspectiveViewMat);
    static bool checkFrustum(const std::vector<mathfu::vec4> &planes, const CAaBox &box, const std::vector<mathfu::vec3> &points);
    static bool checkFrustum2D(std::vector<mathfu::vec3> &planes, CAaBox &box);
    static std::vector<mathfu::vec3> getHullPoints(std::vector<mathfu::vec3> &points);
    static std::vector<mathfu::vec3> getHullLines(std::vector<mathfu::vec3> &points);



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

    static const mathfu::mat4 &getAdtToWorldMat4() {
        const float TILESIZE = 533.333333333f;

//        mathfu::mat4 adtToWorldMat4 = mathfu::mat4::Identity();
//        adtToWorldMat4 *= MathHelper::RotationX(toRadian(90));
//        adtToWorldMat4 *= MathHelper::RotationY(toRadian(90));
//        adtToWorldMat4 *= mathfu::mat4::FromTranslationVector(mathfu::vec3(32*TILESIZE, 0, 32*TILESIZE));
//        adtToWorldMat4 *= mathfu::mat4::FromScaleVector(mathfu::vec3(-1, 1, -1));

        static mathfu::mat4 adtToWorldMat4 = {
                {0, -1, 0, 0},
                {0, 0, 1, 0},
                {-1, 0, 0, 0},
                {32*TILESIZE, 32*TILESIZE, 0, 1}};

        return adtToWorldMat4;
    }
//

    static bool planeCull(std::vector<mathfu::vec3> &points, std::vector<mathfu::vec4> &planes);

    static void sortVec3ArrayAgainstPlane(std::vector<mathfu::vec3> &thisPortalVertices, const mathfu::vec4 &plane);
    static mathfu::vec4 createPlaneFromEyeAndVertexes(
        const mathfu::vec3 &eye,
        const mathfu::vec3 &vertex1,
        const mathfu::vec3 &vertex2);

    static float distanceFromAABBToPoint(const CAaBox &aabb, mathfu::vec3 &p);

    static bool isPointInsideAABB(const CAaBox &aabb, mathfu::vec3 &p);
    static bool isPointInsideAABB(const mathfu::vec2 aabb[2], mathfu::vec2 &p);

    static bool isPointInsideNonConvex(mathfu::vec3 &p, const CAaBox &aabb, const std::vector<mathfu::vec2> &points);

    static mathfu::vec3 getBarycentric(mathfu::vec3 &p, mathfu::vec3 &a, mathfu::vec3 &b, mathfu::vec3 &c);

    static mathfu::vec4 planeLineIntersection(mathfu::vec4 &plane, mathfu::vec4 &p1, mathfu::vec4 &p2);
    static mathfu::vec3 getIntersection( mathfu::vec3 &tail1, mathfu::vec3 &head1, mathfu::vec3 &tail2, mathfu::vec3 &head2);

    static float distance_aux(float p, float lower, float upper);

    static float distanceFromAABBToPoint2DSquared(const mathfu::vec2 aabb[2], mathfu::vec2 &p);
};


#endif //WOWVIEWERLIB_MATHHELPER_H
