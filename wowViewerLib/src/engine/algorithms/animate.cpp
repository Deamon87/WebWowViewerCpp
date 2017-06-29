//
// Created by deamon on 29.06.17.
//

#include "animate.h"

int binary_search(M2Array<uint32_t>& vec, int start, int end, uint32_t& key)
{
    // Termination condition: start index greater than end index
    if(start > end)
    {
        return -1;
    }

    // Find the middle element of the vector and use that for splitting
    // the array into two pieces.
    const int middle = start + ((end - start) / 2);

    if(*vec[middle] == key)
    {
        return middle;
    }
    else if(*vec[middle] > key)
    {
        return binary_search(vec, start, middle - 1, key);
    }

    return binary_search(vec, middle + 1, end, key);
}

int32_t findTimeIndex(
        unsigned int currTime,
        int animationIndex,
        M2Array<M2Array<uint32_t>> &timestamps
) {
    int timeIndex = -1;
    if (timestamps.size == 0 || animationIndex < timestamps.size ) {
        return -1;
    }

    M2Array<uint32_t> *timeStamp = timestamps[animationIndex];
    int32_t times_len = timeStamp->size;

    if (times_len > 1 ) {
        return binary_search(*timeStamp, 0, times_len, currTime);
    } else if (times_len == 1){
        return 0;
    } else {
        return -1;
    }
}

template<typename T, typename R>
R animateTrack(
        unsigned int currTime,
        uint32_t maxTime,
        int animationIndex,
        M2Track<T> &animationBlock,
        M2Array<M2Loop> &global_loops,
        std::vector<int> &globalSequenceTimes,
        R &defaultValue) {

    int16_t globalSequence = animationBlock.global_sequence;

    int32_t timeIndex;
    if (globalSequence >=0) {
        currTime = globalSequenceTimes[globalSequence];
        maxTime = global_loops[globalSequence]->timestamp;
    }

    M2Array<uint32_t > *times = animationBlock.timestamps[animationIndex];
    M2Array<T> *values = animationBlock.values[animationIndex];

    timeIndex = findTimeIndex(currTime, animationIndex, animationBlock.timestamps);

    if (timeIndex > 0) {
        animationBlock.timestamps;
        T value1 = values->getElement(timeIndex - 1);
        T value2 = values->getElement(timeIndex);

        int time1 = *times->getElement(timeIndex - 1);
        int time2 = *times->getElement(timeIndex);

        uint16_t interpolType = animationBlock.interpolation_type;
        if (interpolType == 0) {
            return value1;
        } else if (interpolType >= 1) {
            return T::Lerp(value1, value2, (currTime - time1)/(time2 - time1));
        }
    } else if (timeIndex == 0){
        return times->getElement(0);
    } else {
        return defaultValue;
    }
}