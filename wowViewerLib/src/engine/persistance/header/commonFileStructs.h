//
// Created by deamon on 16.06.17.
//

#ifndef WOWVIEWERLIB_COMMONFILESTRUCTS_H
#define WOWVIEWERLIB_COMMONFILESTRUCTS_H
#include <cstdint>
#include <string>
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
#if __x86_64__ || __ppc64__ || __aarch64__
#define ENVIRONMENT64
#else
#define ENVIRONMENT32
#endif
#endif

#ifndef _MSC_VER
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#else
#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop) )
#endif

#ifdef _MSC_VER
//#include "../../../include/stdint_msvc.h"
#else
#include <stdint.h>
#endif

#define DEBUGPOINTER 0

template<class T>
struct PointerChecker {
private:
#ifdef DEBUGPOINTER
    int &maxLenPtr;
#endif
    T *elementOffset = nullptr;

    PointerChecker() = default;
//    T& operator=(int index) {
//        assert(true);
//    }
public:
#ifdef DEBUGPOINTER
    PointerChecker(int &maxLen) : maxLenPtr(maxLen) {
    }
#else
    PointerChecker(int &maxLen){
    }
#endif

    inline T* operator=(T* other) {
        this->elementOffset = other;
        return other;
    }
    inline  T* operator->() {
#ifdef DEBUGPOINTER
        assert(elementOffset != nullptr);
#endif
        return this->elementOffset;
    }

    inline T& operator[](size_t index) {
#ifdef DEBUGPOINTER
        if (index >= maxLenPtr) {
            assert(index < maxLenPtr);
        }
#endif
        return elementOffset[index];
    }
    inline T& operator[](size_t index) const {
#ifdef DEBUGPOINTER
        if (index >= maxLenPtr) {
            assert(index < maxLenPtr);
        }
#endif
        return elementOffset[index];
    }
    inline bool operator==(T* other) const{
        return elementOffset == other;
    }
    inline bool operator!=(T* other) const{
        return elementOffset != other;
    }
};




using fixed16 = int16_t;
typedef mathfu::vec4_packed C4Vector;
typedef mathfu::vec3_packed C3Vector;
typedef mathfu::vec2_packed C2Vector;
typedef mathfu::vec4_packed C4Quaternion;

typedef uint8_t ubyte4[4];

struct M2Box {
    C3Vector ModelRotationSpeedMin;
    C3Vector ModelRotationSpeedMax;
};

struct M2Range {
    float min;
    float max;
};
struct M2RangeInt {
    uint32_t min;
    uint32_t max;
};
typedef M2Range CRange;

struct CAaBox
{
public:
    CAaBox(){};
    CAaBox(C3Vector pmin, C3Vector pmax) {
		this->min = pmin;
        this->max = pmax;
    }

//    CAaBox operator=(CAaBox &a) {
//        this->min = a.min;
//        this->max = a.max;
//        return *this;
//    }

    C3Vector min = mathfu::vec3_packed(mathfu::vec3(20000, 20000, 20000));
    C3Vector max = mathfu::vec3_packed(mathfu::vec3(-20000, -20000, -20000));
};
struct CRect
{
    float miny;
    float minx;
    float maxy;
    float maxx;
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
    int32_t size;
    int32_t offset; // pointer to T, relative to begin of m2 data block (i.e. MD21 chunk content or begin of file)

    void initM2Array(void * m2File) {
        static_assert(std::is_pod<M2Array<T> >::value, "M2Array<> is not POD");
#ifdef ENVIRONMENT64
        offset = (uint32_t) (((uint64_t)m2File)+(uint64_t)offset - (uint64_t)this);
#else
        offset = (uint32_t)offset + (uint32_t)m2File;
#endif
    }
    T* getElement(int index) const {
        if (index >= size) {
            return nullptr;
        }
#ifdef ENVIRONMENT64
        return &((T* ) (((uint64_t)this)+offset))[index];
#else
        return &((T* )offset)[index];
#endif
    }
    T* operator[](int index) const {
        return getElement(index);
    }
    inline std::string toString(){
        static_assert(true, "This conversion to string is not defined");
        return "";
    }
};
template<>
inline std::string M2Array<char>::toString() {
    char * ptr = this->getElement(0);
    std::string result;
    result = std::string(ptr, ptr+size);

    return result;
}

struct CM2SequenceLoad {
    int animationIndex;
    uint8_t *animFileDataBlob;
};

struct CSkelSequenceLoad {
    int animationIndex;
    uint8_t *animFileDataBlob;
    uint8_t *animFileBoneDataBlob;
    uint8_t *animFileAttDataBlob;
};

struct M2Sequence {
    uint16_t id;                   // Animation id in AnimationData.dbc
    uint16_t variationIndex;       // Sub-animation id: Which number in a row of animations this one is.

    uint32_t duration;             // The length (timestamps) of the animation. I believe this actually the length of the animation in milliseconds.
    float movespeed;               // This is the speed the character moves with in this animation.
    uint32_t flags;                // See below.
    int16_t frequency;             // This is used to determine how often the animation is played. For all animations of the same type, this adds up to 0x7FFF (32767).
    uint16_t _padding;
    M2RangeInt replay;                // May both be 0 to not repeat. Client will pick a random number of repetitions within bounds if given.
    uint32_t blendtime;            // The client blends (lerp) animation states between animations where the end and start values differ. This specifies how long that blending takes. Values: 0, 50, 100, 150, 200, 250, 300, 350, 500.
    M2Bounds bounds;
    int16_t variationNext;         // id of the following animation of this AnimationID, points to an Index or is -1 if none.
    uint16_t aliasNext;            // id in the list of animations. Used to find actual animation if this sequence is an alias (flags & 0x40)
};

template <typename T>
void initAnimationArray(M2Array<M2Array<T> > &array2D, void *m2File, M2Array<M2Sequence> *sequences, CM2SequenceLoad *cm2SequenceLoad){
    static_assert(std::is_pod<M2Array<M2Array<T> > >::value, "M2Array<M2Array<T>> array2D is not POD");
    if (cm2SequenceLoad == nullptr) {
        array2D.initM2Array(m2File);
        int count = array2D.size;
        for (int i = 0; i < count; i++) {
            if (sequences != nullptr) {
                if ((sequences->size > 0) && (sequences->getElement(i)->flags & 0x20) == 0) continue;
            }

            M2Array<T> *array1D = array2D.getElement(i);
            array1D->initM2Array(m2File);
        }
    } else {
        if (cm2SequenceLoad->animationIndex >= array2D.size)
            return;

        M2Array<T> *array1D = array2D.getElement(cm2SequenceLoad->animationIndex);
        array1D->initM2Array(cm2SequenceLoad->animFileDataBlob);
    }
}

struct M2TrackBase {
    uint16_t interpolation_type;
    uint16_t global_sequence;
    M2Array<M2Array<uint32_t> > timestamps;
    void initTrackBase(void * m2File, M2Array<M2Sequence> *sequences, CM2SequenceLoad *cm2SequenceLoad) {
        initAnimationArray(timestamps, m2File, sequences, cm2SequenceLoad);
    }
};

template<typename T>
struct M2PartTrack {
    M2Array<fixed16> timestamps;
    M2Array<T> values;
    void initPartTrack(void * m2File){
        timestamps.initM2Array(m2File);
        values.initM2Array(m2File);
    };
};

template<typename T>
struct M2Track
{
    uint16_t interpolation_type;
    int16_t global_sequence;
    M2Array<M2Array<uint32_t> > timestamps;
    M2Array<M2Array<T> > values;
    void initTrack(void * m2File, M2Array<M2Sequence> *sequences, CM2SequenceLoad *cm2SequenceLoad){
        initAnimationArray(timestamps, m2File, sequences, cm2SequenceLoad);
        initAnimationArray(values, m2File, sequences, cm2SequenceLoad);
    };
};




template<typename T>
struct M2SplineKey {
    T value;
    T inTan;
    T outTan;
};


struct CImVector
{
    unsigned char b;
    unsigned char g;
    unsigned char r;
    unsigned char a;
};
using CArgb = CImVector;

struct C4Plane
{
    union {
        struct {
            C3Vector normal;
            float distance;
        } planeGeneral;
        C4Vector planeVector;
    };
};
PACK(struct vector_2fp_6_9 { uint16_t x; uint16_t y; });
#endif //WOWVIEWERLIB_COMMONFILESTRUCTS_H
