//
// Created by deamon on 29.06.17.
//

#include "animate.h"

int binary_search(M2Array<uint32_t>& vec, int start, int end, animTime_t & key)
{
    // Termination condition: start index greater than end index
    if(start == end) {
        return start;
    }

    if(start > end)
    {
        return end-1;
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

