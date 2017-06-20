//
// Created by deamon on 16.06.17.
//

#ifndef WOWVIEWERLIB_COMMONFILESTRUCTS_H
#define WOWVIEWERLIB_COMMONFILESTRUCTS_H
#include <cstdint>
#include <mathfu/glsl_mappings.h>

// Check windows
#if _WIN32 || _WIN64
#if _WIN64
#define ENVIRONMENT64
#else
#define ENVIRONMENT32
#endif
#endif

// Check GCC
#if __GNUC__
#if __x86_64__ || __ppc64__
#define ENVIRONMENT64
#else
#define ENVIRONMENT32
#endif
#endif

using fixed16 = short;
typedef mathfu::vec3_packed C3Vector;
typedef mathfu::vec2_packed C2Vector;
typedef mathfu::vec4_packed C4Quaternion;

struct M2Box {
    C3Vector ModelRotationSpeedMin;
    C3Vector ModelRotationSpeedMax;
};

struct M2Range {
    float min;
    float max;
};
typedef M2Range CRange;

struct CAaBox
{
    C3Vector min;
    C3Vector max;
};
struct Quat16 {
    int16_t x,y,z,w;
};
using M2CompQuat = Quat16;

struct M2Bounds {
    CAaBox extent;
    float radius;
};
template<typename T>
struct M2Array {
    uint32_t offset; // pointer to T, relative to begin of m2 data block (i.e. MD21 chunk content or begin of file)
    int32_t size;

    void initM2Array(void * m2File) {
        static_assert(std::is_pod<M2Array<T>>::value, "M2Array<> is not POD");
#ifdef ENVIRONMENT64
        offset = (uint32_t) (((uint64_t)m2File)+(uint64_t)offset - (uint64_t)this);
#else
        offset = (uint32_t)offset + (uint32_t)m2File;
#endif
    }
    T* getElement(int index) {
#ifdef ENVIRONMENT64
        return &((T* ) (((uint64_t)this)+offset))[index];
#else
        return ((T* )offset)[index];
#endif
    }
};

template <typename T>
void initM2M2Array(M2Array<M2Array<T>> array2D, void *m2File){
    array2D.initM2Array(m2File);
    int count = array2D.size;
    for (int i = 0; i < count; i++){
        M2Array<T> *array1D = array2D.getElement(i);
        array1D->initM2Array(m2File);
    }
}

struct M2TrackBase {
    uint16_t interpolation_type;
    uint16_t global_sequence;
    M2Array<M2Array<uint32_t>> timestamps;
    void initTrack(void * m2File) {
        initM2M2Array(timestamps, m2File);
    }
};

template<typename T>
struct M2PartTrack {
    M2Array<fixed16> times;
    M2Array<T> values;
};

template<typename T>
struct M2Track : M2TrackBase
{
    M2Array<M2Array<T>> values;
    void initTrack(void * m2File){
        M2TrackBase::initTrack(m2File);
        initM2M2Array(values, m2File);
    };
};



template<typename T>
struct M2SplineKey {
    T value;
    T inTan;
    T outTan;
};

using CArgb = uint32_t;
struct CImVector
{
    unsigned char b;
    unsigned char g;
    unsigned char r;
    unsigned char a;
};

struct C4Plane // todo: verify
{
    C3Vector normal;
    float distance;
};
#endif //WOWVIEWERLIB_COMMONFILESTRUCTS_H
