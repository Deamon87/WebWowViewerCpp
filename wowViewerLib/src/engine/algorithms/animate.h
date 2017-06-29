//
// Created by deamon on 29.06.17.
//

#ifndef WOWVIEWERLIB_ANIMATE_H
#define WOWVIEWERLIB_ANIMATE_H

#include "../persistance/M2File.h"

template<typename T, typename R>
R animateTrack(
        unsigned int currTime,
        uint32_t maxTime,
        int animationIndex,
        M2Track<T> &animationBlock,
        M2Array<M2Loop> &global_loops,
        std::vector<int> &globalSequenceTimes,
        R &defaultValue);

#endif //WOWVIEWERLIB_ANIMATE_H
