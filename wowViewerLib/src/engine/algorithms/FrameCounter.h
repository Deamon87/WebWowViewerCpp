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

private:
    std::chrono::system_clock::time_point m_startTime;
    double m_accomulatedTimeInterval = 0;
    int frameCounter = 0;

};


#endif //AWEBWOWVIEWERCPP_FRAMECOUNTER_H
