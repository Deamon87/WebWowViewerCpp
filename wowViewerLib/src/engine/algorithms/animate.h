//
// Created by deamon on 29.06.17.
//

#ifndef WOWVIEWERLIB_ANIMATE_H
#define WOWVIEWERLIB_ANIMATE_H

#include "../../include/wowScene.h"
#include "../persistance/header/M2FileHeader.h"
#include <vector>

int binary_search(M2Array<uint32_t>& vec, int start, int end, uint32_t& key);


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
template<>
inline float convertHelper<float, float>(float &a ) {
    return a;
};

template<>
inline unsigned char convertHelper<unsigned char, unsigned char>(unsigned char &a ) {
    return a;
};


template<>
inline mathfu::vec3 convertHelper<mathfu::vec3_packed, mathfu::vec3>(mathfu::vec3_packed &a ) {
    return mathfu::vec3(a);
};
inline float stf(unsigned short Short) {
    return (Short / float (32767)) - 1.0f; // (Short > 0 ? Short-32767 : Short+32767)/32767.0;
}
inline float convertUint16ToFloat(unsigned short Short){
    return (Short * 0.000030518044) - 1.0;
}
template<>
inline mathfu::quat convertHelper<Quat16, mathfu::quat>(Quat16 &a ) {
    return mathfu::quat(
        convertUint16ToFloat(a.w),
        convertUint16ToFloat(a.x),
        convertUint16ToFloat(a.y),
        convertUint16ToFloat(a.z)
    ).Normalized();
};
template<>
inline mathfu::quat convertHelper<C4Quaternion, mathfu::quat>(C4Quaternion &a ) {
    return mathfu::quat(
            a.w ,
            a.x,
            a.y,
            a.z
    ).Normalized();
};
template<>
inline float convertHelper<fixed16, float>(fixed16 &a ) {
    return a / 32768.0;
};

template<>
inline animTime_t convertHelper<fixed16, animTime_t>(fixed16 &a ) {
    return (animTime_t)(a / 32768.0);
};

template<>
inline uint32_t convertHelper<animTime_t, uint32_t>(animTime_t &a ) {
    return (uint32_t)a;
};


template<>
inline mathfu::vec4 convertHelper<mathfu::vec3_packed, mathfu::vec4>(mathfu::vec3_packed &a ) {
    return mathfu::vec4(a.x, a.y, a.z, 0);
};

template <typename T, typename R>
int32_t findTimeIndex(
        R currTime,
        T *timestamps,
        int times_len
) {
    T timeConverted = convertHelper<R, T>(currTime);
    if (times_len > 1 ) {
        //return std::binary_search((*timeStamp)[0], (*timeStamp)[timeStamp->size-1], currTime);
//        return binary_search(*timeStamp, 0, times_len-1, currTime) - 1;
        for (int i = 1; i < times_len; i++) {
            T timestamp_time = timestamps[i];
            if (timestamp_time > timeConverted) {
                return i-1;
            }
        }
        return times_len-1;
    } else if (times_len == 1){
        return 0;
    } else {
        return -1;
    }
}



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
inline unsigned char lerpHelper<unsigned char>(unsigned char &value1, unsigned char &value2, float percent) {
    return (value1 * percent) + ((1 - percent) * value2);;
};
template<>
inline mathfu::vec3 lerpHelper<mathfu::vec3>(mathfu::vec3 &value1, mathfu::vec3 &value2, float percent) {
    return mathfu::vec3::Lerp(value1, value2, percent);
};


static inline mathfu::quat custom_slerp(mathfu::quat &a, mathfu::quat &b, double t) {
    // benchmarks:
    //    http://jsperf.com/quaternion-slerp-implementations
    float ax = a[1], ay = a[2], az = a[3], aw = a[0];
    float bx = b[1], by = b[2], bz = b[3], bw = b[0];

    float omega, cosom, sinom, scale0, scale1;

    // calc cosine
    cosom = ax * bx + ay * by + az * bz + aw * bw;
    // adjust signs (if necessary)
    if ( cosom < 0.0 ) {
        cosom = -cosom;
        bx = - bx;
        by = - by;
        bz = - bz;
        bw = - bw;
    }
    // calculate coefficients
    if ( (1.0 - cosom) > 0.000001 ) {
        // standard case (slerp)
        omega  = acos(cosom);
        sinom  = sin(omega);
        scale0 = sin((1.0 - t) * omega) / sinom;
        scale1 = sin(t * omega) / sinom;
    } else {
        // "from" and "to" quaternions are very close
        //  ... so we can do a linear interpolation
        scale0 = 1.0 - t;
        scale1 = t;
    }
    // calculate final values
    mathfu::quat result = mathfu::quat(
        scale0 * aw + scale1 * bw,
        scale0 * ax + scale1 * bx,
        scale0 * ay + scale1 * by,
        scale0 * az + scale1 * bz
    ).Normalized();


    return result;
}

template<>
inline mathfu::quat lerpHelper<mathfu::quat>(mathfu::quat &value1, mathfu::quat &value2, float percent) {
    return mathfu::quat::Slerp(value1, value2, percent);
    //return custom_slerp(value1, value2, percent);
};
template<>
inline float lerpHelper<float>(float &value1, float &value2, float percent) {
    return (value1 * percent) + ((1 - percent) * value2);

};


template<typename T, typename R>
R animateTrack(
        animTime_t currTime,
        uint32_t maxTime,
        int animationIndex,
        M2Track<T> &animationBlock,
        M2Array<M2Loop> &global_loops,
        std::vector<animTime_t> &globalSequenceTimes,
        R &defaultValue) {


    if (animationBlock.timestamps.size <= animationIndex) {
        animationIndex = 0;
    }

    if (animationBlock.timestamps.size <= 0) {
        return defaultValue;
    }

    if (animationIndex <= animationBlock.timestamps.size && animationBlock.timestamps[animationIndex]->size == 0) {
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

//    if (maxTime == 0 ) {
//        maxTime = *times->getElement(times->size - 1);
//        if (maxTime > 0)
//            currTime = currTime % maxTime;
//    }

    currTime = fmod(currTime , maxTime);

    if (animationBlock.timestamps.size == 0 || animationIndex >= animationBlock.timestamps.size ) {
        return defaultValue;
    }

    M2Array<uint32_t> *timeStamp = animationBlock.timestamps[animationIndex];

    timeIndex = findTimeIndex(currTime, timeStamp->getElement(0), timeStamp->size);

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
            return lerpHelper<R>(value1, value2, (float)((float)currTime - time1)/(float)(time2 - time1));
        }
    } else {
        return convertHelper<T, R>(*values->getElement(0));
//        return defaultValue;
    }
}

template<typename T, typename R>
R animatePartTrack(
        animTime_t age,
        M2PartTrack<T> *animationPartBlock,
        R &defaultValue) {

    if (animationPartBlock->timestamps.size <= 0) {
        return defaultValue;
    }

    auto timeStamps = &animationPartBlock->timestamps;
    M2Array<T> *values = &animationPartBlock->values;
    int32_t timeIndex = findTimeIndex(age, timeStamps->getElement(0), timeStamps->size);

    if (timeIndex == timeStamps->size-1) {
        return convertHelper<T, R>(*values->getElement(timeIndex));
    } else if (timeIndex >= 0) {
        R value1 = convertHelper<T, R>(*values->getElement(timeIndex));
        R value2 = convertHelper<T, R>(*values->getElement(timeIndex+1));

        float currTimeF = convertHelper<animTime_t, float>(age);
        float time1 = convertHelper<fixed16, float>(*timeStamps->getElement(timeIndex));
        float time2 = convertHelper<fixed16, float>(*timeStamps->getElement(timeIndex+1));

        return lerpHelper<R>(value1, value2, (float)(currTimeF - time1)/(float)(time2 - time1));
    } else {
        return convertHelper<T, R>(*values->getElement(0));
//        return defaultValue;
    }
}

#endif //WOWVIEWERLIB_ANIMATE_H
