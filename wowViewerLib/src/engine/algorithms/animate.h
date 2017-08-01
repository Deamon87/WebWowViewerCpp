//
// Created by deamon on 29.06.17.
//

#ifndef WOWVIEWERLIB_ANIMATE_H
#define WOWVIEWERLIB_ANIMATE_H

#include "../persistance/header/M2FileHeader.h"
#include <vector>
int binary_search(M2Array<uint32_t>& vec, int start, int end, uint32_t& key);

int32_t findTimeIndex(
        double currTime,
        int animationIndex,
        M2Array<M2Array<uint32_t>> &timestamps
);

template<typename T, typename R>
inline R convertHelper(T &value) {
//    REGISTER_PARSE_TYPE(T);
//    template <typename T> struct MyClassTemplate<T*>;
    //static_assert(false, "This function was not meant to be called");
    throw "This function was not meant to be called";

};
template<>
inline mathfu::vec4 convertHelper<mathfu::vec4_packed, mathfu::vec4>(mathfu::vec4_packed &a ) {
    return mathfu::vec4(a);
};
inline float stf(unsigned short Short) {
    return (Short / float (32767)) - 1.0f; // (Short > 0 ? Short-32767 : Short+32767)/32767.0;
}
template<>
inline mathfu::quat convertHelper<Quat16, mathfu::quat>(Quat16 &a ) {
    return mathfu::quat(
            stf(a.w),
            stf(a.x),
            stf(a.y),
            stf(a.z)
    );
};
template<>
inline mathfu::quat convertHelper<C4Quaternion, mathfu::quat>(C4Quaternion &a ) {
    return mathfu::quat(
            a.w ,
            a.x,
            a.y,
            a.z
    );
};
template<>
inline mathfu::vec4 convertHelper<mathfu::vec3_packed, mathfu::vec4>(mathfu::vec3_packed &a ) {
    return mathfu::vec4(a.x, a.y, a.z, 0);
};

template<typename T>
inline T lerpHelper(T &value1, T &value2, float percent) {
//    REGISTER_PARSE_TYPE(T);
//    template <typename T> struct MyClassTemplate<T*>;
    throw "This function was not meant to be called";

};
template<>
inline mathfu::vec4 lerpHelper<mathfu::vec4>(mathfu::vec4 &value1, mathfu::vec4 &value2, float percent) {
    return mathfu::vec4::Lerp(value1, value2, percent);
};
template<>
inline mathfu::quat lerpHelper<mathfu::quat>(mathfu::quat &value1, mathfu::quat &value2, float percent) {
    return mathfu::quat::Slerp(value1, value2, percent);
};


template<typename T, typename R>
R animateTrack(
        double &currTime,
        uint32_t &maxTime,
        int &animationIndex,
        M2Track<T> &animationBlock,
        M2Array<M2Loop> &global_loops,
        std::vector<double> &globalSequenceTimes,
        R &defaultValue) {

    if (animationBlock.timestamps[animationIndex]->size == 0) {
        return defaultValue;
    }

    int16_t globalSequence = animationBlock.global_sequence;

    int32_t timeIndex;
    if (globalSequence >=0) {
        currTime = globalSequenceTimes[globalSequence];
        maxTime = global_loops[globalSequence]->timestamp;
    }

    M2Array<uint32_t > *times = animationBlock.timestamps[animationIndex];
    M2Array<T> *values = animationBlock.values[animationIndex];

    timeIndex = findTimeIndex(currTime, animationIndex, animationBlock.timestamps);

    if (timeIndex == times->size-1) {
        return convertHelper<T, R>(*values->getElement(timeIndex));
    } else if (timeIndex >= 0) {
        R value1 = convertHelper<T, R>(*values->getElement(timeIndex));
        R value2 = convertHelper<T, R>(*values->getElement(timeIndex+1));

        int time1 = *times->getElement(timeIndex);
        int time2 = *times->getElement(timeIndex+1);

        uint16_t interpolType = animationBlock.interpolation_type;
        if (interpolType == 0) {
            return value1;
        } else if (interpolType >= 1) {
            return lerpHelper<R>(value1, value2, (float)((float)time1 - currTime)/(float)(time1 - time2));
        }
    } else {
        return defaultValue;
    }
}

#endif //WOWVIEWERLIB_ANIMATE_H
