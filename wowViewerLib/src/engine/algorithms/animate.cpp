//
// Created by deamon on 29.06.17.
//

#include "animate.h"

int binary_search(M2Array<uint32_t>& vec, int start, int end, uint32_t& key)
{
    // Termination condition: start index greater than end index
    if(start == end) {
        return start;
    }

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
    if (timestamps.size == 0 || animationIndex >= timestamps.size ) {
        return -1;
    }

    M2Array<uint32_t> *timeStamp = timestamps[animationIndex];
    int32_t times_len = timeStamp->size;

    if (times_len > 1 ) {
//        return binary_search(*timeStamp, 0, times_len-1, currTime);
        for (int i = 1; i < times_len; i++) {
            if (*timeStamp->getElement(i) > currTime) {
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
