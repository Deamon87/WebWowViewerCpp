//
// Created by Deamon on 10/5/2019.
//

#ifndef AWEBWOWVIEWERCPP_FRAMECOUNTER_H
#define AWEBWOWVIEWERCPP_FRAMECOUNTER_H

#include <chrono>
#include <string>

class FrameCounter {

public:
    FrameCounter(){};
    void beginMeasurement();
    void endMeasurement(const std::string &source);

    double getTimePerFrame() {
        return timePerFrame;
    }
private:
//#if defined(_MSC_VER)
//    std::chrono::time_point<std::chrono::steady_clock> m_startTime;
//#else
    using hi_res_time_point = std::chrono::time_point<std::chrono::high_resolution_clock>;

    hi_res_time_point m_startTime;
//#endif
    double m_accomulatedTimeInterval = 0;
    int frameCounter = 0;

    double timePerFrame;

};


#endif //AWEBWOWVIEWERCPP_FRAMECOUNTER_H
