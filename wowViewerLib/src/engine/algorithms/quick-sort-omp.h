//
// Created by Deamon on 12/26/2019.
//

#ifndef AWEBWOWVIEWERCPP_QUICK_SORT_OMP_H
#define AWEBWOWVIEWERCPP_QUICK_SORT_OMP_H
#include <functional>
#include "utility.h"

void quickSort_parallel_internal(int* array, int left, int right, int cutoff, std::function<bool(int a, int b)> &compare);

void quickSort_parallel(int* array, int lenArray, int numThreads, int cutoff, std::function<bool(int a, int b)> compare){
    {
        {
            {
                quickSort_parallel_internal(array, 0, lenArray - 1, cutoff, compare);
            }
        }
    }

}

// A utility function to swap two elements
void swap(int* a, int* b)
{
    int t = *a;
    *a = *b;
    *b = t;
}

/* This function takes last element as pivot, places
the pivot element at its correct position in sorted
array, and places all smaller (smaller than pivot)
to left of pivot and all greater elements to right
of pivot */
int partition (int* arr, int low, int high, std::function<bool(int a, int b)> &compare)
{
    int pivot = arr[high]; // pivot
    int i = (low - 1); // Index of smaller element

    for (int j = low; j <= high - 1; j++)
    {
        // If current element is smaller than the pivot
        if (compare(arr[j], pivot))
        {
            i++; // increment index of smaller element
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return (i + 1);
}


void quickSort_parallel_internal(int* array, int left, int right, int cutoff, std::function<bool(int a, int b)> &compare)
{
    if (right <= left) return;

    int newPartPoint = partition(array, left, right, compare);  ;

    if ( ((right-left)<cutoff) ){
        { quickSort_parallel_internal(array, left, newPartPoint-1, cutoff, compare); }
        { quickSort_parallel_internal(array, newPartPoint+1, right, cutoff, compare); }

    }else {
        {
            {
                quickSort_parallel_internal(array, left, newPartPoint-1, cutoff, compare);
            }

            {
                quickSort_parallel_internal(array, newPartPoint+1, right, cutoff, compare);
            }
        }
    }
}


namespace internal
{
    std::size_t g_depth = 0L;
    const std::size_t cutoff = 1000000L;

    template<class RanIt, class _Pred>
    void qsort3w(RanIt _First, RanIt _Last, _Pred compare)
    {
        if (_First >= _Last) return;

        std::size_t _Size = 0L; g_depth++;
        if ((_Size = std::distance(_First, _Last)) > 0)
        {
            RanIt _LeftIt = _First, _RightIt = _Last;
            bool is_swapped_left = false, is_swapped_right = false;
            typename std::iterator_traits<RanIt>::value_type _Pivot = *_First;

            RanIt _FwdIt = _First + 1;
            while (_FwdIt <= _RightIt)
            {
                if (compare(*_FwdIt, _Pivot))
                {
                    is_swapped_left = true;
                    std::iter_swap(_LeftIt, _FwdIt);
                    _LeftIt++; _FwdIt++;
                }

                else if (compare(_Pivot, *_FwdIt)) {
                    is_swapped_right = true;
                    std::iter_swap(_RightIt, _FwdIt);
                    _RightIt--;
                }

                else _FwdIt++;
            }

            if (_Size >= internal::cutoff)
            {
#pragma omp taskgroup
                {
#pragma omp task untied mergeable
                    if ((std::distance(_First, _LeftIt) > 0) && (is_swapped_left))
                        qsort3w(_First, _LeftIt - 1, compare);

#pragma omp task untied mergeable
                    if ((std::distance(_RightIt, _Last) > 0) && (is_swapped_right))
                        qsort3w(_RightIt + 1, _Last, compare);
                }
            }

            else
            {
#pragma omp task untied mergeable
                {
                    if ((std::distance(_First, _LeftIt) > 0) && is_swapped_left)
                        qsort3w(_First, _LeftIt - 1, compare);

                    if ((std::distance(_RightIt, _Last) > 0) && is_swapped_right)
                        qsort3w(_RightIt + 1, _Last, compare);
                }
            }
        }
    }

    template<class BidirIt, class _Pred >
    void parallel_sort(BidirIt _First, BidirIt _Last, _Pred compare)
    {
        std::size_t pos = 0L; g_depth = 0L;
        if (!misc::sorted(_First, _Last, pos, compare))
        {
#pragma omp parallel num_threads(12)
#pragma omp master
            internal::qsort3w(_First, _Last - 1, compare);
        }
    }
}

#endif //AWEBWOWVIEWERCPP_QUICK_SORT_OMP_H
