//
// Created by Deamon on 12/26/2019.
//

#ifndef AWEBWOWVIEWERCPP_QUICK_SORT_OMP_H
#define AWEBWOWVIEWERCPP_QUICK_SORT_OMP_H
#include <functional>

void quickSort_parallel_internal(int* array, int left, int right, int cutoff, std::function<bool(int a, int b)> compare);

void quickSort_parallel(int* array, int lenArray, int numThreads, int cutoff, std::function<bool(int a, int b)> compare){
    {
        #pragma omp parallel num_threads(numThreads)
        {
            #pragma omp single nowait
            {
                quickSort_parallel_internal(array, 0, lenArray - 1, cutoff, compare);
            }
        }
    }

}



void quickSort_parallel_internal(int* array, int left, int right, int cutoff, std::function<bool(int a, int b)> compare)
{

    int i = left, j = right;
    int tmp;
    int pivot = array[(left + right) / 2];


    {
        /* PARTITION PART */
        while (i <= j) {
            while (compare(array[i], pivot) && i < right)
                i++;
            while (!compare(array[j], pivot) && j > left)
                j--;
            if (i <= j) {
                tmp = array[i];
                array[i] = array[j];
                array[j] = tmp;
                i++;
                j--;
            }
        }

    }


    if ( ((right-left)<cutoff) ){
        if (left < j){ quickSort_parallel_internal(array, left, j, cutoff, compare); }
        if (i < right){ quickSort_parallel_internal(array, i, right, cutoff, compare); }

    }else {
        #pragma omp parallel
        {
            #pragma omp task shared(array, left, j, cutoff, compare)
            {
                quickSort_parallel_internal(array, left, j, cutoff, compare);
            }

            #pragma omp task shared(array, right, i, cutoff, compare)
            {
                quickSort_parallel_internal(array, i, right, cutoff, compare);
            }
        }
    }

}


#endif //AWEBWOWVIEWERCPP_QUICK_SORT_OMP_H
