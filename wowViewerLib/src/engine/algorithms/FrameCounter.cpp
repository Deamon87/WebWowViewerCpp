//
// Created by Deamon on 10/5/2019.
//

#include "FrameCounter.h"
#include <chrono>
#include <iostream>

void FrameCounter::beginMeasurement() {
    m_startTime =  std::chrono::high_resolution_clock::now();
}

void FrameCounter::endMeasurement(const std::string &source) {
    auto end = std::chrono::high_resolution_clock::now();
    frameCounter++;

    double time_taken =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - m_startTime).count();

    m_accomulatedTimeInterval += time_taken;

    //More than 1 second elapsed
    if (m_accomulatedTimeInterval > 1000) {
        auto timePerFrame = m_accomulatedTimeInterval / frameCounter;

        std::cout << source << " perframe time = " << timePerFrame << "ms " << std::endl;

        frameCounter = 0;
        m_accomulatedTimeInterval = 0;
    }
}
