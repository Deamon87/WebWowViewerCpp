//
// Created by Deamon on 25.09.22.
//

#ifndef AWEBWOWVIEWERCPP_MATHHELPER_CULLING_SSE_H
#define AWEBWOWVIEWERCPP_MATHHELPER_CULLING_SSE_H

#include "mathHelper.h"

#include <xmmintrin.h>
template<typename T>
class ObjectCullingSEE {
private:
    static inline void transpose4x4(const __m128 row0, const __m128 row1, const __m128 row2, const __m128 row3,
                             __m128 &res0, __m128 &res1, __m128 &res2, __m128 &res3) {
        __m128 row_per0 = _mm_shuffle_ps(row0, row1, _MM_SHUFFLE(1, 0, 1, 0)); // 1 2 5 6
        __m128 row_per1 = _mm_shuffle_ps(row0, row1, _MM_SHUFFLE(3, 2, 3, 2)); // 3 4 7 8
        __m128 row_per2 = _mm_shuffle_ps(row2, row3, _MM_SHUFFLE(1, 0, 1, 0)); // 9 10 13 14
        __m128 row_per3 = _mm_shuffle_ps(row2, row3, _MM_SHUFFLE(3, 2, 3, 2));  // 11 12 15 16

        res0 = _mm_shuffle_ps(row_per0, row_per2, _MM_SHUFFLE(2, 0, 2, 0));// 1 5 9 13
        res1 = _mm_shuffle_ps(row_per0, row_per2, _MM_SHUFFLE(3, 1, 3, 1));// 2 6 10 14
        res2 = _mm_shuffle_ps(row_per1, row_per3, _MM_SHUFFLE(2, 0, 2, 0));// 3 7 11 15
        res3 = _mm_shuffle_ps(row_per1, row_per3, _MM_SHUFFLE(3, 1, 3, 1));// 4 8 12 16
    }

    template <unsigned int N>
//End here is size as in std::vector.size()
    static const void culling_aabb_see_template(const MathHelper::PlanesUndPoints &frustum,
                                   const int start,
                                   const int end,
                                   const std::vector<T> &objects,
                                   std::vector<uint32_t> &culling_res)
    {
        __m128 frustum_planes_x[N];
        __m128 frustum_planes_y[N];
        __m128 frustum_planes_z[N];
        __m128 frustum_planes_d[N];
        __m128 zeros = _mm_setzero_ps();

        for (int i = 0; i < N; i++)
        {
            frustum_planes_x[i] = _mm_set1_ps(frustum.planes[i].x);
            frustum_planes_y[i] = _mm_set1_ps(frustum.planes[i].y);
            frustum_planes_z[i] = _mm_set1_ps(frustum.planes[i].z);
            frustum_planes_d[i] = _mm_set1_ps(frustum.planes[i].w);
        }

        for (int k = 0, i =start; k < ((end-start)/4); k++, i+=4) {
            const CAaBox& bbox = retrieveAABB(objects[i]);
            const CAaBox& bbox2 = retrieveAABB(objects[i+1]);
            const CAaBox& bbox3 = retrieveAABB(objects[i+2]);
            const CAaBox& bbox4 = retrieveAABB(objects[i+3]);

            ALIGNED_(16) const mathfu::vec4 minMaxVals[8] = {
                mathfu::vec4(bbox.min.x, bbox.min.y, bbox.min.z, 1.0),
                mathfu::vec4(bbox2.min.x, bbox2.min.y, bbox2.min.z, 1.0),
                mathfu::vec4(bbox3.min.x, bbox3.min.y, bbox3.min.z, 1.0),
                mathfu::vec4(bbox4.min.x, bbox4.min.y, bbox4.min.z, 1.0),
                mathfu::vec4(bbox.max.x, bbox.max.y, bbox.max.z, 1.0),
                mathfu::vec4(bbox2.max.x, bbox2.max.y, bbox2.max.z, 1.0),
                mathfu::vec4(bbox3.max.x, bbox3.max.y, bbox3.max.z, 1.0),
                mathfu::vec4(bbox4.max.x, bbox4.max.y, bbox4.max.z, 1.0),
            };

            __m128 aabb_min[4];
            for (int j = 0; j < 4; j++) aabb_min[j] = _mm_load_ps(minMaxVals[j].data_);
            __m128 aabb_max[4];
            for (int j = 0; j < 4; j++) aabb_max[j] = _mm_load_ps(minMaxVals[4+j].data_);
            __m128 aabb_min_xxxx, aabb_min_yyyy, aabb_min_zzzz, aabb_min_wwww;
            __m128 aabb_max_xxxx, aabb_max_yyyy, aabb_max_zzzz, aabb_max_wwww;

            transpose4x4(aabb_min[0], aabb_min[1], aabb_min[2], aabb_min[3],
                         aabb_min_xxxx, aabb_min_yyyy, aabb_min_zzzz, aabb_min_wwww);

            transpose4x4(aabb_max[0], aabb_max[1], aabb_max[2], aabb_max[3],
                         aabb_max_xxxx, aabb_max_yyyy, aabb_max_zzzz, aabb_max_wwww);


            __m128 intersection_current = _mm_setzero_ps();//_mm_set1_epi32(0xFFFFFFFF); //=0
            for (int j = 0; j < N; j++) { //plane index
                __m128 aabbMin_frustumPlane_x = _mm_mul_ps(aabb_min_xxxx, frustum_planes_x[j]);
                __m128 aabbMin_frustumPlane_y = _mm_mul_ps(aabb_min_yyyy, frustum_planes_y[j]);
                __m128 aabbMin_frustumPlane_z = _mm_mul_ps(aabb_min_zzzz, frustum_planes_z[j]);

                __m128 aabbMax_frustumPlane_x = _mm_mul_ps(aabb_max_xxxx, frustum_planes_x[j]);
                __m128 aabbMax_frustumPlane_y = _mm_mul_ps(aabb_max_yyyy, frustum_planes_y[j]);
                __m128 aabbMax_frustumPlane_z = _mm_mul_ps(aabb_max_zzzz, frustum_planes_z[j]);

                //We have 8 corners. It's always a combination out of {(min_x, max_x),(min_y, max_y),(min_y, max_y)}
                //So to find max distance, we can just find max on product for each of components
                //And that would give us the closest corner
                __m128 res_x = _mm_max_ps(aabbMin_frustumPlane_x, aabbMax_frustumPlane_x);
                __m128 res_y = _mm_max_ps(aabbMin_frustumPlane_y, aabbMax_frustumPlane_y);
                __m128 res_z = _mm_max_ps(aabbMin_frustumPlane_z, aabbMax_frustumPlane_z);

                //Calc final distance to plane = dot(aabb_point.xyz, plane.xyz) + plane.w
                __m128 sum_xy = _mm_add_ps(res_x, res_y);
                __m128 sum_zw = _mm_add_ps(res_z, frustum_planes_d[j]);
                __m128 distance_to_plane = _mm_add_ps(sum_xy, sum_zw);

                __m128 plane_res = _mm_cmple_ps(distance_to_plane, zeros); //is distance to plane < 0 - write 0xFFFFFFFF into result
                intersection_current = _mm_or_ps(intersection_current, plane_res); //the object is out of frustum if intersection is 0xFFFFFFFF
            }
            //Save the result
            __m128i intersection_prev = _mm_load_si128((__m128i *)&culling_res[i]); //=0
            __m128i intersection_current_i = _mm_castps_si128(intersection_current);
            intersection_current_i = _mm_and_si128(intersection_current_i, intersection_prev);
            _mm_store_si128((__m128i *)&culling_res[i], intersection_current_i);
        }
    }

    template <typename C, int... Is>
    static constexpr std::array<C, sizeof...(Is)> make_lut(std::integer_sequence<int, Is...>) {
        std::array<C, sizeof...(Is)> res{};
        ((res[Is] = &culling_aabb_see_template<Is>), ...);
        return res;
    }

    template <int Max, typename C>
    static constexpr auto make_lut() {
        return ObjectCullingSEE::make_lut<C>(std::make_integer_sequence<int, Max>{});
    }
public:
    static void cull(const MathHelper::FrustumCullingData &cullingData,
                     const int start,
                     const int end,
                     const std::vector<T> &objects,
                     std::vector<uint32_t> &culling_res) {
        static const auto lut2 = make_lut<15, std::function<void(const MathHelper::PlanesUndPoints &frustum,
                                                                     const int start,
                                                                     const int end,
                                                                     const std::vector<T> &objects,
                                                                     std::vector<uint32_t> &culling_res)>>();
        //Culling res: 0xFFFFFFFF - do not render
        //        res: 0          - render
        //Default value is 0xFFFFFFFF
        const int offCycleBegin = (4 - (start & 0x3)) & 0x3;
        const int offCycleEnd = (end & 0x3);
        for (int i = 0; i < offCycleBegin; i++) {
            culling_res[start+i] &= MathHelper::checkFrustum(cullingData.frustums, retrieveAABB(objects[start+i])) ? 0 : 0xFFFFFFFF;
        }
        for (int i = end-offCycleEnd; i < end; i++) {
            culling_res[i] &= MathHelper::checkFrustum(cullingData.frustums, retrieveAABB(objects[i])) ? 0 : 0xFFFFFFFF;
        }

        for (auto &frustum: cullingData.frustums) {
            if (frustum.planes.size() < 15) {
                lut2[frustum.planes.size()](frustum, start+offCycleBegin, end - offCycleEnd, objects, culling_res);
            }
        }
    }
};

#endif //AWEBWOWVIEWERCPP_MATHHELPER_CULLING_SSE_H
