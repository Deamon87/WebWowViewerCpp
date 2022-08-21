//
// Created by Deamon on 10/5/2019.
//

#include "FrameCounter.h"
#include <chrono>
#include <iostream>
#include <ratio>

void FrameCounter::beginMeasurement() {
//#ifndef SKIP_VULKAN
    m_startTime = std::chrono::high_resolution_clock::now();
//#endif
}

void FrameCounter::endMeasurement(const std::string &source) {
//    #ifndef SKIP_VULKAN
    hi_res_time_point end = std::chrono::high_resolution_clock::now();
    frameCounter++;

    double time_taken =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - m_startTime).count();
//
    m_accomulatedTimeInterval += time_taken;
//
    //More than 1 second elapsed
//    if (m_accomulatedTimeInterval > 1000) {
    if (frameCounter > 10) {
        timePerFrame = m_accomulatedTimeInterval / frameCounter;
//

//
        frameCounter = 0;
        m_accomulatedTimeInterval = 0;
    }
//#endif
}
