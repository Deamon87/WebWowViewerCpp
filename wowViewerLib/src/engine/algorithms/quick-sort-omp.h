//
// Created by Deamon on 12/26/2019.
//

#ifndef AWEBWOWVIEWERCPP_QUICK_SORT_OMP_H
#define AWEBWOWVIEWERCPP_QUICK_SORT_OMP_H
#include <functional>

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


#endif //AWEBWOWVIEWERCPP_QUICK_SORT_OMP_H
